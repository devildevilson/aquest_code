#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"

namespace devils_engine {
  namespace utils {
    size_t add_city(const sol::table &table) {
      return global::get<utils::table_container>()->add_table(core::structure::city, table);
    }
    
    bool validate_city(const sol::table &table) {
      return true;
    }
    
    bool validate_city_and_save(sol::this_state lua, const sol::table &table) {
      return true;
    }
    
    void parse_city(core::city* city, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      {
        const size_t index = table["province"];
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
        memcpy(city->current_stats, city_type->stats, sizeof(core::stat_container) * core::city_stats::count);
      }
      
      city->tile_index = table["tile_index"];
      
      // пока все ???
    }
  }
}
