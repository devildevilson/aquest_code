#ifndef INPUT_H
#define INPUT_H

#include "id.h"
// #include "memory_pool.h"
#include <cstddef>
#include <chrono>
#include <unordered_map>
#include <tuple>
#include "utility.h"

namespace devils_engine {
  namespace input {
    const size_t text_memory_size = 256;
    const size_t key_count = 350;
    const size_t container_size = key_count;
    extern const char* key_names[key_count];
    const size_t long_press_time = ONE_SECOND / 3;
    const size_t double_press_time = ONE_SECOND / 3;
    const uint8_t event_key_slots = 2;
    const uint8_t game_states = 1;

    enum type {
      release,
      press,
      repeated
    };
    
    enum state {
      state_initial      =      (0), // по идее тут 0 нужно сделать
      state_press        = (1 << 0),
      state_click        = (1 << 1),
      state_double_press = (1 << 2),
      state_double_click = (1 << 3),
      state_long_press   = (1 << 4),
      state_long_click   = (1 << 5)
    };

    struct event_data {
      utils::id id;
//       size_t time;

      event_data();
      event_data(const utils::id &id);
    };

    struct key_data {
      // эвентов на кнопке может быть несколько в зависимости от количества состояний игрока 
      // (ну то есть эвенты персонажа, персонажа в машине, персонажа в самолете и прочее)
      // (в моем случае возможно игрок на глобальной карте, игрок в бою, игрок в геройском бою, и еще что то)
      event_data data[utils::player_states_count];
      type event;
      enum state state;
      size_t state_time; // время непосредственно стейта (то есть переключение стейтов идет по нему)
      size_t event_time; // время нажатия (тут проверяем изменился ли стейт, можем ли мы это делать через время стейта?)
      key_data();
    };

    struct keys {
      struct event_keys_container { int keys[event_key_slots]; };
//       utils::memory_pool<event_data, sizeof(event_data)*50> events_pool;
      key_data container[container_size]; // тут должен быть контейнер с указателями
      // если одна из этих кнопок нажата то эвент срабатывает
      // а что если у одной кнопки двойное нажатие, а другой длинное? нужно 
      std::unordered_map<utils::id, event_keys_container> event_keys;
    };

    struct data {
      bool interface_focus;

      keys key_events;

      float mouse_wheel;

      uint32_t current_text;
      uint32_t text[text_memory_size];
      size_t last_frame_time;

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
    //struct input_event_state input_event_state(const utils::id &id);
    std::tuple<type, size_t> input_event_state(const utils::id &id);
    bool is_event_pressed(const utils::id &id);
    bool is_event_released(const utils::id &id);
    std::pair<utils::id, size_t> pressed_event(size_t &mem);
    utils::id get_event(const std::string_view &str);

    void update_time(const size_t &time);
    const char* get_key_name(const uint32_t &key);
    const char* get_event_key_name(const utils::id &id, const uint8_t &slot);
    void set_key(const int &key, const utils::id &id, const uint8_t &slot = 0);
    event_data get_event_data(const int &key);
    bool check_key(const int &key, const uint32_t &states);
    bool timed_check_key(const int &key, const uint32_t &states, const size_t &wait, const size_t &period);
    bool check_event(const utils::id &event, const uint32_t &states);
    bool timed_check_event(const utils::id &event, const uint32_t &states, const size_t &wait, const size_t &period);
    std::tuple<uint32_t, uint32_t> get_framebuffer_size();
    std::tuple<float, float> get_window_content_scale();
    std::tuple<float, float> get_monitor_content_scale();
    std::tuple<int32_t, int32_t> get_monitor_physical_size();
  }
}

#endif
