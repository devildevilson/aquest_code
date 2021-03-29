#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <atomic>
#include "render/shared_structures.h"

namespace devils_engine {
  namespace utils {
    // мне нужно возвращать время по текущему ходу
    // и нужно возвращать ход по времени
    // нужно указать начальную дату и текущую
    // начальная дата может начинаться до нашей эры
    // как то можно сделать длинные периоды 
    // например как в песни льда и пламени: несколько лет длится лето, несколько лет длится зима
    // но при этом тогда нужно делать время жизни через ходы
    // в цк2 все персонажи начинали с 5 хп, смерть от возраста - фиксирована
    // хп как то воздействует на фиксированный шанс
    // что наверное ограничивает меня в настройке календаря
    // нужно тогда ввести ограничение: в сутках 24 часа, средняя продолжительность жизни 50*365 дней
    // скорее всего сильно поменять обычный ход времени не представляется возможным
    // точнее время должно всегда идти по установленному порядку и изменять его нельзя
    // значит сезоны должны должны меняться по специальным эвентам и 
    // зависеть от текущей даты просто потому что удобно эвент прицепить к году
    
    struct calendar {
      struct month_data {
        size_t name_str;
        uint32_t days_count;
      };
      
      struct date {
        int32_t m_year;
        uint16_t m_month;
        uint16_t m_day;
        
        date();
        date(const int32_t &m_year, const uint16_t &m_month, const uint16_t &m_day);
        int32_t year() const;
        uint32_t month() const;
        uint32_t day() const;
        bool before_zero() const;
        bool valid() const;
      };
      
      size_t m_name_str;
      uint32_t m_week_days_count;
      int64_t m_start_day;
      std::atomic<size_t> m_current_turn;
      size_t m_years_days;
      std::vector<month_data> m_months;
      
      calendar();
      void set_start_date(const int32_t &year, const uint32_t &month, const uint32_t &day);
      void set_current_date(const int32_t &year, const uint32_t &month, const uint32_t &day);
      void set_week_days_count(const uint32_t &count);
      void add_month_data(const month_data &data);
      
      date start_date() const;
      date current_date() const;
      size_t current_turn() const;
      int64_t current_day() const;
      
      void advance_turn();
      
      date convert_turn_to_date(const size_t &turn) const;
      int64_t convert_turn_to_days(const size_t &turn) const;
      size_t convert_date_to_turn(const struct date &date) const;
      date convert_days_to_date(const int64_t &days) const;
      int64_t convert_date_to_days(const struct date &date) const;
      
      void validate();
    };
    
    bool operator==(const calendar::date &date1, const calendar::date &date2);
    bool operator!=(const calendar::date &date1, const calendar::date &date2);
    bool operator>=(const calendar::date &date1, const calendar::date &date2);
    bool operator<=(const calendar::date &date1, const calendar::date &date2);
    bool operator>(const calendar::date &date1, const calendar::date &date2);
    bool operator<(const calendar::date &date1, const calendar::date &date2);
  }
}

#endif
