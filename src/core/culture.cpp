#include "culture.h"

#include "bin/data_parser.h"
#include "utils/globals.h"
#include "bin/map_creator.h"
#include "realm_mechanics_arrays.h"
#include "context.h"

namespace devils_engine {
  namespace core {
    const structure culture::s_type;
    const size_t culture::max_stat_modifiers_count;
    culture::culture() : 
//       name_id(SIZE_MAX), 
//       name_bank(nullptr), 
      grandparent_name_chance(0.0f),
      group(nullptr),
      parent(nullptr),
      children(nullptr)
    {}
    
    void culture::add_member(core::character* member) const {
      if (members == nullptr) members = member;
      else utils::ring::list_radd<utils::list_type::culture_member>(members, member);
    }
    
    void culture::remove_member(core::character* member) const {
      if (members == member) members = utils::ring::list_next<utils::list_type::culture_member>(members, members);
    }
    
    script::culture_group_t culture::get_culture_group() const {
      return group;
    }
    
    const utils::check_table_value culture_group_table[] = {
      {
        "id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "name_id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "description_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "different_groups",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, 0,
        {
          {
            "character_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "popular_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_cultures",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, 0,
        {
          {
            "character_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "popular_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_child_cultures",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, 0,
        {
          {
            "character_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "popular_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      }
    };
    
    const utils::check_table_value culture_table[] = {
      {
        "id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "name_id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "description_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "group",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "parent",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "names_table_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "patronims_table_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "additional_table_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "grandparent_name_chance",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "color",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "different_groups",
        utils::check_table_value::type::array_t,
        0, 0,
        {
          {
            "character_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "popular_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_cultures",
        utils::check_table_value::type::array_t,
        0, 0,
        {
          {
            "character_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "popular_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_child_cultures",
        utils::check_table_value::type::array_t,
        0, 0,
        {
          {
            "character_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "popular_opinion",
            utils::check_table_value::type::float_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
//       {
//         "provinces",
//         utils::check_table_value::type::array_t,
//         utils::check_table_value::value_required, 0, {}
//       }
    };
    
    size_t add_culture_group(const sol::table &table) {
      return global::get<devils_engine::map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::culture_group), table);
    }
    
    bool validate_culture_group(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "culture_group" + std::to_string(index);
      }

      const size_t size = sizeof(culture_group_table) / sizeof(culture_group_table[0]);
      recursive_check(check_str, "culture_group", table, nullptr, culture_group_table, size, counter);

      return counter == 0;
    }
    
    void parse_culture_group(core::culture_group* culture_group, const sol::table &table) {
      {
        const auto id = table["id"].get<std::string>();
        culture_group->id = id;
      }
      
      {
        const auto name_id = table["name_id"].get<std::string>();
        culture_group->name_id = name_id;
      }
      
      if (const auto description_id = table["description_id"]; description_id.valid() && description_id.get_type() == sol::type::string) {
        culture_group->description_id = description_id;
      }
      
      {
        const auto t = table["different_groups"].get<sol::table>();
        culture_group->different_groups.character_opinion = t["character_opinion"];
        culture_group->different_groups.popular_opinion = t["popular_opinion"];
      }
      
      {
        const auto t = table["different_cultures"].get<sol::table>();
        culture_group->different_cultures.character_opinion = t["character_opinion"];
        culture_group->different_cultures.popular_opinion = t["popular_opinion"];
      }
      
      {
        const auto t = table["different_child_cultures"].get<sol::table>();
        culture_group->different_child_cultures.character_opinion = t["character_opinion"];
        culture_group->different_child_cultures.popular_opinion = t["popular_opinion"];
      }
    }
    
    size_t add_culture(const sol::table &table) {
      return global::get<devils_engine::map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::culture), table);
    }
    
    bool validate_culture(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "culture" + std::to_string(index);
      }

      const size_t size = sizeof(culture_table) / sizeof(culture_table[0]);
      recursive_check(check_str, "culture", table, nullptr, culture_table, size, counter);
      
      //if (const auto fs = table["features"]; fs.valid() && fs.get_type() == sol::type::table)

      return counter == 0;
    }
    
    void parse_culture(core::culture* culture, const sol::table &table) {
      auto ctx = global::get<core::context>();
      
      {
        const auto id = table["id"].get<std::string>();
        culture->id = id;
      }
      
      {
        const auto name_id = table["name_id"].get<std::string>();
        culture->name_id = name_id;
      }
      
      {
        const auto group = table["group"];
        const auto str = group.get<std::string_view>();
        const auto group_ptr = ctx->get_entity<core::culture_group>(str);
        if (group_ptr == nullptr) throw std::runtime_error("Could not find culture group " + std::string(str));
        culture->group = group_ptr;
      }
      
      if (const auto parent = table["parent"]; parent.valid() && parent.get_type() == sol::type::string) {
        const auto str = parent.get<std::string_view>();
        const auto parent_ptr = ctx->get_entity<core::culture>(str);
        if (parent_ptr == nullptr) throw std::runtime_error("Could not find parent culture " + std::string(str));
        culture->parent = parent_ptr;
      }
      
      if (const auto description_id = table["description_id"]; description_id.valid() && description_id.get_type() == sol::type::string) {
        culture->description_id = description_id;
      }
      
      if (const auto names_table_id = table["names_table_id"]; names_table_id.valid() && names_table_id.get_type() == sol::type::string) {
        culture->names_table_id = names_table_id;
      }
      
      if (const auto patronims_table_id = table["patronims_table_id"]; patronims_table_id.valid() && patronims_table_id.get_type() == sol::type::string) {
        culture->patronims_table_id = patronims_table_id;
      }
      
      if (const auto additional_table_id = table["additional_table_id"]; additional_table_id.valid() && additional_table_id.get_type() == sol::type::string) {
        culture->additional_table_id = additional_table_id;
      }
      
      if (const auto grandparent_name_chance = table["grandparent_name_chance"]; grandparent_name_chance.valid() && grandparent_name_chance.get_type() == sol::type::number) {
        culture->grandparent_name_chance = grandparent_name_chance;
      }
      
      if (const auto proxy = table["different_groups"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto t = proxy.get<sol::table>();
        culture->different_groups.character_opinion = t["character_opinion"];
        culture->different_groups.popular_opinion = t["popular_opinion"];
      } 
//       else {
//         culture->different_groups = culture->group->different_groups;
//       }
      
      if (const auto proxy = table["different_cultures"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto t = proxy.get<sol::table>();
        culture->different_cultures.character_opinion = t["character_opinion"];
        culture->different_cultures.popular_opinion = t["popular_opinion"];
      } 
//       else {
//         culture->different_cultures = culture->group->different_cultures;
//       }
      
      if (const auto proxy = table["different_child_cultures"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto t = proxy.get<sol::table>();
        culture->different_child_cultures.character_opinion = t["character_opinion"];
        culture->different_child_cultures.popular_opinion = t["popular_opinion"];
      } 
//       else {
//         culture->different_child_cultures = culture->group->different_child_cultures;
//       }
      
      if (const auto features = table["features"]; features.valid() && features.get_type() == sol::type::table) {
        for (size_t i = 0; i < core::culture_mechanics::count; ++i) {
          const auto name = core::culture_mechanics::names[i];
          const auto proxy = features[name];
          if (proxy.valid() && proxy.get_type() == sol::type::boolean) {
            const bool val = proxy;
            culture->set_mechanic(i, val);
          }
        }
      }
      
      if (const auto bonuses = table["bonuses"]; bonuses.valid() && bonuses.get_type() == sol::type::table) {
        size_t counter = 0;
        const auto t = bonuses.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.second.get_type() != sol::type::table) continue;
          if (counter >= core::culture::max_stat_modifiers_count) throw std::runtime_error("Too many bonuses");
          
          const auto bonus = pair.second.as<sol::table>();
          const uint32_t stat_id = bonus["stat"];
          // еще добавиться непосредственно бонусы
          
          ++counter;
        }
      }
      
      // бонусы к отношениям
      
//       {
//         const auto t = table["provinces"].get<sol::table>();
//         for (const auto &pair : t) {
//           if (!pair.second.is<uint32_t>()) continue;
//           
//           const uint32_t index = FROM_LUA_INDEX(pair.second.as<uint32_t>());
//           if (index >= ctx->get_entity_count<core::province>()) throw std::runtime_error("Bad province index " + std::to_string(index));
//           
//           auto prov = ctx->get_entity<core::province>(index);
//           ASSERT(prov->culture == nullptr);
//           prov->culture = culture;
//         }
//       }
    }
  }
}
