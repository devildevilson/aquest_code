#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"
#include "utils/serializator_helper.h"
#include "map_creator.h"
#include <iostream>

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace utils {
    const check_table_value city_table[] = {
      {
        "province",
        check_table_value::type::int_t,
        check_table_value::value_required, 0, {}
      },
      {
        "title",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "city_type",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "tile_index",
        check_table_value::type::int_t,
        check_table_value::value_required, 0, {}
      },
    };
    
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
//       opts["keyallow"] = keyallow; // похоже что серпент и вложенные таблицы тоже так же проверяет значит придется сохранять все
      
      auto ret = f(table, opts);
      if (!ret.valid()) {
        sol::error err = ret;
        throw std::runtime_error("Could not serialize lua table: " + std::string(err.what()));
      }
      
      std::string value = ret;
      return value;
    }
    
    size_t add_city(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::city), table);
    }
    
    bool validate_city(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "city" + std::to_string(index);
      }
      
      const size_t size = sizeof(city_table) / sizeof(city_table[0]);
      recursive_check(check_str, "city", table, nullptr, city_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_city_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_city(index, table);
      if (!ret) return false;
      
      sol::state_view state(lua);
      //auto keyallow = state.create_table();
      //keyallow.add("province", "title", "city_type", "tile_index");
      //auto str = table_to_string(lua, table, keyallow);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize city type table");
      ASSERT(false);
      //container->add_data(core::structure::city, std::move(str));
      
      return true;
    }
    
    void parse_city(core::city* city, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      {
        // индексы у нас приходят из луа, а это значит что все они будут смещены на единичку
        // тут в парсере нужно исправлять это дело, можно ли сделать это унифицированным способом?
        // так чтобы на всякий случай можно было бы быстро переключить? макросы короче говоря
        const size_t index = FROM_LUA_INDEX(table["province"].get<uint32_t>());
        auto province = ctx->get_entity<core::province>(index);
        city->province = province;
      }
      
      { // вообще удобно наверное здесь указать титул
        const std::string str = table["title"];
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find title " + str);
        auto title = ctx->get_entity<core::titulus>(index);
        city->title = title;
      }
      
      {
        const std::string str = table["city_type"];
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find city_type " + str);
        auto city_type = ctx->get_entity<core::city_type>(index);
        city->type = city_type;
        memcpy(city->current_stats.data(), city_type->stats.data(), sizeof(core::stat_container) * core::city_stats::count);
      }
      
      city->tile_index = FROM_LUA_INDEX(table["tile_index"].get<uint32_t>());
      
      // пока все ???
    }
  }
}
