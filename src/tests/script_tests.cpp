#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <iostream>
#include "script/system.h"
#include "script/functions_init.h"
#include "utils/utility.h"
#include "core/structures_header.h"
#include "core/stats_table.h"
#include "fmt/format.h"

// по умолчанию используется add (сложение всех числе подряд в таблице)
const std::string_view script1 = "return { 5, 5, 10 }";
// вызов другой блочной функции
const std::string_view script2 = "return { 1, multiply = { 2, 10 } }";
// кондишен не даст сложить центральное число с остальными
const std::string_view script3 = "return { 5, { condition = false, 5 }, 10 }";
// вызовы обычных функций
const std::string_view script4 = "return { sin = 5, cos = 5, mix = { 1, 2, 0.5 } }";
// по умолчанию используется AND
const std::string_view script5 = "return { true, true, false }";
const std::string_view script51 = "return { 1, 1, 0 }";

const std::string_view script6 = "return { true, add_military = 5 }";


using namespace devils_engine;

TEST_CASE("basic script") 
//int main() 
{
  script::system sys;
  register_functions(&sys);
  
  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::bit32, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::package, sol::lib::utf8);
  
  // специальное значение, задача которого давать понять что мы пропускаем текущее вычисленное значение
  {
    CHECK(script::ignore_value.ignore());
    CHECK(!script::ignore_value.is<double>());
    CHECK(script::ignore_value.type == script::type_id<double>());
    CHECK(std::isnan(script::ignore_value.value));
  }
  
  {
    const auto ret = lua.script(script1);
    CHECK_ERROR_THROW(ret)
    
    const sol::object o = ret;
    const auto num_script = sys.create_number<script::object>(o, "basic script");
    script::context ctx("basic script", "num_script", 1);
    const double num = num_script.compute(&ctx);
    CHECK_EQ(num, 20);
  }
  
  {
    const auto ret = lua.script(script2);
    CHECK_ERROR_THROW(ret)
    
    const sol::object o = ret;
    const auto num_script = sys.create_number<script::object>(o, "basic script");
    script::context ctx("basic script", "num_script", 1);
    const double num = num_script.compute(&ctx);
    CHECK_EQ(num, 21);
  }
  
  {
    const auto ret = lua.script(script3);
    CHECK_ERROR_THROW(ret)
    
    const sol::object o = ret;
    const auto num_script = sys.create_number<script::object>(o, "basic script");
    script::context ctx("basic script", "num_script", 1);
    const double num = num_script.compute(&ctx);
    CHECK_EQ(num, 15);
  }
  
  {
    const auto ret = lua.script(script4);
    CHECK_ERROR_THROW(ret)
    
    const sol::object o = ret;
    const auto num_script = sys.create_number<script::object>(o, "basic script");
    script::context ctx("basic script", "num_script", 1);
    const double num = num_script.compute(&ctx);
    const double ans = std::sin(5.0) + std::cos(5.0) + glm::mix(1.0, 2.0, 0.5);
    CHECK(std::abs(num - ans) < EPSILON);
  }
  
  {
    const auto ret = lua.script(script5);
    CHECK_ERROR_THROW(ret)
    const sol::object o = ret;
    const auto num_script = sys.create_condition<script::object>(o, "basic script");
    script::context ctx("basic script", "num_script", 1);
    const bool num = num_script.compute(&ctx);
    const bool ans = true && true && false;
    CHECK_EQ(num, ans);
  }
  
  // мы можем использовать boolean значения в качестве чисел и наоборот
  {
    const auto ret = lua.script(script5);
    CHECK_ERROR_THROW(ret)
    const sol::object o = ret;
    const auto num_script = sys.create_number<script::object>(o, "basic script");
    script::context ctx("basic script", "num_script", 1);
    const double num = num_script.compute(&ctx);
    const double ans = double(true) + double(true) + double(false);
    CHECK_EQ(num, ans);
  }
  
  {
    const auto ret = lua.script(script51);
    CHECK_ERROR_THROW(ret)
    const sol::object o = ret;
    const auto num_script = sys.create_condition<script::object>(o, "basic script");
    script::context ctx("basic script", "num_script", 1);
    const double num = num_script.compute(&ctx);
    const double ans = bool(1.0) && bool(1.0) && bool(0.0);
    CHECK_EQ(num, ans);
  }
  
  // пытаемся создать функцию-эффект не в эффект скрипте
  {
    const auto ret = lua.script(script6);
    CHECK_ERROR_THROW(ret)
    const sol::object o = ret;
    CHECK_THROWS(sys.create_number<core::character*>(o, "basic script"));
  }
}

TEST_CASE("custom functions script") {
  script::system sys;
  register_functions(&sys);
  
  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::bit32, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::package, sol::lib::utf8);
  
  SUBCASE("character stats getter") {
    core::character ch(true, false);
    for (size_t i = 0; i < core::character_stats::count; ++i) {
      ch.stats.set(i, i);
      ch.current_stats.set(i, core::character_stats::count + i);
    }
    
    // проверяем в первом случае возвращаемое значение, во втором сравнение с числом (по умолчанию 'больше или равно')
    constexpr std::string_view format_get_stat = "return {{ \"{}{}\" }}";
    constexpr std::string_view format_compare_stat = "return {{ {}{} = {} }}";
    for (size_t i = 0; i < core::character_stats::count; ++i) {
      const auto name = core::character_stats::names[i];
      {
        const auto script = fmt::format(format_get_stat, "", name);
        const auto ret = lua.script(script);
        CHECK_ERROR_THROW(ret)
        const sol::object o = ret;
        const auto num_script = sys.create_number<core::character*>(o, "basic script");
        script::context ctx("basic script", "num_script", 1);
        ctx.root = ctx.current = script::object(&ch);
        const double num = num_script.compute(&ctx);
        CHECK_EQ(num, double(ch.current_stats.get(i)));
      }
      
      {
        const auto script = fmt::format(format_get_stat, "base_", name);
        const auto ret = lua.script(script);
        CHECK_ERROR_THROW(ret)
        const sol::object o = ret;
        const auto num_script = sys.create_number<script::object>(o, "basic script");
        script::context ctx("basic script", "num_script", 1);
        ctx.root = ctx.current = script::object(&ch);
        const double num = num_script.compute(&ctx);
        CHECK_EQ(num, double(ch.stats.get(i)));
      }
      
      {
        const double ref = double(core::character_stats::count + 5);
        const auto script = fmt::format(format_compare_stat, "", name, ref);
        const auto ret = lua.script(script);
        CHECK_ERROR_THROW(ret)
        const sol::object o = ret;
        const auto num_script = sys.create_condition<script::object>(o, "basic script");
        script::context ctx("basic script", "num_script", 1);
        ctx.root = ctx.current = script::object(&ch);
        const bool num = num_script.compute(&ctx);
        CHECK_EQ(num, double(ch.current_stats.get(i)) >= ref);
      }
      
      {
        const double ref = double(5);
        const auto script = fmt::format(format_compare_stat, "base_", name, ref);
        const auto ret = lua.script(script);
        CHECK_ERROR_THROW(ret)
        const sol::object o = ret;
        const auto num_script = sys.create_condition<core::character*>(o, "basic script");
        script::context ctx("basic script", "num_script", 1);
        ctx.root = ctx.current = script::object(&ch);
        const bool num = num_script.compute(&ctx);
        CHECK_EQ(num, double(ch.stats.get(i)) >= ref);
      }
    }
  }
    
  SUBCASE("character stats addition") {
    core::character ch(true, false);
    for (size_t i = 0; i < core::character_stats::count; ++i) {
      ch.stats.set(i, i);
      ch.current_stats.set(i, i);
    }
    
    // проверяем функцию-эффект, которая изменит стат у персонажа
    constexpr std::string_view format_add_stat = "return {{ {}{} = {} }}";
    for (size_t i = 0; i < core::character_stats::count; ++i) {
      const auto name = core::character_stats::names[i];
      {
        const auto script = fmt::format(format_add_stat, "add_", name, 5);
        const auto ret = lua.script(script);
        CHECK_ERROR_THROW(ret)
        const sol::object o = ret;
        const auto num_script = sys.create_effect<core::character*>(o, "basic script");
        script::context ctx("basic script", "num_script", 1);
        ctx.root = ctx.current = script::object(&ch);
        num_script.compute(&ctx);
        CHECK_EQ(double(ch.current_stats.get(i) + 5), double(ch.stats.get(i)));
      }
    }
  }
}
