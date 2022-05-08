#include "trait.h"

#include "utils/shared_mathematical_constant.h"
#include "bin/data_parser.h"
#include "utils/globals.h"
#include "core/context.h"
#include "utils/systems.h"
#include "traits_modifier_attributes_arrays.h"
#include "stats_table.h"

namespace devils_engine {
  namespace core {
    const structure trait::s_type;
    const size_t trait::max_stat_modifiers_count;
    const size_t trait::max_opinion_modifiers_count;
    trait_group::trait_group() :
      same_trait_opinion(fNAN),
      same_group_opinion(fNAN),
      different_group_opinion(fNAN),
      opposite_opinion(fNAN),
      nogroup_opinion(fNAN)
    {}
    
    trait::trait() : 
      opposite(nullptr),
      group(nullptr),
      numeric_attribs{0,0,0,0}, 
      icon{GPU_UINT_MAX},
      same_trait_opinion(fNAN),
      same_group_opinion(fNAN),
      different_group_opinion(fNAN),
      opposite_opinion(fNAN),
      nogroup_opinion(fNAN)
    {}
    
    float trait::get_same_trait_opinion() const {
      if (std::isnan(same_trait_opinion) && group != nullptr) return std::isnan(group->same_trait_opinion) ? 0.0f : group->same_trait_opinion;
      return std::isnan(same_trait_opinion) ? 0.0f : (group == nullptr ? 0.0f : same_trait_opinion);
    }
    
    float trait::get_same_group_opinion() const {
      if (std::isnan(same_group_opinion) && group != nullptr) return std::isnan(group->same_group_opinion) ? 0.0f : group->same_group_opinion;
      return std::isnan(same_group_opinion) ? 0.0f : (group == nullptr ? 0.0f : same_group_opinion);
    }
    
    float trait::get_different_group_opinion() const {
      if (std::isnan(different_group_opinion) && group != nullptr) return std::isnan(group->different_group_opinion) ? 0.0f : group->different_group_opinion;
      return std::isnan(different_group_opinion) ? 0.0f : (group == nullptr ? 0.0f : different_group_opinion);
    }
    
    float trait::get_opposite_opinion() const {
      if (std::isnan(opposite_opinion) && group != nullptr) return std::isnan(group->opposite_opinion) ? 0.0f : group->opposite_opinion;
      return std::isnan(opposite_opinion) ? 0.0f : opposite_opinion;
    }
    
    float trait::get_nogroup_opinion() const {
      if (std::isnan(nogroup_opinion) && group != nullptr) return std::isnan(group->nogroup_opinion) ? 0.0f : group->nogroup_opinion;
      return std::isnan(nogroup_opinion) ? 0.0f : nogroup_opinion;
    }
    
    OUTPUT_TRAIT_TYPE trait::get_opposite() const { return nullptr; }
    
    const utils::check_table_value trait_group_table[] = {
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
        "same_group_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "different_group_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "opposite_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "nogroup_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
    };
    
    const utils::check_table_value trait_table[] = {
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
        "opposite",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "group",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "birth_chance",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "inherit_chance",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "both_parent_inherit_chance",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "attributes",
        utils::check_table_value::type::array_t,
        0, 0, {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "same_group_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "different_group_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "opposite_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "nogroup_opinion",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
    };
    
    bool validate_trait_group(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "trait_group" + std::to_string(index);
      }

      const size_t size = sizeof(trait_group_table) / sizeof(trait_group_table[0]);
      recursive_check(check_str, "trait_group", table, nullptr, trait_group_table, size, counter);

      return counter == 0;
    }
    
    bool validate_trait(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str; 
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "trait" + std::to_string(index);
      }

      const size_t size = sizeof(trait_table) / sizeof(trait_table[0]);
      recursive_check(check_str, "trait", table, nullptr, trait_table, size, counter);

      return counter == 0;
    }
    
#define SET_NUM_FROM_TABLE(obj, name) if (const auto proxy = table[#name]; proxy.valid()) obj->name = proxy.get<float>();
    void parse_trait_group(core::trait_group* trait_group, const sol::table &table) {
      trait_group->id = table["id"];
      trait_group->name_id = table["name_id"];
      if (const auto proxy = table["description_id"]; proxy.valid()) trait_group->description_id = proxy.get<std::string>();
      
      SET_NUM_FROM_TABLE(trait_group, same_group_opinion)
      SET_NUM_FROM_TABLE(trait_group, different_group_opinion)
      SET_NUM_FROM_TABLE(trait_group, opposite_opinion)
      SET_NUM_FROM_TABLE(trait_group, nogroup_opinion)
    }

#define SET_NUM_FROM_TABLE2(obj, name) if (const auto proxy = table[#name]; proxy.valid()) obj->numeric_attribs.name = proxy.get<float>();
    void parse_trait(core::trait* trait, const sol::table &table) {
      auto ctx = global::get<systems::map_t>()->core_context.get();
      
      trait->id = table["id"];
      trait->name_id = table["name_id"];
      if (const auto proxy = table["description_id"]; proxy.valid()) trait->description_id = proxy.get<std::string>();
      
      if (const auto proxy = table["opposite"]; proxy.valid()) {
        const auto str = proxy.get<std::string_view>();
        const auto opposite = ctx->get_entity<core::trait>(str);
        if (opposite == nullptr) throw std::runtime_error("Could not find opposite trait " + std::string(str));
        trait->opposite = opposite;
      }
      
      if (const auto proxy = table["group"]; proxy.valid()) {
        const auto str = proxy.get<std::string_view>();
        // берется тоже из контекста
      }
      
      if (const auto proxy = table["attributes"]; proxy.valid()) {
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.second.get_type() != sol::type::string) continue;
          
          const auto str = pair.second.as<std::string_view>();
          const auto itr = core::trait_attributes::map.find(str);
          if (itr == core::trait_attributes::map.end()) throw std::runtime_error("Could not find trait attribute " + std::string(str));
          
          trait->set_attrib(itr->second, true);
        }
      }
      
      SET_NUM_FROM_TABLE2(trait, birth_chance)
      SET_NUM_FROM_TABLE2(trait, inherit_chance)
      SET_NUM_FROM_TABLE2(trait, both_parent_inherit_chance)
      
      SET_NUM_FROM_TABLE(trait, same_group_opinion)
      SET_NUM_FROM_TABLE(trait, different_group_opinion)
      SET_NUM_FROM_TABLE(trait, opposite_opinion)
      SET_NUM_FROM_TABLE(trait, nogroup_opinion)
      
      // что тут? треиты могут быть только у персонажа, поэтому тут довольно просто
      if (const auto proxy = table["bonuses"]; proxy.valid()) {
        size_t counter = 0;
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::number) continue;
          
          if (counter >= core::trait::max_stat_modifiers_count) throw std::runtime_error("Too many trait stat bonuses");
          
          const auto str = pair.first.as<std::string_view>();
          const double value = pair.second.as<double>();
          
          const auto itr = core::character_stats::map.find(str);
          if (itr == core::character_stats::map.end()) throw std::runtime_error("Could not find character stat " + std::string(str));
          
          const stat_modifier m(core::stat_type::character_stat, itr->second, value);
          trait->bonuses[counter] = m;
          ++counter;
        }
      }
      
      // пока не особо понимаю как будут выглядеть модификаторы отношений (даже не понимаю где толком они будут расчитываться)
      if (const auto proxy = table["opinion_bonuses"]; proxy.valid()) {
        size_t counter = 0;
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::number) continue;
          
          
        }
      }
    }
  }
}
