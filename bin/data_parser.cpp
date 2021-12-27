#include "data_parser.h"

#include "utils/globals.h"
#include "utils/assert.h"
#include "utils/table_container.h"
#include "utils/utility.h"
#include "core_structures.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
#define SAME_TYPE 1
#define DIFFERENT_TYPE 0
#define DONT_CARE 2
    int check_sol_type(const sol::type &t, const sol::object &obj) {
      switch (t) {
        case sol::type::number: {
          if (obj.is<size_t>()) return SAME_TYPE;
          if (obj.is<double>()) return SAME_TYPE;
          break;
        }
        
        case sol::type::string: {
          if (obj.is<std::string>()) return SAME_TYPE;
          break;
        }
        
        default: return DONT_CARE;
      }
      
      return DIFFERENT_TYPE;
    }
    
    void recursive_check(const std::string_view &id, const std::string_view &data_type, const sol::table &table, const check_table_value* current_check, const check_table_value* array_check, const size_t &size, size_t &counter) {
      // если current_check != nullptr мы можем понять какой тип массива к нам пришел
      
      ASSERT(size != 0);
      const bool is_id_array = array_check[0].key == ID_ARRAY;
      const bool is_stats_array = array_check[0].key == STATS_ARRAY;
      const bool is_numeric_array = array_check[0].key == NUM_ARRAY;
      const bool is_stats_v2_array = array_check[0].key == STATS_V2_ARRAY;
      const bool basic_check = !is_id_array && !is_stats_array && !is_numeric_array && !is_stats_v2_array;
      
      if (!basic_check) {
        ASSERT(current_check->value_type == check_table_value::type::array_t);
        sol::type key_type = sol::type::none;
        sol::type value_type = sol::type::none;
        size_t maximum_key_num = array_check[0].max_count == 0 ? SIZE_MAX : array_check[0].max_count;
        size_t minimum_key_num = array_check[0].flags;
        if (is_stats_array) { key_type = sol::type::number; value_type = sol::type::number; }
        if (is_id_array) { value_type = sol::type::string; }
        if (is_numeric_array) { value_type = sol::type::number; }
        if (is_stats_v2_array) { key_type = sol::type::string; value_type = sol::type::number; }
        
        size_t data_counter = 0;
        for (const auto &pair : table) {
          int kret = check_sol_type(key_type, pair.first);
          int vret = check_sol_type(value_type, pair.second);
          if (kret == DIFFERENT_TYPE) continue;
          if (vret == DIFFERENT_TYPE) continue;
          if (kret == DONT_CARE && vret == DONT_CARE) continue;
          
          if (key_type == sol::type::number) {
            const size_t key = pair.first.as<size_t>();
            if ((key < minimum_key_num || key >= maximum_key_num) && is_stats_array) { PRINT("Bad stat key value in container " + std::string(current_check->key) + " in table " + std::string(id)); ++counter; }
          }
          
          ++data_counter;
        }
        
        const size_t max_data_values = current_check->max_count == 0 ? SIZE_MAX : current_check->max_count;
        if (data_counter > max_data_values) {
          PRINT("Too many data in container " + std::string(current_check->key) + " in table " + std::string(id)); ++counter;
          //throw std::runtime_error("fs;amsfs");
        }
      }
      
      if (basic_check) {
        for (size_t i = 0; i < size; ++i) {
          const auto &check = array_check[i];
          const bool stat_key = check.key == STAT_ID;
          const bool req = (check.flags & check_table_value::value_required) == check_table_value::value_required || stat_key;
          auto proxy = table[check.key];
          if (req && !proxy.valid()) { PRINT("Table field " + std::string(check.key) + " is required in " + std::string(data_type) + " data type"); ++counter; }
          if (!proxy.valid()) continue;
          switch (check.value_type) {
            case check_table_value::type::bool_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::boolean) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be boolean"); ++counter; }
              break;
            }
            
            case check_table_value::type::int_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::number) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be number"); ++counter; }
              break;
            }
            
            case check_table_value::type::float_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::number) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be number"); ++counter; }
              break;
            }
            
            case check_table_value::type::string_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::string) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be string"); ++counter; }
              break;
            }
            
            case check_table_value::type::array_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::table) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be table"); ++counter; }
              recursive_check(id, data_type, proxy.get<sol::table>(), &check, check.nested_array_data.begin(), check.nested_array_data.size(), counter);
              break;
            }
          }
          
          if (stat_key) {
            const size_t stat_id = proxy.get<size_t>();
            const uint32_t start = check.flags;
            const uint32_t end = check.max_count;
            if (stat_id >= start && stat_id < end) continue;
            ASSERT(current_check != nullptr);
            PRINT("Bad stat id in container " + std::string(current_check->key) + " in table " + std::string(id)); ++counter;
          }
        }
      }
    }
    
    std::string table_to_string(sol::this_state lua, const sol::table &table, const sol::table &keyallow) {
      sol::state_view state(lua);
      auto table_ser = state["serpent"];
      if (!table_ser.valid()) throw std::runtime_error("Serializator is not loaded");
      sol::protected_function f = table_ser["line"];
      if (!f.valid()) throw std::runtime_error("Could not load serializator function");
      
      auto opts = state.create_table();
      opts["compact"] = true;
      opts["fatal"] = true;
      opts["comment"] = false;
      (void)keyallow;
//       opts["keyallow"] = keyallow; // похоже что серпент и вложенные таблицы тоже так же проверяет значит придется сохранять все
      
      auto ret = f(table, opts);
      if (!ret.valid()) {
        sol::error err = ret;
        throw std::runtime_error("Could not serialize lua table: " + std::string(err.what()));
      }
      
      std::string value = ret;
      return value;
    }
  }
}
