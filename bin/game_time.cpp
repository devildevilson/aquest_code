#include "game_time.h"

namespace devils_engine {
  namespace utils {
    uint32_t calendar::date::year() const { return m_year; }
    uint32_t calendar::date::month() const { return m_month; }
    uint32_t calendar::date::day() const { return std::abs(m_day); }
    bool calendar::date::before_zero() const { return m_day < 0; }
    
    void calendar::set_start_date(const bool before_zero, const uint32_t &year, const uint32_t &month, const uint32_t &day) {
      const int16_t final_day = before_zero ? -int16_t(day) : int16_t(day);
      const date tmp {
        int32_t(year),
        int16_t(month),
        final_day
      };
      
      m_start_day = convert_date_to_days(tmp);
    }
    
    void calendar::set_current_date(const bool before_zero, const uint32_t &year, const uint32_t &month, const uint32_t &day) {
      const int16_t final_day = before_zero ? -int16_t(day) : int16_t(day);
      const date tmp {
        int32_t(year),
        int16_t(month),
        final_day
      };
      
      m_current_turn = convert_date_to_turn(tmp);
    }
    
    void calendar::set_week_days_count(const uint32_t &count) { m_week_days_count = count; }
    void calendar::add_month_data(const month_data &data) { m_months.push_back(data); }
    
    calendar::date calendar::start_date() const { return convert_days_to_date(m_start_day); }
    calendar::date calendar::current_date() const { return convert_turn_to_date(m_current_turn); }
    size_t calendar::current_turn() const { return m_current_turn; }
    int64_t calendar::current_day() const { return convert_turn_to_days(m_current_turn); }
    
    void calendar::advance_turn() { ++m_current_turn; }
    
    calendar::date calendar::convert_turn_to_date(const size_t &turn) const {
      const int64_t turn_days = turn * m_week_days_count;
      const int64_t current_day = m_start_day + turn_days;
      return convert_days_to_date(current_day);
    }
    
    int64_t calendar::convert_turn_to_days(const size_t &turn) const {
      return m_start_day + int64_t(turn * m_week_days_count);
    }
    
    size_t calendar::convert_date_to_turn(const struct date &date) const {
      const size_t final_days = convert_date_to_days(date);
      return final_days / m_week_days_count;
    }
    
    calendar::date calendar::convert_days_to_date(const int64_t &days) const {
      const uint32_t cur_year = std::abs(days) / m_years_days;
      size_t current_year_day = std::abs(days) - cur_year * m_years_days;
      ASSERT(current_year_day < m_years_days);
      uint32_t month_index = 0;
      while (month_index < m_months.size() && current_year_day > m_months[month_index].days_count) {
        current_year_day -= m_months[month_index].days_count;
        ++month_index;
        ASSERT(month_index != m_months.size());
      }
      
      const int16_t final_days = days < 0 ? -int16_t(current_year_day) : int16_t(current_year_day);
      const date current_date{
        int32_t(cur_year),
        int16_t(month_index),
        final_days
      };
      
      return current_date;
    }
    
    int64_t calendar::convert_date_to_days(const struct date &date) const {
      int64_t days_count = date.day();
      int32_t month_index = date.month()-1;
      while (month_index >= 0) {
        days_count += m_months[month_index].days_count;
        --month_index;
      }
      
      days_count += date.year() * m_years_days; // это количество дней прошедшее от 0 года
      return std::abs(m_start_day) + (date.before_zero() ? -1 * days_count : days_count);
    }
    
    bool operator==(const calendar::date &date1, const calendar::date &date2) {
      return date1.m_year == date2.m_year && date1.m_month == date2.m_month && date1.m_day == date2.m_day;
    }
    
    bool operator>=(const calendar::date &date1, const calendar::date &date2) {
      if (date1.before_zero() == date2.before_zero()) {
        if (date1.year() == date2.year()) {
          if (date1.month() == date2.month()) return date1.m_day >= date2.m_day;
          return date1.before_zero() ? -date1.m_month >= -date2.m_month : date1.m_month >= date2.m_month;
        }
        
        return date1.before_zero() ? -date1.m_year >= -date2.m_year : date1.m_year >= date2.m_year;
      }
      
      return uint32_t(!date1.before_zero()) >= uint32_t(!date2.before_zero());
    }
    
    bool operator<=(const calendar::date &date1, const calendar::date &date2) {
      if (date1.before_zero() == date2.before_zero()) {
        if (date1.year() == date2.year()) {
          if (date1.month() == date2.month()) return date1.m_day <= date2.m_day;
          return date1.before_zero() ? -date1.m_month <= -date2.m_month : date1.m_month <= date2.m_month;
        }
        
        return date1.before_zero() ? -date1.m_year <= -date2.m_year : date1.m_year <= date2.m_year;
      }
      
      return uint32_t(!date1.before_zero()) <= uint32_t(!date2.before_zero());
    }
    
    bool operator>(const calendar::date &date1, const calendar::date &date2) {
      if (date1.before_zero() == date2.before_zero()) {
        if (date1.year() == date2.year()) {
          if (date1.month() == date2.month()) return date1.m_day > date2.m_day;
          return date1.before_zero() ? -date1.m_month > -date2.m_month : date1.m_month > date2.m_month;
        }
        
        return date1.before_zero() ? -date1.m_year > -date2.m_year : date1.m_year > date2.m_year;
      }
      
      return uint32_t(!date1.before_zero()) > uint32_t(!date2.before_zero());
    }
    
    bool operator<(const calendar::date &date1, const calendar::date &date2) {
      if (date1.before_zero() == date2.before_zero()) {
        if (date1.year() == date2.year()) {
          if (date1.month() == date2.month()) return date1.m_day < date2.m_day;
          return date1.before_zero() ? -date1.m_month < -date2.m_month : date1.m_month < date2.m_month;
        }
        
        return date1.before_zero() ? -date1.m_year < -date2.m_year : date1.m_year < date2.m_year;
      }
      
      return uint32_t(!date1.before_zero()) < uint32_t(!date2.before_zero());
    }
  }
}
