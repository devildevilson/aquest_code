#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"
#include "utils/serializator_helper.h"
#include "map_creator.h"

namespace devils_engine {
  namespace utils {
    const check_table_value title_table[] = {
      {
        "type",
        check_table_value::type::int_t,
        check_table_value::value_required, static_cast<int32_t>(core::titulus::type::count), {}
      },
      {
        "parent",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "main_color",
        check_table_value::type::int_t,
        check_table_value::value_required, UINT32_MAX, {}
      }, 
      {
        "border_color1",
        check_table_value::type::int_t,
        check_table_value::value_required, UINT32_MAX, {}
      }, 
      {
        "border_color2",
        check_table_value::type::int_t,
        check_table_value::value_required, UINT32_MAX, {}
      }, 
      {
        "heraldy",
        check_table_value::type::string_t,
        0, 0, {}
      }
    };
    
    size_t add_title(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::titulus), table);
    }
    
    bool validate_title(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "title" + std::to_string(index);
      }
      
      const size_t size = sizeof(title_table) / sizeof(title_table[0]);
      recursive_check(check_str, "title", table, nullptr, title_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_title_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_title(index, table);
      if (!ret) return false;
      
      sol::state_view state(lua);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize title table");
      container->add_data(core::structure::titulus, std::move(str));
      
      return true;
    }
    
    void parse_title(core::titulus* title, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto heraldy_data = global::get<utils::numeric_string_container>();
      auto ctx = global::get<core::context>();
      
      // мы создаем титул до городов? наверное сначало город, тогда нам можно не заполнять детей
      //title->id = table["id"];
      
      {
        const int32_t lua_val = table["type"];
        if (lua_val < 0 || lua_val >= static_cast<int32_t>(core::titulus::type::count)) throw std::runtime_error("Bad title type for " + title->id);
        const auto value = static_cast<enum core::titulus::type>(lua_val);
        title->type = value;
      }
      
      if (const auto &parent = table["parent"]; parent.valid()) { // у некоторых титулов низкого уровня может не быть родительских титулов
        const std::string str = parent.get<std::string>();
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find parent title " + str + " for " + title->id);
        auto t = ctx->get_entity<core::titulus>(index);
        title->parent = t;
      } 
      /*else if (title->type != core::titulus::type::city && title->type != core::titulus::type::imperial) {
        throw std::runtime_error("King, duchy, baron titles must have a parent title");
      }*/
      
      ASSERT(title->count == 0);
      
      // наверное лучше собрать детей по родительским указателям
      switch (title->type) {
        case core::titulus::type::imperial:
        case core::titulus::type::king: 
        case core::titulus::type::duke: {
//           const auto &childs = table.get<sol::table>("childs");
//           std::vector<core::titulus*> titles;
//           for (auto itr = childs.begin(); itr != childs.end(); ++itr) {
//             if (!(*itr).second.is<std::string>()) continue;
//             
//             const auto &str = (*itr).second.as<std::string>();
//             const size_t index = to_data->get(str);
//             if (index == SIZE_MAX) throw std::runtime_error("Could not find title " + str);
//             auto child = ctx->get_entity<core::titulus>(index);
//             titles.push_back(child);
//           }
//           
//           // проверять на количество или создавать массив самостоятельно? 
//           // могу ли я гарантировать что в sol table будет храниться только строки-id титулов?
//           // в доках кажется сказано что нет
//           
//           title->create_children(titles.size());
//           for (size_t i = 0; i < titles.size(); ++i) {
//             title->set_child(i, titles[i]);
//           }
          
          break;
        }
        
        case core::titulus::type::baron: {
//           title->create_children(1);
//           const uint32_t &province = table["province"];
//           auto province_ptr = ctx->get_entity<core::province>(province);
//           title->set_province(province_ptr);
          break;
        }
        
        case core::titulus::type::city: {
//           title->create_children(1);
//           const uint32_t &city = table["city"];
//           auto city_ptr = ctx->get_entity<core::city>(city);
//           title->set_city(city_ptr);
          break;
        }
        
        default: throw std::runtime_error("Bad title type");
      }
      
      {
        const uint32_t col = table["main_color"];
        render::color_t c{col};
        title->main_color = c;
      }
      
      {
        const uint32_t col = table["border_color1"];
        render::color_t c{col};
        title->border_color1 = c;
      }
      
      {
        const uint32_t col = table["border_color2"];
        render::color_t c{col};
        title->border_color2 = c;
      }
      
      if (auto proxy = table["heraldy"]; proxy.valid()) {
        const std::string str = proxy;
        const size_t index = heraldy_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find heraldy " + str);
        if (index >= UINT32_MAX) throw std::runtime_error("Could bad heraldy " + str + " index");
        title->heraldy = index;
      }
      
      // добавится герб
      
      // по идее мы можем здесь указать владельца титула
      // но это будет менее правильно
      // потому что по идее мы сначало раскидываем титулы по провинциям и землям
      // после этого по идее должно быть формирование государств (хотя может и нет)
      // тем не менее мне кажется что указать список титулов персонажу лучше 
      
      // указывать здесь владельца или нет зависит в основном от порядка загрузки (нет)
      // точнее если я хочу взять какую то информацию из объекта, то понятное дело он уже к этому времени должен быть загружен
      // здесь я хочу взять родительский титул из города (делать мы это будем после парсинга я так понимаю)
    }
  }
}
