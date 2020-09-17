#include "interface2.h"
#include <stdexcept>
#include "core_structures.h"
#include "helper.h"
#include "interface_context.h"
#include "utils/lua_initialization.h"

namespace devils_engine {
  namespace utils {
    timer::timer() : current_time(SIZE_MAX), end_time(0) {}
    timer::timer(const size_t &end_time) : current_time(0), end_time(end_time) {}
    bool timer::valid() const { return end_time != 0; }
    size_t timer::current() const { return current_time; }
    size_t timer::end() const { return end_time; }
    double timer::norm() const { return valid() ? double(current_time) / double(end_time) : 0.0; }
    void timer::reset() { current_time = 0; }
    void timer::update(const size_t &time) { current_time += valid() ? time : 0; }
    void timer::set_invalid() { end_time = 0; }
    
    interface::interface() {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::utf8, sol::lib::bit32, sol::lib::package);
      // как закрыть доступ к io.write(), нужно по всей видимости использовать сэндбокс
//       if (!light_interface) {
//         setup_lua_types(lua);
//         setup_lua_game_logic(lua);
//       }
//       
//       setup_lua_constants(lua);
//       setup_lua_input(lua);
      
//       size_t counter = 0;
//       if (lua["constants"].valid()) {
//         PRINT("constants table are valid")
//       } else ++counter;
//       
//       if (lua["utils"].valid()) {
//         PRINT("utils table are valid")
//       } else ++counter;
//       
//       if (lua["core"].valid()) {
//         PRINT("core table are valid")
//       } else ++counter;
//       
//       if (lua["input"].valid()) {
//         PRINT("input table are valid")
//       } else ++counter;
//       
//       if (lua["constants"]["one_second"] == ONE_SECOND) {
//         PRINT("constants table has one_second value")
//       } else ++counter;
//       
//       if (lua["input"]["repeated"] == input::repeated) {
//         PRINT("input table has repeated value")
//       } else ++counter;
//       
//       if (lua["core"]["character_stats"]["agility"] == core::character_stats::agility) {
//         PRINT("core table has agility value")
//       } else ++counter;
//       
//       if (counter != 0) throw std::runtime_error("wtf");
      
      setup_lua_package_path(lua);
      
      const std::string script_dir = global::root_directory() + "scripts/";
      // в донт старв тогезер скрипты загружались в специальном сэндбоксе, который во первых не позволял 
      // загружать бинарники, а также запрещал использование некоторых функций (например os.execute)
      // а io.open путь выходящий за пределы папки с игрой приводил к ошибке, мне нужно сделать примерно то же самое
//       lua.require_file("nk", script_dir + "moonnuklear.so", true, sol::load_mode::binary); // не работает, либо нужно делать сэндбокс (сол энвайронмент?), либо разрешить тогда уж че там
      auto script_from_file_result = lua.safe_script_file(script_dir + "interface_init.lua"); 
      if (!script_from_file_result.valid()) {
        sol::error err = script_from_file_result;
        throw std::runtime_error("Error in interface_init.lua file: " + std::string(err.what()));
      }
      
      {
        sol::protected_function func = lua["init_nk_context"];
        auto ctx = &global::get<devils_engine::interface::context>()->ctx;
        auto res = func(sol::make_light(ctx));
        if (!res.valid()) {
          sol::error err = res;
          std::string what = err.what();
          std::cout << what << std::endl;
          throw std::runtime_error("Could not make interface context");
        }
        
        sol::object obj = res.get<sol::object>();
        moonnuklear_ctx = obj; // нужно наверное положить в другое место
        lua["init_nk_context"] = sol::nil;
      }
      
      {
        sol::protected_function func = lua["init_nk_font"];
        for (uint32_t i = 0; i < fonts::count; ++i) {
          auto font = global::get<devils_engine::interface::context>()->fonts[i];
          if (font == nullptr) continue;
          auto res = func(sol::make_light(font));
          if (!res.valid()) {
            sol::error err = res;
            std::string what = err.what();
            std::cout << what << std::endl;
            throw std::runtime_error("Could not make interface font");
          }
          
          fonts[i] = res.get<sol::object>();
        }
        
        lua["init_nk_font"] = sol::nil;
      }
    }
    
    interface::~interface() {}
    
    timer* interface::create_timer(const size_t &end_time) {
      for (size_t i = 0; i < timers_count; ++i) {
        if (used_timers.get(i)) continue;
        timers[i] = timer(end_time);
        used_timers.set(i, true);
        return &timers[i];
      }
      
      return nullptr;
    }
    
    void interface::release_timer(timer* t) {
      if (t < timers.data()) throw std::runtime_error("Bad timer ptr");
      const size_t index = t - timers.data();
      if (index >= timers_count) throw std::runtime_error("Bad timer ptr");
      timers[index].set_invalid();
      used_timers.set(index, false);
    }
    
    bool interface::is_layer_visible(const uint32_t &index) const {
      if (index >= window_types_count) throw std::runtime_error("Bad window index");
      return windows.get(index);
    }
    
    void interface::close_layer(const uint32_t &index) {
      if (index >= window_types_count) throw std::runtime_error("Bad window index");
      close_windows.set(index, true);
    }
    
    void interface::draw(const size_t &time) {
      for (size_t i = 0; i < timers_count; ++i) {
        timers[i].update(time);
      }
      
      for (size_t i = 0 ; i < window_types_count; ++i) {
        if (!windows.get(i)) continue;
//         using devils_engine::interface::context;
        //auto ctx_ptr = &global::get<context>()->ctx;
        const auto res = current_windows[i].second->window(moonnuklear_ctx, this, current_windows[i].first);
        // в муннаклире используется своя структура поверх структуры наклира
//         auto ctx_ptr = global::get<context>()->moonnuklear_ctx;
//         auto font_ptr = global::get<context>()->moonnuklear_font;
//         const auto res = current_windows[i].second->window(ctx_ptr, font_ptr, current_windows[i].first);
        if (!res.valid()) {
          const sol::error err = res;
          const std::string what = err.what();
          std::cout << what << std::endl;
          throw std::runtime_error("Lua function error");
        }
        
        const bool close = res.get<bool>();
        if (close) close_windows.set(i, true);
      }
      
      for (size_t i = 0 ; i < window_types_count; ++i) {
        if (!close_windows.get(i)) continue;
        windows.set(i, false);
        close_windows.set(i, false);
        current_windows[i].first = sol::object(sol::nil);
        current_windows[i].second = nullptr;
      }
      
      for (size_t i = 0 ; i < window_types_count; ++i) {
        if (open_windows[i].second == nullptr) continue;
//         ASSERT(open_windows[i].first != nullptr); // пока что передаю нулл
        // нужно ли заставлять закрывать окна? то есть обязательно возвращать true?
        // это поможет сориентироваться, нужно потом добавить ошибку при не закрытии окна
        current_windows[i] = open_windows[i];
        windows.set(i, true);
        open_windows[i] = std::make_pair(sol::nil, nullptr);
      }
    }
    
    // понятное дело registered_windows не должен меняться во время игры
    void interface::open_layer(const std::string &window_id, sol::object data) {
      auto itr = registered_windows.find(window_id);
      if (itr == registered_windows.end()) throw std::runtime_error("Could not find gui window " + window_id);
      ASSERT(itr->second.type < window_types_count);
      
      // тут мы можем проверить тип того что к нам приходит
      // хотя теперь когда мы используем сол объект зачем нам это?
      // 
      
      const size_t index = itr->second.type;
      open_windows[index] = std::make_pair(data, &itr->second);
    }
    
    bool interface::update_data(const uint32_t &layer_id, sol::object data) {
      if (layer_id >= window_types_count) throw std::runtime_error("Bad layer id");
      if (current_windows[layer_id].second == nullptr) return false;
      current_windows[layer_id].first = data;
      return true;
    }
    
    void interface::register_layer(const std::string &name, const window_info &window) {
      if (window.type >= window_types_count) throw std::runtime_error("Bad window type");
      auto itr = registered_windows.find(name);
      if (itr != registered_windows.end()) throw std::runtime_error("Layer " + name + " is already registered");
      registered_windows[name] = window;
    }
    
    const interface::window_info* interface::get_data(const std::string &name) const {
      auto itr = registered_windows.find(name);
      if (itr != registered_windows.end()) return nullptr;
      return &itr->second;
    }
    
    void interface::clear() {
      registered_windows.clear();
      close_windows.reset();
      windows.reset();
      for (size_t i = 0; i < window_types_count; ++i) {
        current_windows[i].first = sol::object(sol::nil);
        current_windows[i].second = nullptr;
        open_windows[i].first = sol::object(sol::nil);
        open_windows[i].second = nullptr;
      }
    }
    
    sol::state & interface::get_state() {
      return lua;
    }
    
    sol::object & interface::get_ctx() {
      return moonnuklear_ctx;
    }
    
    void interface::init_constants() {
      setup_lua_constants(lua);
    }
    
    void interface::init_input() {
      setup_lua_input(lua);
    }
    
    void interface::init_types() {
      setup_lua_types(lua);
    }
    
    void interface::init_game_logic() {
      setup_lua_game_logic(lua);
    }
  }
}
