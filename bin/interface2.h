#ifndef INTERFACE2_H
#define INTERFACE2_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <limits>
#include <functional>
#include <string>
#include <unordered_map>
#include "utils/bit_field.h"
#include "declare_structures.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

// тут должны быть указаны интерфейсы + инициализация
// единовременно может быть нарисовано несколько интерфейсов
// но при этом только один тип за раз (то есть нет смысла показывать два эвента за раз)
// с нашей стороны требуется четко определить эти типы 
// нужно сделать запуск этих функций в специальном энвайроменте, чтобы никто не мог загрузить какую нибудь хрень
// мне нужно сделать возвращение на предыдущее окно (да и вообще обработку кнопки эскейп)
// оставить это на усмотрение интерфейса? нужно видимо сделать управление интерфейсами прямо в самой функции

struct nk_context;

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
      bool valid() const;
      size_t current() const;
      size_t end() const;
      double norm() const;
      void reset();
      
      void update(const size_t &time);
      void set_invalid();
    };
    
    class interface {
    public:
      static const size_t timers_count = 256;
      static const size_t bit_field_size = ceil(double(timers_count) / double(SIZE_WIDTH));
      static const size_t window_types_count = 32;
      static const size_t bit_field_32_size = ceil(double(window_types_count) / double(UINT32_WIDTH));
      
      struct window_info {
        // должны ли мы здесь что нибудь возвращать? мы должны собрать данные для десизион и эвентов
        // это мы собираем по специальным функциям + мы должны нарисовать портреты и какие то другие картинки
        // возвращаем закрытие окна
        //std::function<bool(nk_context*, interface*, const void*)> window;
        sol::protected_function window;
        uint32_t type;
        core::structure target;
      };
      
      interface();
      ~interface();
      
      timer* create_timer(const size_t &end_time);
      void release_timer(timer* t);
      
      bool is_window_type_visible(const uint32_t &index) const;
      void close_window(const uint32_t &index);
      
      // как открыть окно? по идее нам достаточно передать название окна, нужно еще передать какой то объект, как проверить тип?
      // проверить можно если только на все структуры прилепить специальную переменную с типом структуры (как в вулкане)
      // думаю что так можно сделать, но потом
      void open_window(const std::string &window_id, const void* data); // тут чаще всего персонаж, но не всегда
      
      void draw(const size_t &time);
      void register_window(const std::string &name, const window_info &window);
      
      sol::state & get_state();
    private:
      sol::state lua; // большинство вещей в этом стейте будут рид онли
      
      std::array<std::pair<const void*, const window_info*>, window_types_count> current_windows;
      bit_field_32<bit_field_32_size> windows;
      bit_field_32<bit_field_32_size> close_windows;
      std::array<std::pair<const void*, const window_info*>, window_types_count> open_windows;
      std::array<timer, timers_count> timers;
      bit_field<bit_field_size> used_timers;
      
      std::unordered_map<std::string, window_info> registered_windows;
    };
  }
}

#endif
