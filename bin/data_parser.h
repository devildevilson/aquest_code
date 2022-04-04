#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <string_view>
#include "utils/sol.h"

#include "core/declare_structures.h"

#define UINT_VALUE_NOT_FOUND SIZE_MAX
#define UINT_VALUE_NOT_VALID (UINT_VALUE_NOT_FOUND-1)

// скрипты более мнее понятно как делать теперь (смотри decision_thoughts.lua)

struct nk_context;

#define ID_ARRAY "id_array"
#define STATS_ARRAY "stats_array"
#define STATS_V2_ARRAY "stats_v2_array"
#define MODIFIERS_ARRAY "modifiers_array"
#define STAT_ID "stat"
#define INDEX_ARRAY "index_array"
#define COLOR_ARRAY "color_array"
#define VEC4_ARRAY "vec4_array"
#define NUM_ARRAY "num_array"
#define NESTED_ARRAY "nested_array"

namespace devils_engine {
  namespace utils {
    class world_serializator;
    
    struct check_table_value {
      enum class type {
        bool_t,
        int_t,
        float_t,
        string_t,
        array_t,
      };
      
      enum flags_bits {
        value_required = (1 << 0),
        
      };
      
      const std::string_view key;
      const type value_type;
      const uint32_t flags;
      const uint32_t max_count;
      const std::initializer_list<check_table_value> nested_array_data;
    };
    
//     #define SAME_TYPE 1
//     #define DIFFERENT_TYPE 0
//     #define DONT_CARE 2
//     int check_sol_type(const sol::type &t, const sol::object &obj);
    void recursive_check(const std::string_view &id, const std::string_view &data_type, const sol::table &table, const check_table_value* current_check, const check_table_value* array_check, const size_t &size, size_t &counter);
    
    std::string table_to_string(sol::this_state lua, const sol::table &table, const sol::table &keyallow);
  }
}

// как то так выглядит, персонаж самая сложная часть
// нужно потом еще сделать систему правления (набор механик)
// теперь у нас более менее ясная картина того как это все выглядит
// теперь по идее мне всего хватает для того чтобы сделать первых персонажей
// и первый ход, мне не дают покоя константы в классе персонажа (размеры контейнеров)

#endif
