#include "titulus.h"

#include "bin/data_parser.h"
#include "utils/globals.h"
#include "context.h"
#include "utils/string_container.h"
#include "render/targets.h"
#include "core/internal_lua_state.h"
#include "utils/systems.h"
#include "province.h"
#include "realm_rights_checker.h"

namespace devils_engine {
  namespace core {
    const structure titulus::s_type;
    const size_t titulus::events_container_size;
    const size_t titulus::flags_container_size;
    titulus::titulus() : 
      t(type::count), 
      city(nullptr),
      province(nullptr),
      parent(nullptr), 
      children(nullptr),
      owner(nullptr), 
      static_state(global::advance_state()), 
      main_color{UINT32_MAX},
      border_color1{UINT32_MAX},
      border_color2{UINT32_MAX}
    {
      memset(heraldy_container.data(), 0, sizeof(heraldy_container[0]) * heraldy_container.size());
    }
    
    titulus::titulus(const enum type &t) : 
      t(t),  
      city(nullptr),
      province(nullptr),
      parent(nullptr), 
      children(nullptr),
      owner(nullptr), 
      // static_state - нужно заполнить более контроллируемым способом
      static_state(global::advance_state()),
      main_color{UINT32_MAX},
      border_color1{UINT32_MAX},
      border_color2{UINT32_MAX}
    {
      memset(heraldy_container.data(), 0, sizeof(heraldy_container[0]) * heraldy_container.size());
    }
    
    titulus::~titulus() {}
    
    enum titulus::type titulus::type() const {
      //assert((t == type::city) == (province != nullptr)); // зачем?
      return t;
      //return t == type::city && province->capital == city ? type::baron : t;
    }
    
    bool titulus::is_formal() const {
      return t != type::city && t != type::baron && children == nullptr;
    }
    
    OUTPUT_TITLE_TYPE titulus::get_parent() { return parent; }
    OUTPUT_CITY_TYPE2 titulus::get_city() { return city; }
    OUTPUT_PROVINCE_TYPE titulus::get_province() { return province; }
    OUTPUT_TITLE_TYPE titulus::get_barony() { return t == type::city ? parent : (t == type::baron ? this : nullptr); }
    OUTPUT_TITLE_TYPE titulus::get_duchy() { return rights::get_duchy(this); }
    OUTPUT_TITLE_TYPE titulus::get_kingdom() { return rights::get_kingdom(this); }
    OUTPUT_TITLE_TYPE titulus::get_empire() { return rights::get_empire(this); }
    OUTPUT_TITLE_TYPE titulus::get_top_title() { return rights::get_top_title(this); }
    OUTPUT_TITLE_TYPE titulus::get_de_facto_liege() { return owner.valid() && owner->liege.valid() ? owner->liege->main_title : nullptr; }
    OUTPUT_TITLE_TYPE titulus::get_de_jure_liege() { return get_parent(); }
    OUTPUT_PROVINCE_TYPE titulus::get_main_province() { return nullptr; } // не знаю что тут
    OUTPUT_REALM_TYPE titulus::get_owner() { return owner; }
    
    const utils::check_table_value title_table[] = {
      {
        "type",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, static_cast<int32_t>(core::titulus::type::count), {}
      },
      {
        "parent",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "main_color",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, UINT32_MAX, {}
      }, 
      {
        "border_color1",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, UINT32_MAX, {}
      }, 
      {
        "border_color2",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, UINT32_MAX, {}
      }, 
      {
        "heraldy",
        utils::check_table_value::type::string_t,
        0, 0, {}
      }
    };
    
//     size_t add_title(const sol::table &table) {
//       return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::titulus), table);
//     }
    
    bool validate_title(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "title" + std::to_string(index);
      }
      
      const size_t size = sizeof(title_table) / sizeof(title_table[0]);
      recursive_check(check_str, "title", table, nullptr, title_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_title_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator*) {
      const bool ret = validate_title(index, table);
      if (!ret) return false;
      
      sol::state_view state(lua);
      auto str = utils::table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize title table");
      ASSERT(false);
//       container->add_data(core::structure::titulus, std::move(str));
      
      return true;
    }
    
    void parse_title(core::titulus* title, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
//       auto heraldy_data = global::get<utils::numeric_string_container>();
      auto ctx = global::get<core::context>();
      auto buffers = global::get<render::buffers>();
      auto internal = global::get<systems::core_t>()->internal.get();
      
      // мы создаем титул до городов? наверное сначало город, тогда нам можно не заполнять детей
      //title->id = table["id"];
      
      {
        const int32_t lua_val = table["type"];
        if (lua_val < 0 || lua_val >= static_cast<int32_t>(core::titulus::type::count)) throw std::runtime_error("Bad title type for " + title->id);
        const auto value = static_cast<enum core::titulus::type>(lua_val);
        title->t = value;
      }
      
      if (const auto &parent = table["parent"]; parent.valid()) { // у некоторых титулов низкого уровня может не быть родительских титулов
        if (title->t == titulus::type::top_type) throw std::runtime_error("Top tier title " + title->id + " could not have a parent title");
        const std::string str = parent.get<std::string>();
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find parent title " + str + " for " + title->id);
        auto t = ctx->get_entity<core::titulus>(index);
        title->parent = t;
      } 
      
      // наверное лучше собрать детей по родительским указателям
      
      {
        const uint32_t col = table["main_color"];
        render::color_t c{col};
        title->main_color = c;
      }
      
      {
        const uint32_t col = table["border_color1"];
        render::color_t c{col};
        title->border_color1 = c;
      }
      
      {
        const uint32_t col = table["border_color2"];
        render::color_t c{col};
        title->border_color2 = c;
      }
      
      auto heraldy_proxy = table["heraldy"];
      sol::table id_table;
      if (heraldy_proxy.get_type() == sol::type::table) {
        id_table = heraldy_proxy;
      }
      
      if (heraldy_proxy.get_type() == sol::type::string) {
        const std::string str = heraldy_proxy;
        const auto func_proxy = internal->gen_funcs_table[str];
        if (!func_proxy.valid() || func_proxy.get_type() != sol::type::function) throw std::runtime_error("Could not find heraldy gen function " + str);
        // откуда брать сид для генератора? по идее мы можем взять из глобала
        const auto func = func_proxy.get<sol::function>();
        sol::state_view s = func.lua_state();
        const auto ret = func(uns_to_signed64(title->static_state));
        CHECK_ERROR_THROW(ret);
        if (ret.get_type() != sol::type::table) throw std::runtime_error("Heraldy gen function " + str + " invalid returns");
        id_table = ret;
        
//         const size_t index = heraldy_data->get(str);
//         if (index == SIZE_MAX) throw std::runtime_error("Could not find heraldy " + str);
//         if (index >= UINT32_MAX) throw std::runtime_error("Could bad heraldy " + str + " index");
//         title->heraldy = index;
      }
      
      if (!id_table.valid()) throw std::runtime_error("Missing heraldy data for titulus " + title->id);
      
      std::vector<uint32_t> indices;
      for (const auto &pair : id_table) {
        if (pair.second.get_type() != sol::type::string) continue;
        
        const auto str = pair.second.as<std::string_view>();
        const auto itr = buffers->heraldy_layers_map.find(str);
        if (itr == buffers->heraldy_layers_map.end()) throw std::runtime_error("Could not find heraldy layer " + std::string(str));
        indices.push_back(itr->second);
      }
      
      if (indices.size() > core::titulus::heraldy_container_size) throw std::runtime_error("Supported heraldy up to " + std::to_string(core::titulus::heraldy_container_size) + " layers");
      title->heraldy_layers_count = indices.size();
      memcpy(title->heraldy_container.data(), indices.data(), indices.size() * sizeof(indices[0]));
      
      //buffers->new_indices.emplace_back(title, std::move(indices));
      
      // добавится герб
      
      // по идее мы можем здесь указать владельца титула
      // но это будет менее правильно
      // потому что по идее мы сначало раскидываем титулы по провинциям и землям
      // после этого по идее должно быть формирование государств (хотя может и нет)
      // тем не менее мне кажется что указать список титулов персонажу лучше 
      
      // указывать здесь владельца или нет зависит в основном от порядка загрузки (нет)
      // точнее если я хочу взять какую то информацию из объекта, то понятное дело он уже к этому времени должен быть загружен
      // здесь я хочу взять родительский титул из города (делать мы это будем после парсинга я так понимаю)
    }
  }
}
