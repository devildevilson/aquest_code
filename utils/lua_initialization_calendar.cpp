#include "lua_initialization.h"

#include "bin/game_time.h"
#include "globals.h"
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
    void setup_lua_calendar(sol::state_view lua) {
      auto calendar = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::calendar)].get_or_create<sol::table>();
      // нужно ли дать возможность изменять переменные? не думаю, это не полезно
      calendar.new_usertype<calendar::date>("date", sol::constructors<calendar::date(), calendar::date(const int32_t&, const uint16_t&, const uint16_t&)>(),
        "valid", &calendar::date::valid,
        "year", &calendar::date::year,
        "month", &calendar::date::month,
        "day", &calendar::date::day,
        "before_zero", &calendar::date::before_zero
      );
      
      calendar.set_function("set_start_date", [] (const int32_t &year, const uint32_t &month, const uint32_t &day) {
        global::get<utils::calendar>()->set_start_date(year, month, day);
      });
      
      calendar.set_function("set_current_date", [] (const int32_t &year, const uint32_t &month, const uint32_t &day) {
        global::get<utils::calendar>()->set_current_date(year, month, day);
      });
      
      calendar.set_function("set_week_days_count", [] (const uint32_t &count) {
        global::get<utils::calendar>()->set_week_days_count(count);
      });
      
      calendar.set_function("add_month_data", [] (const std::string &name_id, const uint32_t &count) {
        global::get<utils::calendar>()->add_month_data({name_id, count});
      });
      
      calendar.set_function("start_date", [] () {
        return global::get<utils::calendar>()->start_date();
      });
      
      calendar.set_function("current_date", [] () {
        return global::get<utils::calendar>()->current_date();
      });
      
      calendar.set_function("current_turn", [] () {
        return global::get<utils::calendar>()->current_turn();
      });
      
      calendar.set_function("current_day", [] () {
        return global::get<utils::calendar>()->current_day();
      });
      
      calendar.set_function("convert_turn_to_date", [] (const size_t &turn) {
        return global::get<utils::calendar>()->convert_turn_to_date(turn);
      });
      
      calendar.set_function("convert_turn_to_days", [] (const size_t &turn) {
        return global::get<utils::calendar>()->convert_turn_to_days(turn);
      });
      
      calendar.set_function("convert_date_to_turn", [] (const struct utils::calendar::date &date) {
        return global::get<utils::calendar>()->convert_date_to_turn(date);
      });
      
      calendar.set_function("convert_days_to_date", [] (const int64_t &days) {
        return global::get<utils::calendar>()->convert_days_to_date(days);
      });
      
      calendar.set_function("convert_date_to_days", [] (const struct utils::calendar::date &date) {
        return global::get<utils::calendar>()->convert_date_to_days(date);
      });
      
      calendar.set_function("get_month_data", [] (const uint32_t index) -> std::tuple<std::string_view, uint32_t> {
        auto cal = global::get<utils::calendar>();
        if (index >= cal->m_months.size()) return std::make_tuple("", 0);
        return std::make_tuple(std::string_view(cal->m_months[index].name_id), cal->m_months[index].days_count);
      });
    }
  }
}
