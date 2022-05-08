#include "lua_initialization_hidden.h"

#include <queue>

#include "render/shared_render_utility.h"
#include "render/shared_structures.h"

#include "utils/localization_container.h"
#include "utils/magic_enum_header.h"
#include "utils/linear_rng.h"
#include "utils/lua_environment.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "lua_initialization_handle_types.h"

#include "fmt/format.h"

#include "core/structures_header.h"
#include "core/stats.h"
#include "core/stats_table.h"
#include "core/realm_mechanics_arrays.h"
#include "core/map.h"

#include "ai/path_container.h"

#include <iostream>

#define DEFAULT_LUA_REF_COPY_SIZE 256
#define DEFAULT_ONE_FUNCTION_MAXIMUM_CALLS 256

#define CHECK_ERROR(ret) if (!ret.valid()) {       \
  sol::error err = ret;                            \
  std::cout << err.what();                         \
  luaL_error(s, "Catched lua error");              \
}

namespace devils_engine {
  namespace utils {
    constexpr bool find_substr(const std::string_view &str, const std::string_view &sub) {
      return str.find(sub) != std::string_view::npos;
    }
    
    static double create_pair_u32f32(const uint32_t &u32, const float &f32) {
      union convert { uint64_t u; double d; };
      const uint32_t &data = glm::floatBitsToUint(f32);
      const uint64_t packed_data = (uint64_t(u32) << 32) | uint64_t(data);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    static std::tuple<uint32_t, float> unpack_pair_u32f32(const double &data) {
      union convert { uint64_t u; double d; };
      convert c;
      c.d = data;
      const uint64_t num = c.u;
      const uint32_t data1 = uint32_t(num >> 32);
      const uint32_t data2 = uint32_t(num);
      const float fdata = glm::uintBitsToFloat(data2);
      return std::tie(data1, fdata);
    }
    
    static double make_pair_u32u32(const uint32_t &u1, const uint32_t &u2) {
      union convert { uint64_t u; double d; };
      const uint64_t packed_data = (uint64_t(u1) << 32) | uint64_t(u2);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    static std::tuple<uint32_t, uint32_t> unpack_pair_u32u32(const double &d) {
      union convert { uint64_t u; double d; };
      convert c;
      c.d = d;
      const uint64_t num = c.u;
      const uint32_t data1 = uint32_t(num >> 32);
      const uint32_t data2 = uint32_t(num);
      return std::tie(data1, data2);
    }
    
    constexpr bool is_integer(const double num) {
      return std::abs(double(int64_t(num)) - num) < EPSILON;
    }
    
    static size_t rnd_index(utils::xoshiro256starstar::state &state, const size_t &size) {
      uint64_t i = UINT64_MAX;
      while (i >= size) {
        state = utils::xoshiro256starstar::rng(state);
        const uint64_t value = utils::xoshiro256starstar::get_value(state);
        const double norm = utils::rng_normalize(value);
        i = norm * double(size);
      }
      
      return i;
    }
    
    static size_t count_func(sol::this_state this_s, const sol::object &iterable) {
      sol::state_view s = this_s;
      size_t func_counter = 0;
      if (iterable.get_type() == sol::type::table) {
        const auto &table = iterable.as<sol::table>();
        for (const auto &pair : table) { ++func_counter; (void)pair; }
      } else if (iterable.get_type() == sol::type::string) {
        const auto &str = iterable.as<std::string_view>();
        func_counter = str.size();
      } else if (iterable.get_type() == sol::type::function) {
        const auto &iterable_func = iterable.as<sol::function>();
        const auto &itr_ret = iterable_func(); CHECK_ERROR_THROW(itr_ret);
        std::tuple<sol::object, sol::object> current = itr_ret;
        while (std::get<0>(current).valid() && std::get<0>(current).get_type() != sol::type::nil) {
          ++func_counter;
          const auto &itr_ret = iterable_func(); CHECK_ERROR_THROW(itr_ret);
          current = itr_ret;
        }
      } else throw std::runtime_error("utils.count: bad iterable input");
      
      return func_counter;
    }
    
    // вообще имеет смысл тут передать формат и скормить это дело все в fmt
    // формат вида фмт то есть выглядит примерно так "{} {}'{}"
    static std::string compose_string(const std::string_view &format, const sol::variadic_args &args) {
      // тут мы должны получать строки из локализации и соединять их в одну через пробел (через пробел ли?)
      // правила соединения строк могут быть разными, но с друго стороны эта функция тупо делает компоновку строк 
      // наиболее быстро и эффективно (тут мы хотя бы память заранее задаем), в локализации у нас может вернуться таблица
      // а не строка, то есть таблица с множеством имен в индекс массиве, из индекс массива нам нужно взять имя по индексу
      // а хотя мы можем скармливать строчку вида: id1.id2.3, но нам соответственно нужно будет определить что перед нами число, 
      // а не символ строки, ну хотя нейминг может быть поразному устроен, хотя лучше наверное индекс все же числом получать
      
      // короч теперь я могу получить что то вразумительное из локализации даже скормив строчку вида table1.212.table2
      // поэтому имеет смысл ожидать здесь строку в любом случае
      
      auto loc = global::get<systems::core_t>()->loc.get();
      
      assert(format.back() != '\0');
      
      const size_t max_count = 256;
      size_t string_count = 0;
      std::array<fmt::format_args::format_arg, max_count> strings;
      for (const auto &obj : args) {
        if (obj.get_type() != sol::type::string) throw std::runtime_error("compose_string expecting only strings");
        auto str = obj.as<std::string_view>();
        if (find_substr(str, ".")) {
          const auto obj = loc->get(devils_engine::localization::container::get_current_locale(), str);
          if (obj.get_type() != sol::type::string) throw std::runtime_error("Got bad object from localization container using key " + std::string(str));
          str = obj.as<std::string_view>();
        }
        
        strings[string_count] = fmt::v8::detail::make_arg<fmt::format_context>(str);
        ++string_count;
        if (string_count >= max_count) throw std::runtime_error("Too many args");
      }
      
      const auto &str = fmt::vformat(format, fmt::format_args(strings.data(), string_count));
      return str;
    }
    
    void setup_lua_safe_utils(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("prng32", render::prng);
      utils.set_function("prng32_2", [] (const uint32_t &val1, const uint32_t &val2) { return render::prng2(render::prng(val1), render::prng(val2)); });
      utils.set_function("prng_normalize32", render::prng_normalize);
      utils.set_function("prng64", [] (const int64_t &value) {
        const uint64_t val = s_to_unsigned64(value);
        return uns_to_signed64(splitmix64::get_value(splitmix64::rng({val})));
      });
      utils.set_function("prng64_2", [] (const int64_t &value1, const int64_t &value2) {
        const uint64_t val1 = s_to_unsigned64(value1);
        const uint64_t val2 = s_to_unsigned64(value2);
        //if (value1 + value2 == 0) throw std::runtime_error("Summ of prng64_2 args must not be 0"); // ???
        return uns_to_signed64(xoroshiro128starstar::get_value(
          xoroshiro128starstar::rng({
            splitmix64::get_value(splitmix64::rng({val1})), 
            splitmix64::get_value(splitmix64::rng({val2}))
          })
        ));
      });
      utils.set_function("prng_normalize64", [] (const int64_t &value) {
        const uint64_t val = s_to_unsigned64(value);
        return rng_normalize(val);
      });
      
      utils.set_function("make_color", [] (const double r, const double g, const double b, const double a) {
        return render::make_color(r, g, b, a).container;
      });
      
      utils.set_function("unpack_color", [] (const uint32_t data) -> std::tuple<double, double, double, double> {
        const auto col = render::get_color(render::color_t(data));
        return std::make_tuple(col.r, col.g, col.b, col.a);
      });
      
      // add - эту хуйню вообще лучше не использовать
      utils.set_function("init_array", [] (const double &num, sol::object default_value, sol::this_state s) -> sol::table {
        const size_t size = num;
        sol::state_view view(s);
        auto t = view.create_table(size, 0);
        if (default_value.get_type() == sol::type::table) {
          for (size_t i = 0; i < size; ++i) {
            const size_t index = TO_LUA_INDEX(i);
            t[index] = view.create_table(30, 0);
            //t.add(view.create_table(30, 0));
          }
        } else {
          for (size_t i = 0; i < size; ++i) {
            const size_t index = TO_LUA_INDEX(i);
            t[index] = default_value;
            //t.add(default_value);
          }
        }
        return t;
      });
      
      utils.set_function("create_table", [] (const sol::object arr_size, const sol::object hash_size, sol::this_state s) -> sol::table {
        sol::state_view view(s);
        const uint32_t narr = arr_size.is<uint32_t>() ? arr_size.as<uint32_t>() : 100;
        const uint32_t nhash = hash_size.is<uint32_t>() ? hash_size.as<uint32_t>() : 100;
        return view.create_table(narr, nhash);
      });
      
      utils.set_function("create_pair_u32f32", &create_pair_u32f32);
      utils.set_function("unpack_pair_u32f32", &unpack_pair_u32f32);
      
      utils.set_function("create_pair_u32u32", &make_pair_u32u32);
      utils.set_function("unpack_pair_u32u32", &unpack_pair_u32u32);
      
      utils.set_function("compose_string", &compose_string);
      
      // это мне нахрен не нужно если есть функциональная либа
      utils.set_function("count", &count_func);
      // с функциональной либой не получилось, точшее луа сам по себе неплохо код оптимизирует, а вызовы с++ функций все портят
      
      utils.set_function("int_queue", [] (sol::this_state s, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
        if (first_count < 0) throw std::runtime_error("Bad count value " + std::to_string(first_count));
        if (std::abs(first_count) < EPSILON) return;
        
        sol::state_view lua = s;
        std::queue<int64_t> queue;
        const auto push_func = [&queue] (const int64_t data) { queue.push(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        
        const size_t size = first_count;
        for (size_t i = 0; i < size; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const int64_t data = queue.front();
          queue.pop();
          
          queue_function(data, lua_push_func);
        }
      });
      
      utils.set_function("num_queue", [] (sol::this_state s, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
        if (first_count < 0) throw std::runtime_error("Bad count value " + std::to_string(first_count));
        if (std::abs(first_count) < EPSILON) return;
        
        sol::state_view lua = s;
        std::queue<double> queue;
        const auto push_func = [&queue] (const double data) { queue.push(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        
        const size_t size = first_count;
        for (size_t i = 0; i < size; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const double data = queue.front();
          queue.pop();
          
          queue_function(data, lua_push_func);
        }
      });
      
      utils.set_function("queue", [] (sol::this_state s, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
        if (first_count < 0) throw std::runtime_error("Bad count value " + std::to_string(first_count));
        if (std::abs(first_count) < EPSILON) return;
        
        sol::state_view lua = s;
        std::queue<sol::object> queue;
        const auto push_func = [&queue] (const sol::object data) { queue.push(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        
        const size_t size = first_count;
        for (size_t i = 0; i < size; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const sol::object data = queue.front();
          queue.pop();
          
          queue_function(data, lua_push_func);
        }
      });      
      
      utils.set_function("int_random_queue", [] (sol::this_state s, const int64_t &seed, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
        if (first_count < 0) throw std::runtime_error("Bad count value " + std::to_string(first_count));
        if (std::abs(first_count) < EPSILON) return;
        
        sol::state_view lua = s;
        std::vector<int64_t> queue;
        queue.reserve(first_count * 2);
        const auto push_func = [&queue] (const int64_t data) { queue.push_back(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        auto rnd_state = xoshiro256starstar::init(s_to_unsigned64(seed));
        
        for (size_t i = 0; i < first_count; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const size_t rand_index = rnd_index(rnd_state, queue.size());
          const int64_t data = queue[rand_index];
          queue[rand_index] = queue.back();
          queue.pop_back();
          
          queue_function(data, lua_push_func);
        }
      });
      
      utils.set_function("num_random_queue", [] (sol::this_state s, const int64_t &seed, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
        if (first_count < 0) throw std::runtime_error("Bad count value " + std::to_string(first_count));
        if (std::abs(first_count) < EPSILON) return;
        
        sol::state_view lua = s;
        std::vector<double> queue;
        const size_t size = first_count;
        queue.reserve(size * 2);
        const auto push_func = [&queue] (const double data) { queue.push_back(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        auto rnd_state = xoshiro256starstar::init(s_to_unsigned64(seed));
        
        for (size_t i = 0; i < size; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const size_t rand_index = rnd_index(rnd_state, queue.size());
          const double data = queue[rand_index];
          queue[rand_index] = queue.back();
          queue.pop_back();
          
          queue_function(data, lua_push_func);
        }
      });
      
      utils.set_function("random_queue", [] (sol::this_state s, const int64_t &seed, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
        if (first_count < 0) throw std::runtime_error("Bad count value " + std::to_string(first_count));
        if (std::abs(first_count) < EPSILON) return;
        
        sol::state_view lua = s;
        std::vector<sol::object> queue;
        const size_t size = first_count;
        queue.reserve(size * 2);
        const auto push_func = [&queue] (const sol::object data) { queue.push_back(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        auto rnd_state = xoshiro256starstar::init(s_to_unsigned64(seed));
        
        for (size_t i = 0; i < size; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const size_t rand_index = rnd_index(rnd_state, queue.size());
          const sol::object data = queue[rand_index];
          queue[rand_index] = queue.back();
          queue.pop_back();
          
          queue_function(data, lua_push_func);
        }
      });
      
      // что тут вернуть? строку или число? нагляднее наверное будет вернуть строку, type вот у меня тоже строку возвращает
      // другое дело что возврат строки 200% связан с выделением памяти для обекта + вычислением хеша
      // по всей видимости не нужно об этом задумываться
      utils.set_function("get_stat_type", [] (sol::this_state, const sol::object &stat) -> std::string_view {
        if (stat.get_type() != sol::type::number && stat.get_type() != sol::type::string) return "invalid";
                         
        if (stat.get_type() == sol::type::number) {
          const auto val = FROM_LUA_INDEX(stat.as<double>());
          const size_t final_val = val;
          return core::stat_type::names[core::get_stat_type(final_val)];
        }
        
#define STAT_CONDITION_FUNC(name) if (const auto itr = core::name##s::map.find(str); itr != core::name##s::map.end()) return core::stat_type::names[core::stat_type::name];
        
        // супер тяжелая функция
        const auto &str = stat.as<std::string_view>();
        STAT_CONDITION_FUNC(character_stat)
        STAT_CONDITION_FUNC(realm_stat)
        STAT_CONDITION_FUNC(province_stat)
        STAT_CONDITION_FUNC(city_stat)
        STAT_CONDITION_FUNC(army_stat)
        STAT_CONDITION_FUNC(hero_troop_stat)
        STAT_CONDITION_FUNC(troop_stat)
        STAT_CONDITION_FUNC(hero_stat)
        STAT_CONDITION_FUNC(character_resource)
        STAT_CONDITION_FUNC(realm_resource)
        STAT_CONDITION_FUNC(city_resource)
        STAT_CONDITION_FUNC(army_resource)
        
#undef STAT_CONDITION_FUNC
        
        return core::stat_type::names[0];
      });
      
      utils.set_function("get_stat_name", [] (sol::this_state, const size_t &index) {
        const size_t num = FROM_LUA_INDEX(index);
#define STAT_OFFSET_FUNC(name)                                       \
        if (num >= core::offsets::name && num < core::name::count) { \
          return core::name::names[num];                             \
        }
        
        STATS_OFFSET_LIST
#undef STAT_OFFSET_FUNC
        
        return std::string_view("");
      });
      
      utils.set_function("get_opinion_type_name", [] (sol::this_state, const size_t &index) {
        //const size_t num = FROM_LUA_INDEX(index);
        const size_t num = index;
        if (num == 0 || num >= core::opinion_modifiers::count) return core::opinion_modifiers::names[0];
        return core::opinion_modifiers::names[num];
      });
      
      utils.set_function("get_right_name", [] (sol::this_state, const size_t &index) {
        const size_t num = FROM_LUA_INDEX(index);
        if (num >= core::power_rights::offset && num < core::power_rights::count) {
          return core::power_rights::names[num];
        }
        
        if (num >= core::state_rights::offset && num < core::state_rights::count) {
          return core::state_rights::names[num];
        }
        
        return std::string_view("");
      });
      
      utils.set_function("get_culture_feature_name", [] (sol::this_state, const size_t &index) {
        const size_t num = FROM_LUA_INDEX(index);
        if (num < core::culture_mechanics::count) {
          return core::culture_mechanics::names[num];
        }
        
        return std::string_view("");
      });
      
      utils.set_function("get_religion_feature_name", [] (sol::this_state, const size_t &index) {
        const size_t num = FROM_LUA_INDEX(index);
        if (num < core::religion_mechanics::count) {
          return core::religion_mechanics::names[num];
        }
        
        return std::string_view("");
      });
      
      auto core = lua[magic_enum::enum_name(reserved_lua::core)].get_or_create<sol::table>();
      core.set_function("type", [] (sol::this_state, const sol::object &obj) -> std::string_view {
        if (!obj.valid()) return "nil";
                        
        const auto type = obj.get_type();
        if (type != sol::type::userdata) {
          switch (type) {
            case sol::type::nil: return "nil";
            case sol::type::none: return "none";
            case sol::type::string: return "string";
            case sol::type::number: return "number";
            case sol::type::thread: return "thread";
            case sol::type::boolean: return "boolean";
            case sol::type::function: return "function";
            case sol::type::userdata: return "userdata";
            case sol::type::lightuserdata: return "lightuserdata";
            case sol::type::table: return "table";
            default: assert(false);
          }
        }
        
#define GAME_STRUCTURE_FUNC(val) if (obj.is<core::val*>()) return #val;
        GAME_STRUCTURES_LIST
#undef GAME_STRUCTURE_FUNC
        // может ли сюда придти lightuserdata?
        if (obj.is<utils::lua_handle_realm>())      return "realm_handle";
        if (obj.is<utils::lua_handle_army>())       return "army_handle";
        if (obj.is<utils::lua_handle_hero_troop>()) return "hero_troop_handle";
        if (obj.is<utils::lua_handle_war>())        return "war_handle";
        if (obj.is<utils::lua_handle_troop>())      return "troop_handle";
                        
        return "userdata";
      });
      
      core.set_function("clear_module", [] (sol::this_state s, sol::this_environment e, const std::string_view &module_name) -> void {
        sol::state_view lua = s;
        const sol::object nil_obj = sol::make_object(lua, sol::nil);
        sol::environment &env = e;
        env[module_name] = nil_obj;
        
        std::string_view mod_name;
        std::string_view module_path;
        parse_module_name(module_name, mod_name, module_path);
        
        sol::table loaded_table = lua["package"]["loaded"];
        if (!mod_name.empty()) {
          const std::string local_module_name = "module_" + std::string(mod_name) + "_" + std::string(module_path);
          loaded_table[local_module_name] = nil_obj;
        }
      });
      
      // синхронный поиск =(
      core.set_function("find_path", [] (sol::this_state s, const uint32_t &tile_index1, const uint32_t &tile_index2, const sol::optional<sol::function> &func) {
        const uint32_t final_index1 = FROM_LUA_INDEX(tile_index1);
        const uint32_t final_index2 = FROM_LUA_INDEX(tile_index2);
        
        if (final_index1 >= core::map::hex_count_d(core::map::detail_level)) throw std::runtime_error("Start tile index " + std::to_string(tile_index1) + " is invalid");
        if (final_index2 >= core::map::hex_count_d(core::map::detail_level)) throw std::runtime_error("End tile index " + std::to_string(tile_index2) + " is invalid");
        
        auto path_finder = global::get<systems::core_t>()->path_managment;
          
        ai::path_container* path = nullptr;
        size_t path_size = 0;
        if (func.has_value()) {
          using float_t = ai::path_managment::float_t;
          const auto vertex_func = [func = func.value()] (const uint32_t &current_tile, const uint32_t &next_tile, const utils::user_data &) -> float_t {
            auto ret = func(TO_LUA_INDEX(current_tile), TO_LUA_INDEX(next_tile));
            CHECK_ERROR_THROW(ret);
            
            const float_t val = ret;
            return val;
          };
          
          path = path_finder->find_path_raw(final_index1, final_index2, utils::user_data(), vertex_func, path_size);
        } else {
          path = path_finder->find_path_raw(final_index1, final_index2, utils::user_data(), nullptr, path_size);
        }
        
        if (path_size == 0 || path == nullptr) return sol::object(sol::nil);
        
        sol::state_view lua = s;
        auto table = lua.create_table(path_size, 0);
        auto cur_path = path;
        size_t lua_counter = 1;
        for (size_t i = 0, counter = 0; i < path_size; ++i, ++counter) {
          if (counter >= ai::path_container::container_size) {
            cur_path = ai::advance_container(cur_path, 1);
            counter = 0;
          }
          
          // как положить данные? мне нужна пара (индекс, расстояние)
          // расстояние хранится в double, перевести его во float? расстояние хранится во флоате, будет ли потеря точности?
          // может быть положить просто один за другим?
          const auto &piece = cur_path->tile_path[counter];
          const double data = create_pair_u32f32(TO_LUA_INDEX(piece.tile), piece.cost);
          //table.add(data);
          table[lua_counter] = data;
          ++lua_counter;
        }
        
        path_finder->free_path(path);
        
        return sol::object(table);
      });
    }
  }
}
