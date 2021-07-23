#ifndef SCRIPT_UTILITY_H
#define SCRIPT_UTILITY_H

#include "utils/sol.h"
#include <string>
#include <tuple>
#include "target.h"

#define CONDITION 0
#define ACTION 1

#define FALSE_BLOCK 0
#define TRUE_BLOCK 1
#define IGNORE_BLOCK 2

namespace devils_engine {
  namespace script {
    struct script_data;
    struct context;
    
    char* copy_str_to_char_array(const std::string_view &str);
    void make_lvalue(const std::string_view &str, const bool has_colon, script_data* data);
    // ожидается что value будет заполненно текущим таргетом
    void get_lvalue(context* ctx, const script_data* lvalue_array, script_data* value);
    void make_complex_value(const uint32_t &target_type, const sol::table &table, script_data* data, const uint32_t &expected_type);
    void get_complex_value(const target_t* t, context* ctx, const script_data* complex_array, script_data* value);
    void boolean_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data);
    void variable_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data);
    void string_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data);
    void target_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data);
    // value, special_type, number_type, compare_type
    std::tuple<double, uint16_t, uint8_t, uint8_t> get_num_from_data(const struct target_t* target, context* ctx, const script_data* data);
    char* get_raw_string_from_data(const struct target_t* target, context* ctx, const script_data* data);
    std::string_view get_string_from_data(const struct target_t* target, context* ctx, const script_data* data);
    target_t get_target_from_data(const struct target_t* target, context* ctx, const script_data* data);
    
    // тут бы и токен не помешало бы вернуть, хотя он в таргете лежит
    sol::object make_object(sol::state_view s, const struct target_t &target);
    
    // по идее ретюрн валью всегда bool, нет, иногда его просто может не быть, тогда лучше передавать int32
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, const bool value, const bool original, const int32_t &ret);
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, 
                       const double &value, const uint32_t &compare_type, const uint32_t &special_stat, 
                       const double &original, const int32_t &ret);
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, const std::string_view &str, const sol::object &ad_data, const int32_t &ret);
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, const target_t* obj, const sol::object &ad_data, const int32_t &ret);
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, 
                       const sol::object &obj, const sol::object &ad_data, const sol::object &ret,
                       const uint32_t &compare_type = UINT32_MAX, const uint32_t &special_stat = UINT32_MAX);
  }
}

#endif
