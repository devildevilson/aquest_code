#include "map_creator.h"
#include "interface_context.h"
#include "utils/globals.h"
#include "fmt/format.h"
#include "generator_system2.h"
#include "map_generators2.h"
#include "utils/random_engine.h"
#include "FastNoise.h"
#include "generator_container.h"
#include "render/render_mode_container.h"
#include "utils/thread_pool.h"

#include <stdexcept>
#include <random>
#include <iostream>
#include <filesystem>

namespace devils_engine {
  namespace map {
    property_int::property_int(const create_info &info) : 
      min(info.min), 
      default_val(info.default_val),
      max(info.max), 
      step(info.step), 
      pixel_step(info.pixel_step), 
      prop_name(info.prop_name), 
      var_name(info.var_name) 
    {}
    
    void property_int::draw(sol::table &table) {
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      nk_layout_row_dynamic(ctx, 30.0f, 1);
      table["userdata"][var_name] = nk_propertyi(ctx, prop_name.c_str(), min, table["userdata"][var_name], max, step, pixel_step);
    }
    
    void property_int::set_default_value(sol::table &table) {
      table["userdata"][var_name] = default_val;
    }
    
    property_float::property_float(const create_info &info) :
      min(info.min), 
      default_val(info.default_val), 
      max(info.max), 
      step(info.step), 
      pixel_step(info.pixel_step), 
      prop_name(info.prop_name), 
      var_name(info.var_name) 
    {}
    
    void property_float::draw(sol::table &table) {
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      nk_layout_row_dynamic(ctx, 30.0f, 1);
      table["userdata"][var_name] = nk_propertyf(ctx, prop_name.c_str(), min, table["userdata"][var_name], max, step, pixel_step);
    }
    
    void property_float::set_default_value(sol::table &table) {
      table["userdata"][var_name] = default_val;
    }
    
//     step::step(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode) : 
//       first(first), 
//       container(container_size), 
//       pairs(pairs), 
//       name(name), 
//       rendering_mode(rendering_mode)
//     {}
    
    step::step(const sol::function &interface, const std::vector<map::generator_pair> &pairs) : pairs(pairs), interface(interface) {} //const bool first, 
    
    step::~step() {
//       for (auto p : variables) {
//         container.destroy(p);
//       }
    }
    
//     int32_t step::prepare(systems::generator &gen, map::generator::context* context, sol::table &table) {
//       auto interface = global::get<interface::context>();
//       auto ctx = &interface->ctx;
//       
//       static uint32_t random_seed = 1;
//       static char buffer[16] = "1";
//       static int32_t buffer_len = 1;
//       
//       int32_t code = 0;
//       if (nk_begin(ctx, name.c_str(), nk_rect(5, 5, 400, 400), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
//         nk_layout_row_static(ctx, 30.0f, 400, 1);
//         nk_label(ctx, name.c_str(), NK_TEXT_ALIGN_LEFT);
//         
//         if (first) {
//           const float ratio[] = {0.6, 0.4};
//           nk_layout_row(ctx, NK_DYNAMIC, 30.0f, 2, ratio);
//           nk_edit_string(ctx, NK_EDIT_SIMPLE, buffer, &buffer_len, 11, nk_filter_decimal);
//           if (nk_button_label(ctx, "Randomize")) {
//             std::random_device dev;
//             random_seed = dev();
//             const std::string str = fmt::format(FMT_STRING("{}"), random_seed);
// //                 std::cout << "Randomized " << str << " size " << str.size() << " length " << str.length() << "\n";
//             memcpy(buffer, str.c_str(), str.size());
//             buffer_len = str.length();
//           }
//           
//           buffer[buffer_len] = '\0';
//           const size_t num = atol(buffer);
// //               ASSERT(num != 0);
//           
//           if (num > UINT32_MAX) {
//             random_seed = UINT32_MAX;
//           } else {
//             random_seed = num;
//           }
//           
//           if (num > UINT32_MAX) {
//             const std::string str = fmt::format(FMT_STRING("{}"), random_seed);
//             memcpy(buffer, str.c_str(), str.size());
//             buffer_len = str.size();
//           }
//         }
//         
//         for (auto p : variables) {
//           p->draw(table);
//         }
//         
//         nk_layout_row_static(ctx, 30.0f, 199, 2);
//         if (first) nk_spacing(ctx, 1);
//         else {
//           if (nk_button_label(ctx, "Back")) code = -1;
//         }
//         
//         if (nk_button_label(ctx, "Generate")) {
//           gen.clear();
//           for (const auto &pair : pairs) {
//             gen.add(pair);
//           }
//           
//           if (first) {
//             union transform {
//               uint32_t valu;
//               int32_t vali;
//             };
//             
//             transform t;
//             t.valu = random_seed;
//             
//             context->random->set_seed(random_seed);
//             context->noise->SetSeed(t.vali);
//           }
//           
//           gen.generate(context, table);
//           render::mode(rendering_mode);
//           code = 1;
//         }
//       }
//       nk_end(ctx);
//       
//       return code;
//     }
    
    const sol::function & step::get_interface() const {
      return interface;
    }
    
    const std::vector<map::generator_pair> & step::get_functions() const {
      return pairs;
    }
    
    creator::creator(utils::interface* interface) : table(lua.create_table()), current_step(0), old_step(0), random(1), noise(1), interface(interface) {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::package, sol::lib::string);
      ctx.container = &temp_container;
      ctx.map = global::get<core::map>();
      ctx.noise = &noise;
      ctx.random = &random;
      ctx.pool = global::get<dt::thread_pool>();
      
      table["userdata"] = lua.create_table();
      global::get(&m_table_container);
      global::get(&lua);
      
      const std::string path = global::root_directory() + "scripts/";
      lua.require_file("serpent", path + "serpent.lua", true); //auto obj = 
      
      // нужно тут создать интерфейс тоже, только по всей видимости почти без данных
      // все таки придется сериализовать таблицу
      interface->get_state().require_file("serpent", path + "serpent.lua");
      interface_table = interface->get_state().create_table();
      auto gen_table = interface->get_state()["generator"].get_or_create<sol::table>();
      gen_table["setup_random_seed"] = [this] (const uint32_t &seed) {
        this->random.set_seed(seed);
      };
      
      gen_table["setup_noise_seed"] = [this] (const uint32_t &seed) {
        this->noise.SetSeed(*reinterpret_cast<const int*>(&seed));
      };
      
      gen_table["get_random_int"] = [] () {
        std::random_device dev;
        return dev();
      };
    }
    
    creator::~creator() {
      for (auto p : steps) {
        steps_pool.destroy(p);
      }
      
      global::get<utils::table_container>(reinterpret_cast<utils::table_container*>(SIZE_MAX));
      global::get<sol::state>(reinterpret_cast<sol::state*>(SIZE_MAX));
      ASSERT(global::get<utils::table_container>() == nullptr);
      
      auto gen_table = interface->get_state()["generator"].get_or_create<sol::table>();
      gen_table["setup_random_seed"] = sol::nil;
      gen_table["setup_noise_seed"] = sol::nil;
      gen_table["get_random_int"] = sol::nil;
      
      for (const auto &name : clearing_sol_state) {
        interface->get_state()[name] = sol::nil;
      }
    }
    
//     step* creator::create(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode) {
//       auto step = steps_pool.create(first, container_size, name, pairs, rendering_mode);
//       steps.push_back(step);
//       return step;
//     }
    
    step* creator::create(const std::string_view &interface_name, const std::vector<map::generator_pair> &pairs) { //const bool first, 
      const sol::function &func = interface->get_state()[interface_name];
      clearing_sol_state.insert(std::string(interface_name));
      std::vector<map::generator_pair> final_pairs;
      // это первый степ, мы можем сюда добавить функцию бегин
      if (steps.empty()) final_pairs.push_back(map::default_generator_pairs[0]);
      
      final_pairs.insert(final_pairs.end(), pairs.begin(), pairs.end());
      auto step = steps_pool.create(func, final_pairs);
      steps.push_back(step);
      return step;
    }
    
    sol::table pass_to_other_state(sol::state_view first, const sol::table &table, sol::state_view second) {
      // таблицу в строку и передаем в другую таблицу
      const auto &serializator = first["serpent"];
      if (!serializator.valid()) throw std::runtime_error("Could not load serializator");
      const sol::function &func = serializator["line"];
      if (!func.valid()) throw std::runtime_error("Bad serializator");
      auto opts = first.create_table();
      opts["compact"] = true;
      opts["fatal"] = true;
      opts["comment"] = false;
      auto ret = func(table, opts);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Could not serialize lua table");
      }
      
      std::string value = ret;
//       PRINT_VAR("ser value", value) 
      auto res = second.safe_script("return " + value); // value выглядит так: {table_data1=2334,table_data2=1241}
      if (!res.valid()) {
        sol::error err = res;
        std::cout << err.what();
        throw std::runtime_error("Small script error");
      }
      //if (!second["global_tmp_table"].valid()) throw std::runtime_error("Bad context changing");
      return res;
    }
    
    void creator::generate() {
      if (old_step == current_step) {
        // по всей видимости нужно отделить интерфейс от контейнера, для разных частей программы интерфейс разный
        // разные данные нужно передать и разные данные нужно вернуть, некая утилити таблица с интерфейсами?
        auto ret_lua = steps[current_step]->get_interface()(interface->get_ctx(), interface_table);
        if (!ret_lua.valid()) {
          sol::error err = ret_lua;
          std::cout << err.what();
          throw std::runtime_error("Bad lua function result. Step " + std::to_string(current_step));
        }
        
        int32_t ret = ret_lua;
        ret = std::min(ret,  1);
        ret = std::max(ret, -1);
        current_step += ret;
        if (current_step != old_step && current_step >= 0) {
          table["userdata"] = pass_to_other_state(interface->get_state(), interface_table, lua);
          gen.clear();
          for (const auto &pair : steps[old_step]->get_functions()) {
            gen.add(pair);
          }
          
          auto generator = &gen;
          auto ctx_ptr = &ctx;
          auto table_ptr = &table;
          global::get<dt::thread_pool>()->submitbase([generator, ctx_ptr, table_ptr] () {
            generator->generate(ctx_ptr, *table_ptr);
          });
        }
        
        return;
      }
      
      if (gen.finished()) {
        old_step = current_step;
        return;
      }
      
      auto info_table = interface->get_state()["tmp_table"].get_or_create<sol::table>(); // нужно ли как то ограничить то что происходит внутри?
      info_table["current_step"] = gen.current();
      info_table["hint"] = gen.hint();
      info_table["step_count"] = gen.size();
      info_table["current_generator_part"] = old_step;
      auto res = progress_interface_func(interface->get_ctx(), info_table); // контекст и текущая итерация
      if (!res.valid()) {
        sol::error err = res;
        std::cout << err.what();
        throw std::runtime_error("Progress bar interface function error");
      }
    }
    
    sol::table & creator::get_table() {
      return table;
    }
    
    bool creator::finished() const {
      return size_t(old_step) == steps.size();
    }
    
    bool creator::back_to_menu() const {
      return current_step < 0;
    }
    
    void creator::run_script(const std::string_view &path) {
      std::filesystem::path p(path);
      std::filesystem::directory_entry e(p);
      if (!e.exists()) throw std::runtime_error("Script " + std::string(path) + " does not exist");
      if (p.extension() != ".lua") throw std::runtime_error("Bad script extension. " + std::string(path));
      if (!e.is_regular_file()) throw std::runtime_error("Bad script file. " + std::string(path));
      
      auto res = lua.safe_script_file(p);
      if (!res.valid()) {
        sol::error e = res;
        std::cout << e.what() << "\n";
        throw std::runtime_error("Could not load script " + std::string(path));
      }
    }
    
    void creator::run_interface_script(const std::string_view &path) {
      std::filesystem::path p(path);
      std::filesystem::directory_entry e(p);
      if (!e.exists()) throw std::runtime_error("Script " + std::string(path) + " does not exist");
      if (p.extension() != ".lua") throw std::runtime_error("Bad script extension. " + std::string(path));
      if (!e.is_regular_file()) throw std::runtime_error("Bad script file. " + std::string(path));
      
      auto res = interface->get_state().safe_script_file(p);
      if (!res.valid()) {
        sol::error e = res;
        std::cout << e.what() << "\n";
        throw std::runtime_error("Could not load script " + std::string(path));
      }
    }
    
    void creator::progress_interface(const std::string_view &name) {
      clearing_sol_state.insert(std::string(name));
      progress_interface_func = interface->get_state()[name];
    }
    
    sol::state & creator::state() {
      return lua;
    }
    
    utils::table_container & creator::table_container() {
      return m_table_container;
    }
  }
}
