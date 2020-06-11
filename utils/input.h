#ifndef INPUT_H
#define INPUT_H

#include "id.h"
#include "memory_pool.h"
#include <cstddef>
#include <chrono>
#include "utility.h"

namespace devils_engine {
  namespace input {
    const size_t text_memory_size = 256;
    const size_t key_count = 350;
    const size_t container_size = key_count;
    extern const char* key_names[key_count];

    enum type {
      release,
      press,
      repeated
    };

    struct event_data {
      utils::id id;
      size_t time;

      event_data(const utils::id &id);
    };

    struct key_data {
//       utils::id id;
//       size_t time;
      event_data* data;
      type event;
      // size_t next; // может ли быть на одной копке несколько эвентов? вот у эвента может быть несколько кнопок
      key_data();
    };

    struct keys {
      utils::memory_pool<event_data, sizeof(event_data)*50> events_pool;
      key_data container[container_size]; // тут должен быть контейнер с указателями
    };

    struct data {
      bool interface_focus;

      keys key_events;

      float mouse_wheel;

      uint32_t current_text;
      uint32_t text[text_memory_size];

      std::chrono::steady_clock::time_point double_click_time_point;
      glm::uvec2 click_pos;
      glm::vec2 fb_scale;

      data();
    };

    struct input_event {
      utils::id id;
      type event;
    };
    input_event next_input_event(size_t &mem);
    input_event next_input_event(size_t &mem, const size_t &tolerancy);

    struct input_event_state {
      utils::id id;
      type event;
      size_t time; // изменение было time микросекунд назад
    };
    struct input_event_state input_event_state(const utils::id &id);
    bool is_event_pressed(const utils::id &id);
    bool is_event_released(const utils::id &id);
    std::pair<utils::id, size_t> pressed_event(size_t &mem);

    void update_time(const size_t &time);
    const char* get_key_name(const uint32_t &key);
    void set_key(const int &key, const utils::id &id);
    event_data* get_event(const int &key);
  }
}

#endif
