#ifndef INTERFACE_CONTAINER_H
#define INTERFACE_CONTAINER_H

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <array>
#include "sol.h"
#include "bit_field.h"

namespace devils_engine {
  namespace utils {
    constexpr double ceil(const double &data) {
      return double((static_cast<double>(static_cast<int64_t>(data)) == data) ? static_cast<int64_t>(data) : static_cast<int64_t>(data) + ((data > 0) ? 1 : 0));
    }

    struct timer {
      size_t current_time;
      size_t end_time;

      timer();
      timer(const size_t &end_time);
      ~timer();
      bool valid() const;
      size_t current() const;
      size_t end() const;
      double norm() const;
      void reset();

      void update(const size_t &time);
      void set_invalid();
    };

    struct timer_view {
      timer* t;

      timer_view(timer* t);
      ~timer_view();
      bool valid() const;
      size_t current() const;
      size_t end() const;
      double norm() const;
      void reset();
      //void set_invalid(); // ???
    };

    struct interface_container {
      static const size_t timers_count = 256;
      static const size_t maximum_openned_layers = UINT32_WIDTH;
      static const size_t bit_field_32_size = size_t(ceil(double(maximum_openned_layers) / double(UINT32_WIDTH)));
      static const size_t fonts_count = 4;

      static size_t first_layer();
      static size_t last_layer();

      struct layer_data {
        sol::function function;
        std::vector<sol::object> args;
        sol::object ret;
      };

      sol::state lua;
      sol::table layers_table;
      std::array<layer_data, maximum_openned_layers> openned_layers;
      std::array<timer, timers_count> timers;
      bit_field_32<bit_field_32_size> close_layers;
      std::array<std::pair<std::vector<sol::object>, sol::function>, maximum_openned_layers> open_layers;

      sol::object moonnuklear_ctx;
      sol::object fonts[fonts_count]; // требуется их переделывать
      sol::function get_ctx;
      sol::function get_font;
      sol::function free_font;

      interface_container();
      void draw(const size_t &time);
      bool close_layer(const uint32_t &index);
      void force_close_layer(const uint32_t &index);
      void close_all();
      bool open_layer(const uint32_t &index, const std::string_view &name, const std::initializer_list<sol::object> &data);
      bool force_open_layer(const uint32_t &index, const std::string_view &name, const std::initializer_list<sol::object> &data);
      bool update_data(const uint32_t &index, const std::initializer_list<sol::object> &data);
      bool is_visible(const uint32_t &index) const;

      timer_view create_timer(const size_t &end_time);
      void release_timer(timer_view t);

      // какие то скрипты возвращают функцию, нужно ее получить и как нибудь назвать
      void process_script_file(const std::string &path, const bool return_function = false, const std::string &function_name = "");
      void process_script(const std::string_view &script, const bool return_function = false, const std::string &function_name = "");
      void register_function(const std::string_view &lua_name, const std::string_view &container_name); // из глобальной таблицы перекидываем в локальную

      void init_constants();
      void init_input();
      void init_types();
      void init_game_logic();

      void free_fonts();
      void make_fonts();
      void collect_garbage();
    };
  }
}

#endif
