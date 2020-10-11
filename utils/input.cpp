#include "input.h"

#include "globals.h"
#include "shared_time_constant.h"
#include <cstring>
#include <iostream>
#include "render/window.h"

namespace devils_engine {
  namespace utils {
    static player_states current_player_state_value = player_in_menu;
    player_states current_player_state() {
      return current_player_state_value;
    }
    
    void set_player_state(const player_states &state) {
      // имеет смысл резетнуть состояния кнопок в этот момент
      current_player_state_value = state;
    }
  }
  
  namespace input {
    const char* key_names[key_count] = {
      "mouse1", // 0
      "mouse2",
      "mouse3",
      "mouse4",
      "mouse5",
      "mouse6", // 5
      "mouse7",
      "mouse8",
      nullptr,
      nullptr,
      nullptr, // 10
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr, // 15
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr, // 20
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr, // 25
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr, // 30
      nullptr,
      "space", // GLFW_KEY_SPACE
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      "'", // 39
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      ",", // 44
      "-",
      ".",
      "/",
      "0",
      "1",
      "2",
      "3",
      "4",
      "5",
      "6",
      "7",
      "8",
      "9",
      nullptr,
      ";", // 59
      nullptr,
      "=", // 61
      nullptr,
      nullptr,
      nullptr,
      "a", // 65
      "b",
      "c",
      "d",
      "e",
      "f",
      "g",
      "h",
      "i",
      "j",
      "k",
      "l",
      "m",
      "n",
      "o",
      "p",
      "q",
      "r",
      "s",
      "t",
      "u",
      "v",
      "w",
      "x",
      "y",
      "z",
      "[",
      "\\",
      "]",
      nullptr,
      nullptr,
      "`", // 96
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      "w1", // 161
      "w2",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      "esc", // 256
      "enter",
      "tab",
      "backspace",
      "insert",
      "delete",
      "right",
      "left",
      "down",
      "up",
      "page_up",
      "page_down",
      "home",
      "end",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      "caps_lock", // 280
      "scroll_lock",
      "num_lock",
      "print",
      "pause",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      "f1", // 290
      "f2",
      "f3",
      "f4",
      "f5",
      "f6",
      "f7",
      "f8",
      "f9",
      "f10",
      "f11",
      "f12",
      "f13",
      "f14",
      "f15",
      "f16",
      "f17",
      "f18",
      "f19",
      "f20",
      "f21",
      "f22",
      "f23",
      "f24",
      "f25",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      "num_0", // 320
      "num_1",
      "num_2",
      "num_3",
      "num_4",
      "num_5",
      "num_6",
      "num_7",
      "num_8",
      "num_9",
      "num_decimal",
      "num_/",
      "num_*",
      "num_-",
      "num_+",
      "num_enter",
      "num_=",
      nullptr,
      nullptr,
      nullptr,
      "l_shift", // 340
      "l_ctrl",
      "l_alt",
      "l_super",
      "r_shift",
      "r_ctrl",
      "r_alt",
      "r_super",
      "menu"
    };

    event_data::event_data() {}
    event_data::event_data(const utils::id &id) : id(id) {}
    key_data::key_data() : event(release), state(state_initial), event_layer(UINT32_MAX), state_time(0) {}
    data::data() : interface_focus(false), mouse_wheel(0.0f), current_text(0), last_frame_time(0) {
      memset(text, 0, sizeof(uint32_t)*text_memory_size);
    }

    input_event next_input_event(size_t &mem) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = mem; i < container_size; ++i) {
        if (container.container[i].data[utils::current_player_state()].id.valid() && container.container[i].event_time <= 5) { // первый кадр нажатия это 0
          mem = i+1;
          return {container.container[i].data[utils::current_player_state()].id, container.container[i].event};
        }
      }

      return {utils::id(), release};
    }

    input_event next_input_event(size_t &mem, const size_t &tolerancy) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = mem; i < container_size; ++i) {
        //if (i == 290 && container.container[i].event_time <= tolerancy) { ASSERT(container.container[i].data[utils::current_player_state()].id.valid()); }
        if (container.container[i].data[utils::current_player_state()].id.valid() && container.container[i].event_time <= tolerancy) {
          mem = i+1;
          return {container.container[i].data[utils::current_player_state()].id, container.container[i].event};
        }
      }

      return {utils::id(), release};
    }

//     struct input_event_state input_event_state(const utils::id &id) {
//       const auto &container = global::get<data>()->key_events;
//       auto itr = container.event_keys.find(id);
//       if (itr == container.event_keys.end()) return {utils::id(), release, SIZE_MAX};
//       if (itr->second.keys[1] == INT32_MAX) {
//         const int index = itr->second.keys[0];
//         return {id, container.container[index].event, container.container[index].event_time};
//       }
//       
//       if (container.container[itr->second.keys[0]].event_time < container.container[itr->second.keys[1]].event_time) {
//         const int index = itr->second.keys[0];
//         return {id, container.container[index].event, container.container[index].event_time};
//       }
//       
//       const int index = itr->second.keys[1];
//       return {id, container.container[index].event, container.container[index].event_time};
//     }
    
    std::tuple<type, size_t> input_event_state(const utils::id &id) {
      const auto &container = global::get<data>()->key_events;
      auto itr = container.event_keys.find(id);
      if (itr == container.event_keys.end()) return {release, SIZE_MAX};
      
      int index = INT32_MAX;
      if (itr->second.keys[1] == INT32_MAX) index = itr->second.keys[0];
      else if (container.container[itr->second.keys[0]].event_time < container.container[itr->second.keys[1]].event_time) index = itr->second.keys[0]; // стейт тайм?
      else index = itr->second.keys[1];
      ASSERT(index != INT32_MAX);
      
      return {container.container[index].event, container.container[index].event_time};
    }

    bool is_event_pressed(const utils::id &id) {
      const auto &container = global::get<data>()->key_events;
      auto itr = container.event_keys.find(id);
      if (itr == container.event_keys.end()) return false;
      if (itr->second.keys[1] == INT32_MAX) {
        const int index = itr->second.keys[0];
        return container.container[index].event != release;
      }
      
      if (container.container[itr->second.keys[0]].event_time < container.container[itr->second.keys[1]].event_time) {
        const int index = itr->second.keys[0];
        return container.container[index].event != release;
      }
      
      const int index = itr->second.keys[1];
      return container.container[index].event != release;
    }

    bool is_event_released(const utils::id &id) {
      const auto &container = global::get<data>()->key_events;
      auto itr = container.event_keys.find(id);
      if (itr == container.event_keys.end()) return false;
      if (itr->second.keys[1] == INT32_MAX) {
        const int index = itr->second.keys[0];
        return container.container[index].event == release;
      }
      
      if (container.container[itr->second.keys[0]].event_time < container.container[itr->second.keys[1]].event_time) {
        const int index = itr->second.keys[0];
        return container.container[index].event == release;
      }
      
      const int index = itr->second.keys[1];
      return container.container[index].event == release;
    }

    void update_time(const size_t &time) {
      global::get<data>()->key_events.current_event_layer = 0;
      auto container = global::get<data>()->key_events.container;
      global::get<data>()->last_frame_time = time;
      for (size_t i = 0; i < key_count; ++i) {
        container[i].state_time += time;
        container[i].event_time += time;
        container[i].event_layer = UINT32_MAX;
        switch (container[i].state) {
          case state_initial: {
            const bool press = container[i].event != release;
            container[i].state = press ? state_press : container[i].state;
            container[i].state_time = press ? 0 : container[i].state_time;
            break;
          }
          
          case state_press: {
            const bool press = container[i].event != release;
            if (press && container[i].state_time >= long_press_time) {
              container[i].state = state_long_press;
              container[i].state_time = 0;
              break;
            }
            
            container[i].state = !press ? state_click : container[i].state;
            container[i].state_time = !press ? 0 : container[i].state_time;
            break;
          }
          
          case state_click: {
            const bool press = container[i].event != release;
            if (press && container[i].state_time < double_press_time) {
              container[i].state = state_double_press;
              container[i].state_time = 0;
              break;
            }
            
            if (container[i].state_time >= double_press_time) { // по идее мы ждем тут какое то время, понятное дело нужно предусмотреть это дело в других функциях
              container[i].state = state_initial;
              container[i].state_time = 0;
            }
            break;
          }
          
          case state_double_press: {
            const bool press = container[i].event != release;
            if (press && container[i].state_time >= long_press_time) {
              container[i].state = state_long_press;
              container[i].state_time = 0;
              break;
            }
            
            container[i].state = !press ? state_double_click : container[i].state;
            container[i].state_time = !press ? 0 : container[i].state_time;
            break;
          }
          
          case state_double_click: {
            if (container[i].state_time != 0) {
              container[i].state = state_initial;
              container[i].state_time = 0;
            }
            break;
          }
          
          case state_long_press: {
            const bool press = container[i].event != release;
            container[i].state = !press ? state_long_click : container[i].state;
            container[i].state_time = !press ? 0 : container[i].state_time;
            break;
          }
          
          case state_long_click: {
            if (container[i].state_time != 0) {
              container[i].state = state_initial;
              container[i].state_time = 0;
            }
            break;
          }
        }
        
//         if (container[i].data == nullptr) continue;
//         container[i].data->time += time;
      }
    }

    const char* get_key_name(const uint32_t &key) {
      if (key >= key_count) return nullptr;
      return key_names[key];
    }
    
    const char* get_event_key_name(const utils::id &id, const uint8_t &slot) {
      assert(slot < event_key_slots);
      const auto &container = global::get<data>()->key_events;
      auto itr = container.event_keys.find(id);
      const int key = itr->second.keys[slot];
      return get_key_name(key);
    }

    std::pair<utils::id, size_t> pressed_event(size_t &mem) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = mem; i < container_size; ++i) {
        if (container.container[i].data[utils::current_player_state()].id.valid() && container.container[i].event != release) {
          mem = i+1;
          return std::make_pair(container.container[i].data[utils::current_player_state()].id, container.container[i].event_time);
        }
      }

      return std::make_pair(utils::id(), SIZE_MAX);
    }
    
    utils::id get_event(const std::string_view &str) {
//       PRINT(str)
      return utils::id::get(str);
    }
    
    void block() { global::get<data>()->key_events.blocked = 1; }
    void unblock() { global::get<data>()->key_events.blocked = 0; }
    void increase_layer() { ++global::get<data>()->key_events.current_event_layer; }

    void set_key(const int &key, const utils::id &id, const uint8_t &slot) {
      assert(slot < event_key_slots);
      auto &container = global::get<data>()->key_events;
      auto itr = container.event_keys.find(id);
      if (itr == container.event_keys.end()) {
        itr = container.event_keys.insert(std::make_pair(id, keys::event_keys_container{INT32_MAX, INT32_MAX})).first;
      }
      
      itr->second.keys[slot] = key;
      // нужно ли запоминать в кнопке что к ней обращается? 
      // по идее мне это нужно только для того чтобы подтвердить что пользователь не прилепил на одну кнопку несколько эвентов
      // думаю что это можно сделать отдельно
      if (key == INT32_MAX) return;
      if (size_t(key) >= container_size) throw std::runtime_error("Bad key index");
      auto &cont = global::get<data>()->key_events.container;
      if (cont[key].data[0].id == id) return;
      auto old_id = cont[key].data[0].id;
      cont[key].data[0].id = id;
      if (!old_id.valid()) return;
      
      ASSERT(false);
      auto event_keys_container = container.event_keys.find(old_id);
      ASSERT(event_keys_container != container.event_keys.end());
      if (event_keys_container->second.keys[0] == key) event_keys_container->second.keys[0] = INT32_MAX;
      if (event_keys_container->second.keys[1] == key) event_keys_container->second.keys[1] = INT32_MAX;
    }

    event_data get_event_data(const int &key) {
      if (uint32_t(key) >= container_size) return event_data();
      const auto &container = global::get<data>()->key_events;
      return container.container[key].data[utils::current_player_state()];
    }
    
    bool check_key(const int &key, const uint32_t &states) {
      if (uint32_t(key) >= container_size) return false;
      if (global::get<data>()->key_events.blocked == 1) return false;
      const auto &container = global::get<data>()->key_events.container;
      const uint32_t final_state = container[key].state == state_click && container[key].state_time != 0 ? state_initial : container[key].state;
      return (final_state & states) != 0;
    }
    
    bool timed_check_key(const int &key, const uint32_t &states, const size_t &wait, const size_t &period) {
      if (uint32_t(key) >= container_size) return false;
      if (global::get<data>()->key_events.blocked == 1) return false;
      const auto &container = global::get<data>()->key_events.container;
      const size_t last_frame_time = global::get<data>()->last_frame_time;
//       if (container[key].state_time < wait) return false;
//       if (container[key].state_time % period >= frame_time) return false;
      if (container[key].event_time < wait) return false;
      if (container[key].event_time % period >= last_frame_time) return false;
      const uint32_t final_state = container[key].state == state_click && container[key].state_time != 0 ? state_initial : container[key].state;
      return (final_state & states) != 0;
    }
    
    bool check_event(const utils::id &event, const uint32_t &states) {
      auto &container = global::get<data>()->key_events;
      if (container.blocked == 1) return false;
      auto itr = container.event_keys.find(event);
      if (itr == container.event_keys.end()) return false;
      
      int index = INT32_MAX;
      if (itr->second.keys[1] == INT32_MAX) index = itr->second.keys[0];
      else if (container.container[itr->second.keys[0]].state_time < container.container[itr->second.keys[1]].state_time) index = itr->second.keys[0]; // стейт тайм?
      else index = itr->second.keys[1];
      
      if (container.container[index].event_layer == UINT32_MAX) container.container[index].event_layer = container.current_event_layer;
      if (container.container[index].event_layer != container.current_event_layer) return false;
      
      const uint32_t final_state = container.container[index].state == state_click && container.container[index].state_time != 0 ? state_initial : container.container[index].state;
      return (final_state & states) != 0;
    }
    
    bool timed_check_event(const utils::id &event, const uint32_t &states, const size_t &wait, const size_t &period) {
      auto &container = global::get<data>()->key_events;
      if (container.blocked == 1) return false;
      auto itr = container.event_keys.find(event);
      if (itr == container.event_keys.end()) return false;
      
      const size_t last_frame_time = global::get<data>()->last_frame_time;
      
      int index = INT32_MAX;
      if (itr->second.keys[1] == INT32_MAX) index = itr->second.keys[0];
      else if (container.container[itr->second.keys[0]].state_time < container.container[itr->second.keys[1]].state_time) index = itr->second.keys[0]; // стейт тайм?
      else index = itr->second.keys[1];
      
      if (container.container[index].event_layer == UINT32_MAX) container.container[index].event_layer = container.current_event_layer;
      if (container.container[index].event_layer != container.current_event_layer) return false;
      
//       if (index == 264 && container.container[index].event_time < 10000) PRINT(container.container[index].event_time)
      if (container.container[index].event_time < wait) return false;
      if (container.container[index].event_time % period >= last_frame_time) return false;
      const uint32_t final_state = container.container[index].state == state_click && container.container[index].state_time != 0 ? state_initial : container.container[index].state;
      return (final_state & states) != 0;
    }
    
    std::tuple<uint32_t, uint32_t> get_framebuffer_size() {
      auto w = global::get<render::window>();
      return {w->surface.extent.width, w->surface.extent.height};
    }
    
    std::tuple<float, float> get_window_content_scale() {
      return global::get<render::window>()->content_scale();
    }
    
    std::tuple<float, float> get_monitor_content_scale() {
      return global::get<render::window>()->monitor_content_scale();
    }
    
    std::tuple<int32_t, int32_t> get_monitor_physical_size() {
      return global::get<render::window>()->monitor_physical_size();
    }
  }
}
