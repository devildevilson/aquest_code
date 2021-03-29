#include "input.h"

#include "globals.h"
#include "shared_time_constant.h"
#include <cstring>
#include <iostream>
#include "render/window.h"
#include "GLFW/glfw3.h"

namespace devils_engine {
  namespace utils {
//     static player_states current_player_state_value = player_in_menu;
//     player_states current_player_state() {
//       return current_player_state_value;
//     }
//     
//     void set_player_state(const player_states &state) {
//       // имеет смысл резетнуть состояния кнопок в этот момент
//       current_player_state_value = state;
//     }
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
    
    static std::atomic<keys*> current_keys_map(nullptr);
    static data input_data;
    
    void set_key_map(keys* k) {
      current_keys_map = k;
    }
    
    keys* get_key_map() {
      return current_keys_map;
    }
    
    data* get_input_data() {
      return &input_data;
    }

    input_event next_input_event(size_t &mem) {
      const auto data = get_input_data();
      if (data->input_blocked) return {utils::id(), release};
      
      const auto &key_container = data->container;
      const auto &key_event_map = get_key_map()->container;
      for (size_t i = mem; i < container_size; ++i) {
        if (key_event_map[i].valid() && key_container[i].event_time <= 5) { // первый кадр нажатия это 0
          mem = i+1;
          return {key_event_map[i], key_container[i].event};
        }
      }

      return {utils::id(), release};
    }

    input_event next_input_event(size_t &mem, const size_t &tolerancy) {
      const auto data = get_input_data();
      if (data->input_blocked) return {utils::id(), release};
      
      const auto &key_container = data->container;
      const auto &key_event_map = get_key_map()->container;
      for (size_t i = mem; i < container_size; ++i) {
        const auto id = key_event_map[i];

        if (id.valid() && key_container[i].event_time <= tolerancy) {
          mem = i+1;
          return {id, key_container[i].event};
        }
      }

      return {utils::id(), release};
    }
    
    std::tuple<type, size_t> input_event_state(const utils::id &id) {
      const auto data = get_input_data();
      if (data->input_blocked) return { release, SIZE_MAX };
      
      const auto container = get_key_map();
      auto itr = container->events_map.find(id);
      if (itr == container->events_map.end()) return { release, SIZE_MAX };
      
      const auto &key_container = data->container;
      const auto &found_event = itr->second;
      int index = INT32_MAX;
      {
        const int index1 = found_event.keys[0];
        const int index2 = found_event.keys[1];
        const bool keys_set = index1 != INT32_MAX && index2 != INT32_MAX;
        index = index1 == INT32_MAX ? index2 : index1;
        index = keys_set ? (key_container[index1].state_time < key_container[index2].state_time ? index1 : index2) : index;
      }
      ASSERT(index != INT32_MAX);
      
      return {key_container[index].event, key_container[index].event_time};
    }

    bool is_event_pressed(const utils::id &id) {
      const auto data = get_input_data();
      if (data->input_blocked) return false;
      
      const auto container = get_key_map();
      
      auto itr = container->events_map.find(id);
      if (itr == container->events_map.end()) return false;
      
      const auto &key_container = data->container;
      const auto &found_event = itr->second;
      if (found_event.keys[1] == INT32_MAX) {
        const int index = found_event.keys[0];
        return key_container[index].event != release;
      }
      
      if (key_container[found_event.keys[0]].event_time < key_container[found_event.keys[1]].event_time) {
        const int index = found_event.keys[0];
        return key_container[index].event != release;
      }
      
      const int index = found_event.keys[1];
      return key_container[index].event != release;
    }

    bool is_event_released(const utils::id &id) {
      const auto data = get_input_data();
      if (data->input_blocked) return true;
      
      const auto container = get_key_map();
      
      auto itr = container->events_map.find(id);
      if (itr == container->events_map.end()) return false;
      
      const auto &key_container = data->container;
      const auto& found_event = itr->second;
      if (found_event.keys[1] == INT32_MAX) {
        const int index = found_event.keys[0];
        return key_container[index].event == release;
      }
      
      if (key_container[found_event.keys[0]].event_time < key_container[found_event.keys[1]].event_time) {
        const int index = found_event.keys[0];
        return key_container[index].event == release;
      }
      
      const int index = found_event.keys[1];
      return key_container[index].event == release;
    }

    void update_time(const size_t &time) {
      auto data = get_input_data();
      const auto key_event_container = get_key_map();
      auto &container = data->container;
      
      key_event_container->current_event_layer = 0;
      data->last_frame_time = time;
      
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
      auto data = get_input_data();
      if (data->input_blocked) return nullptr;
      
      const auto container = get_key_map();
      
      assert(slot < event_key_slots);
      auto itr = container->events_map.find(id);
      if (itr == container->events_map.end()) return nullptr;

      const auto& found_event = itr->second;
      const int key = found_event.keys[slot];
      return get_key_name(key);
    }

    std::pair<utils::id, size_t> pressed_event(size_t &mem) {
      auto data = get_input_data();
      if (data->input_blocked) return std::make_pair(utils::id(), SIZE_MAX);
      
      const auto &key_container = data->container;
      const auto &key_event_map = get_key_map()->container;
      for (size_t i = mem; i < container_size; ++i) {
        if (key_event_map[i].valid() && key_container[i].event != release) {
          mem = i+1;
          return std::make_pair(key_event_map[i], key_container[i].event_time);
        }
      }

      return std::make_pair(utils::id(), SIZE_MAX);
    }
    
    utils::id get_event(const std::string_view &str) {
      return utils::id::get(str);
    }
    
    void block() { get_input_data()->input_blocked = true; }
    void unblock() { get_input_data()->input_blocked = false; }
    void increase_layer() { ++get_key_map()->current_event_layer; }

    void set_key(const int &key, const utils::id &id, const uint8_t &slot) {
      assert(slot < event_key_slots);
      ASSERT(id.valid());
      
      // хотя эта функция изменится
      auto data = get_input_data();
      if (data->input_blocked) throw std::runtime_error("Set keys to blocked input");
      
      auto container = get_key_map();
      auto itr = container->events_map.find(id);
      if (itr == container->events_map.end()) {
        itr = container->events_map.emplace(id, keys::event_keys_container{INT32_MAX, INT32_MAX}).first;
      }
      
      auto &event_cont = itr->second;
      const int prev_key = event_cont.keys[slot];
      event_cont.keys[slot] = key;
      ASSERT(itr->second.keys[slot] == key);
      
      if (prev_key != INT32_MAX) {
        ASSERT(size_t(prev_key) < container_size);
        ASSERT(container->container[prev_key] == id);
        container->container[prev_key] = utils::id();
      }
      
      // нужно ли запоминать в кнопке что к ней обращается? 
      // по идее мне это нужно только для того чтобы подтвердить что пользователь не прилепил на одну кнопку несколько эвентов
      // думаю что это можно сделать отдельно
      if (key == INT32_MAX) return; // так мы по идее должны отвязать кнопку от эвента
      if (size_t(key) >= container_size) throw std::runtime_error("Bad key index");
      
      auto &cont = container->container;
      if (cont[key] == id) return;
      auto old_id = cont[key];
      cont[key] = id;
      if (!old_id.valid()) return;
      
      ASSERT(false);
      auto event_keys_container = container->events_map.find(old_id);
      ASSERT(event_keys_container != container->events_map.end());
      //const size_t old_event_index = event_keys_container->second;
      auto &event_key_cont = event_keys_container->second;
      if (event_key_cont.keys[0] == key) event_key_cont.keys[0] = INT32_MAX;
      if (event_key_cont.keys[1] == key) event_key_cont.keys[1] = INT32_MAX;
      
      // надо бы здесь вернуть что то по чему можено сделать вывод что перезаписалось
    }

    utils::id get_event_data(const int &key) {
      auto data = get_input_data();
      if (data->input_blocked) return utils::id();
      
      if (uint32_t(key) >= container_size) return utils::id();
      const auto container = get_key_map();
      return container->container[key];
    }
    
    bool check_key(const int &key, const uint32_t &states) {
      auto data = get_input_data();
      if (data->input_blocked) return false;
      
      if (uint32_t(key) >= container_size) return false;
      const auto &key_container = data->container;
      const uint32_t final_state = key_container[key].state == state_click && key_container[key].state_time != 0 ? state_initial : key_container[key].state;
      return (final_state & states) != 0;
    }
    
    bool timed_check_key(const int &key, const uint32_t &states, const size_t &wait, const size_t &period) {
      auto data = get_input_data();
      if (data->input_blocked) return false;
      
      if (uint32_t(key) >= container_size) return false;
      const auto &key_container = data->container;
      const size_t last_frame_time = data->last_frame_time;

      if (key_container[key].event_time < wait) return false;
      if (key_container[key].event_time % period >= last_frame_time) return false;
      const uint32_t final_state = key_container[key].state == state_click && key_container[key].state_time != 0 ? state_initial : key_container[key].state;
      return (final_state & states) != 0;
    }
    
    bool check_event(const utils::id &event, const uint32_t &states) {
      auto data = get_input_data();
      if (data->input_blocked) return false;
      
      // ищем эвент
      auto container = get_key_map();
      auto itr = container->events_map.find(event);
      if (itr == container->events_map.end()) return false;
      const auto& found_event = itr->second;
      
      auto &key_container = data->container;
      int index = INT32_MAX;
      {
        const int index1 = found_event.keys[0];
        const int index2 = found_event.keys[1];
        const bool keys_set = index1 != INT32_MAX && index2 != INT32_MAX;
        index = index1 == INT32_MAX ? index2 : index1;
        index = keys_set ? (key_container[index1].state_time < key_container[index2].state_time ? index1 : index2) : index;
      }
      if (index == INT32_MAX) return false;
      
      // задаем слой
      if (key_container[index].event_layer == UINT32_MAX) key_container[index].event_layer = container->current_event_layer;
      if (key_container[index].event_layer != container->current_event_layer) return false;
      
      // проверяем условие
      const uint32_t final_state = key_container[index].state == state_click && key_container[index].state_time != 0 ? state_initial : key_container[index].state;
      return (final_state & states) != 0;
    }
    
    bool timed_check_event(const utils::id &event, const uint32_t &states, const size_t &wait, const size_t &period) {
      auto data = get_input_data();
      if (data->input_blocked) return false;
      
      // ищем эвент
      auto container = get_key_map();
      auto itr = container->events_map.find(event);
      if (itr == container->events_map.end()) return false;
      const auto& found_event = itr->second;
      
      const size_t last_frame_time = data->last_frame_time;
      
      auto &key_container = data->container;
      int index = INT32_MAX;
      {
        const int index1 = found_event.keys[0];
        const int index2 = found_event.keys[1];
        const bool keys_set = index1 != INT32_MAX && index2 != INT32_MAX;
        index = index1 == INT32_MAX ? index2 : index1;
        index = keys_set ? (key_container[index1].state_time < key_container[index2].state_time ? index1 : index2) : index;
      }
      if (index == INT32_MAX) return false;
      
      // задаем слой
      if (key_container[index].event_layer == UINT32_MAX) key_container[index].event_layer = container->current_event_layer;
      if (key_container[index].event_layer != container->current_event_layer) return false;
      
      // проверяем условие
      if (key_container[index].event_time < wait) return false;
      if (key_container[index].event_time % period >= last_frame_time) return false;
      const uint32_t final_state = key_container[index].state == state_click && key_container[index].state_time != 0 ? state_initial : key_container[index].state;
      return (final_state & states) != 0;
    }
    
    std::tuple<double, double> get_cursor_pos() {
      auto w = global::get<render::window>();
      double x,y;
      glfwGetCursorPos(w->handle, &x, &y);
      return std::tie(x, y);
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
