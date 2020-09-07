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
    double timer::norm() const { return valid() ? std::min(double(current_time) / double(end_time), 1.0) : 0.0; }
    void timer::reset() { current_time = 0; }
    void timer::update(const size_t &time) { current_time += valid() ? time : 0; }
    void timer::set_invalid() { end_time = 0; }
    
    interface::interface() {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::package);
      setup_lua_types(lua);
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
    
    bool interface::is_window_type_visible(const uint32_t &index) const {
      if (index >= window_types_count) throw std::runtime_error("Bad window index");
      return windows.get(index);
    }
    
    void interface::close_window(const uint32_t &index) {
      if (index >= window_types_count) throw std::runtime_error("Bad window index");
      close_windows.set(index, true);
    }
    
    void interface::draw(const size_t &time) {
      for (size_t i = 0; i < timers_count; ++i) {
        timers[i].update(time);
      }
      
      for (size_t i = 0 ; i < window_types_count; ++i) {
        if (!windows.get(i)) continue;
        using devils_engine::interface::context;
        auto ctx_ptr = &global::get<context>()->ctx;
        //const bool close = current_windows[i].second->window(ctx_ptr, this, current_windows[i].first);
        const auto res = current_windows[i].second->window(ctx_ptr, this, current_windows[i].first);
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
        current_windows[i].first = nullptr;
        current_windows[i].second = nullptr;
      }
      
      for (size_t i = 0 ; i < window_types_count; ++i) {
        if (open_windows[i].second == nullptr) continue;
//         ASSERT(open_windows[i].first != nullptr); // пока что передаю нулл
        // нужно ли заставлять закрывать окна? то есть обязательно возвращать true?
        // это поможет сориентироваться, нужно потом добавить ошибку при не закрытии окна
        current_windows[i] = open_windows[i];
        windows.set(i, true);
        open_windows[i] = std::make_pair(nullptr, nullptr);
      }
    }
    
    // понятное дело registered_windows не должен меняться во время игры
    void interface::open_window(const std::string &window_id, const void* data) {
      auto itr = registered_windows.find(window_id);
      if (itr == registered_windows.end()) throw std::runtime_error("Could not find gui window " + window_id);
      ASSERT(itr->second.type < window_types_count);
      const size_t index = itr->second.type;
      open_windows[index] = std::make_pair(data, &itr->second);
    }
    
    void interface::register_window(const std::string &name, const window_info &window) {
      if (window.type >= window_types_count) throw std::runtime_error("Bad window type");
      auto itr = registered_windows.find(name);
      if (itr != registered_windows.end()) throw std::runtime_error("Window " + name + " is already registered");
      registered_windows[name] = window;
    }
    
    sol::state & interface::get_state() {
      return lua;
    }
  }
}
