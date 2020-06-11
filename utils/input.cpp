#include "input.h"

#include "globals.h"
#include "shared_time_constant.h"
#include <cstring>

namespace devils_engine {
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

    event_data::event_data(const utils::id &id) : id(id), time(0) {}
    key_data::key_data() : data(nullptr), event(release) {}
    data::data() : interface_focus(false), mouse_wheel(0.0f), current_text(0) {
      memset(text, 0, sizeof(uint32_t)*text_memory_size);
    }

    input_event next_input_event(size_t &mem) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = mem; i < container_size; ++i) {
        if (container.container[i].data != nullptr && container.container[i].data->id.valid() && container.container[i].data->time <= DELTA_TIME_CONSTANT) {
          mem = i+1;
          return {container.container[i].data->id, container.container[i].event};
        }
      }

      return {utils::id(), release};
    }

    input_event next_input_event(size_t &mem, const size_t &tolerancy) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = mem; i < container_size; ++i) {
        if (container.container[i].data != nullptr && container.container[i].data->id.valid() && container.container[i].data->time <= tolerancy) {
          mem = i+1;
          return {container.container[i].data->id, container.container[i].event};
        }
      }

      return {utils::id(), release};
    }

    struct input_event_state input_event_state(const utils::id &id) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = 0; i < container_size; ++i) {
        if (container.container[i].data != nullptr && container.container[i].data->id == id) return {id, container.container[i].event, container.container[i].data->time};
      }

      return {utils::id(), release, SIZE_MAX};
    }

    bool is_event_pressed(const utils::id &id) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = 0; i < container_size; ++i) {
        if (container.container[i].data != nullptr && container.container[i].data->id == id) return container.container[i].event != release;
      }
      return false;
    }

    bool is_event_released(const utils::id &id) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = 0; i < container_size; ++i) {
        if (container.container[i].data != nullptr && container.container[i].data->id == id) return container.container[i].event == release;
      }
      return false;
    }

    void update_time(const size_t &time) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = 0; i < key_count; ++i) {
        if (container.container[i].data == nullptr) continue;
        container.container[i].data->time += time;
      }
    }

    const char* get_key_name(const uint32_t &key) {
      if (key >= key_count) return nullptr;
      return key_names[key];
    }

    std::pair<utils::id, size_t> pressed_event(size_t &mem) {
      const auto &container = global::get<data>()->key_events;
      for (size_t i = mem; i < container_size; ++i) {
        if (container.container[i].data != nullptr && container.container[i].event != release) {
          mem = i+1;
          return std::make_pair(container.container[i].data->id, container.container[i].data->time);
        }
      }

      return std::make_pair(utils::id(), SIZE_MAX);
    }

    void set_key(const int &key, const utils::id &id) {
      auto &container = global::get<data>()->key_events;
      container.container[key].data = container.events_pool.create(id);
    }

    event_data* get_event(const int &key) {
      if (uint32_t(key) >= container_size) return nullptr;
      const auto &container = global::get<data>()->key_events;
      return container.container[key].data;
    }
  }
}
