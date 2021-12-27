#include "religion.h"

#include "bin/data_parser.h"
#include "utils/globals.h"
#include "bin/map_creator.h"
#include "realm_mechanics_arrays.h"
#include "context.h"

namespace devils_engine {
  namespace core {
    const structure religion_group::s_type;
    const structure religion::s_type;
    religion::religion() : 
      group(nullptr), 
      parent(nullptr), 
      reformed(nullptr), 
      aggression(0.0f), 
      opinion_stat_index(UINT32_MAX),
      image{GPU_UINT_MAX},
      head(nullptr),
      head_heir(nullptr),
      believers(nullptr),
      secret_believers(nullptr)
    {}
    
    void religion::set_head(core::character* c) { head = c; }
    void religion::set_head_heir(core::character* c) { head_heir = c; }
    
    void religion::add_believer(core::character* believer) const {
      if (believers == nullptr) believers = believer;
      else utils::ring::list_radd<utils::list_type::believer>(believers, believer);
    }
    
    void religion::add_secret_believer(core::character* believer) const {
      if (secret_believers == nullptr) secret_believers = believer;
      else utils::ring::list_radd<utils::list_type::secret_believer>(secret_believers, believer);
    }
    
    void religion::remove_believer(core::character* believer) const {
      if (believers == believer) believers = utils::ring::list_next<utils::list_type::believer>(believers, believers);
    }
    
    void religion::remove_secret_believer(core::character* believer) const {
      if (secret_believers == believer) secret_believers = utils::ring::list_next<utils::list_type::secret_believer>(secret_believers, secret_believers);
    }
    
    core::religion_group* religion::get_religion_group() const {
      return group;
    }
    
    core::character* religion::get_head() const {
      return head;
    }
    
    core::character* religion::get_head_heir() const {
      return head_heir;
    }
    
    const utils::check_table_value religion_group_table[] = {
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
          },
          {
            "intermarriage",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "title_usurpation",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "holy_wars",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_religions",
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
          },
          {
            "intermarriage",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "title_usurpation",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "holy_wars",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_faiths",
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
          },
          {
            "intermarriage",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "title_usurpation",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "holy_wars",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      }
    };
    
    const utils::check_table_value religion_table[] = {
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
        "aggression",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "crusade_name_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "holy_order_names_table_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "scripture_name_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "good_gods_table_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "evil_gods_table_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "high_god_name_id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "piety_name_id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "priest_title_name_id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "reserved_male_names_table_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "reserved_female_names_table_id",
        utils::check_table_value::type::string_t,
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
          },
          {
            "intermarriage",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "title_usurpation",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "holy_wars",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_religions",
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
          },
          {
            "intermarriage",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "title_usurpation",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "holy_wars",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "different_faiths",
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
          },
          {
            "intermarriage",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "title_usurpation",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "holy_wars",
            utils::check_table_value::type::bool_t,
            utils::check_table_value::value_required, 0, {}
          }
        }
      }
    };
    
    size_t add_religion_group(const sol::table &table) {
      return global::get<devils_engine::map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::religion_group), table);
    }
    
    bool validate_religion_group(const size_t &index, const sol::table &table)  {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "religion_group" + std::to_string(index);
      }

      const size_t size = sizeof(religion_group_table) / sizeof(religion_group_table[0]);
      recursive_check(check_str, "religion_group", table, nullptr, religion_group_table, size, counter);

      return counter == 0;
    }
    
    void parse_religion_group(core::religion_group* religion_group, const sol::table &table) {
      {
        const auto id = table["id"].get<std::string>();
        religion_group->id = id;
      }
      
      {
        const auto name_id = table["name_id"].get<std::string>();
        religion_group->name_id = name_id;
      }
      
      if (const auto description_id = table["description_id"]; description_id.valid() && description_id.get_type() == sol::type::string) {
        religion_group->description_id = description_id;
      }
      
      {
        const auto t = table["different_groups"].get<sol::table>();
        religion_group->different_groups.character_opinion = t["character_opinion"];
        religion_group->different_groups.popular_opinion = t["popular_opinion"];
        const bool intermarriage = t["intermarriage"];
        const bool title_usurpation = t["title_usurpation"];
        const bool holy_wars = t["holy_wars"];
        religion_group->different_groups.hostility.set(religion_opinion_data::intermarriage, intermarriage);
        religion_group->different_groups.hostility.set(religion_opinion_data::title_usurpation, title_usurpation);
        religion_group->different_groups.hostility.set(religion_opinion_data::holy_wars, holy_wars);
      }
      
      {
        const auto t = table["different_religions"].get<sol::table>();
        religion_group->different_religions.character_opinion = t["character_opinion"];
        religion_group->different_religions.popular_opinion = t["popular_opinion"];
        const bool intermarriage = t["intermarriage"];
        const bool title_usurpation = t["title_usurpation"];
        const bool holy_wars = t["holy_wars"];
        religion_group->different_religions.hostility.set(religion_opinion_data::intermarriage, intermarriage);
        religion_group->different_religions.hostility.set(religion_opinion_data::title_usurpation, title_usurpation);
        religion_group->different_religions.hostility.set(religion_opinion_data::holy_wars, holy_wars);
      }
      
      {
        const auto t = table["different_faiths"].get<sol::table>();
        religion_group->different_faiths.character_opinion = t["character_opinion"];
        religion_group->different_faiths.popular_opinion = t["popular_opinion"];
        const bool intermarriage = t["intermarriage"];
        const bool title_usurpation = t["title_usurpation"];
        const bool holy_wars = t["holy_wars"];
        religion_group->different_faiths.hostility.set(religion_opinion_data::intermarriage, intermarriage);
        religion_group->different_faiths.hostility.set(religion_opinion_data::title_usurpation, title_usurpation);
        religion_group->different_faiths.hostility.set(religion_opinion_data::holy_wars, holy_wars);
      }
    }
    
    size_t add_religion(const sol::table &table) {
      return global::get<devils_engine::map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::religion), table);
    }
    
    bool validate_religion(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "religion" + std::to_string(index);
      }

      const size_t size = sizeof(religion_table) / sizeof(religion_table[0]);
      recursive_check(check_str, "religion", table, nullptr, religion_table, size, counter);

      return counter == 0;
    }
    
    void parse_religion(core::religion* religion, const sol::table &table) {
      auto ctx = global::get<core::context>();
      
      {
        const auto id = table["id"].get<std::string>();
        religion->id = id;
      }
      
      {
        const auto name_id = table["name_id"].get<std::string>();
        religion->name_id = name_id;
      }
      
      if (const auto description_id = table["description_id"]; description_id.valid() && description_id.get_type() == sol::type::string) {
        religion->description_id = description_id;
      }
      
      {
        const auto group = table["group"];
        const auto str = group.get<std::string_view>();
        const auto group_ptr = ctx->get_entity<core::religion_group>(str);
        if (group_ptr == nullptr) throw std::runtime_error("Could not find religion group " + std::string(str));
        religion->group = group_ptr;
      }
      
      if (const auto parent = table["parent"]; parent.valid() && parent.get_type() == sol::type::string) {
        const auto str = parent.get<std::string_view>();
        const auto parent_ptr = ctx->get_entity<core::religion>(str);
        if (parent_ptr == nullptr) throw std::runtime_error("Could not find parent religion " + std::string(str));
        religion->parent = parent_ptr;
      }
      
      if (const auto crusade_name_id = table["crusade_name_id"]; crusade_name_id.valid() && crusade_name_id.get_type() == sol::type::string) {
        religion->crusade_name_id = crusade_name_id;
      }
      
      if (const auto holy_order_names_table_id = table["holy_order_names_table_id"]; holy_order_names_table_id.valid() && holy_order_names_table_id.get_type() == sol::type::string) {
        religion->holy_order_names_table_id = holy_order_names_table_id;
      }
      
      if (const auto scripture_name_id = table["scripture_name_id"]; scripture_name_id.valid() && scripture_name_id.get_type() == sol::type::string) {
        religion->scripture_name_id = scripture_name_id;
      }
      
      if (const auto good_gods_table_id = table["good_gods_table_id"]; good_gods_table_id.valid() && good_gods_table_id.get_type() == sol::type::number) {
        religion->good_gods_table_id = good_gods_table_id;
      }
      
      if (const auto evil_gods_table_id = table["evil_gods_table_id"]; evil_gods_table_id.valid() && evil_gods_table_id.get_type() == sol::type::number) {
        religion->evil_gods_table_id = evil_gods_table_id;
      }
      
      {
        const auto high_god_name_id = table["high_god_name_id"];
        religion->high_god_name_id = high_god_name_id;
      }
      
      {
        const auto piety_name_id = table["piety_name_id"];
        religion->piety_name_id = piety_name_id;
      }
      
      {
        const auto priest_title_name_id = table["priest_title_name_id"];
        religion->priest_title_name_id = priest_title_name_id;
      }
      
      if (const auto reserved_male_names_table_id = table["reserved_male_names_table_id"]; reserved_male_names_table_id.valid() && reserved_male_names_table_id.get_type() == sol::type::number) {
        religion->reserved_male_names_table_id = reserved_male_names_table_id;
      }
      
      if (const auto reserved_female_names_table_id = table["reserved_female_names_table_id"]; reserved_female_names_table_id.valid() && reserved_female_names_table_id.get_type() == sol::type::number) {
        religion->reserved_female_names_table_id = reserved_female_names_table_id;
      }
      
      if (const auto proxy = table["different_groups"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto t = proxy.get<sol::table>();
        religion->different_groups.character_opinion = t["character_opinion"];
        religion->different_groups.popular_opinion = t["popular_opinion"];
        const bool intermarriage = t["intermarriage"];
        const bool title_usurpation = t["title_usurpation"];
        const bool holy_wars = t["holy_wars"];
        religion->different_groups.hostility.set(religion_opinion_data::intermarriage, intermarriage);
        religion->different_groups.hostility.set(religion_opinion_data::title_usurpation, title_usurpation);
        religion->different_groups.hostility.set(religion_opinion_data::holy_wars, holy_wars);
      }
      
      if (const auto proxy = table["different_religions"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto t = proxy.get<sol::table>();
        religion->different_religions.character_opinion = t["character_opinion"];
        religion->different_religions.popular_opinion = t["popular_opinion"];
        const bool intermarriage = t["intermarriage"];
        const bool title_usurpation = t["title_usurpation"];
        const bool holy_wars = t["holy_wars"];
        religion->different_religions.hostility.set(religion_opinion_data::intermarriage, intermarriage);
        religion->different_religions.hostility.set(religion_opinion_data::title_usurpation, title_usurpation);
        religion->different_religions.hostility.set(religion_opinion_data::holy_wars, holy_wars);
      }
      
      if (const auto proxy = table["different_faiths"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto t = proxy.get<sol::table>();
        religion->different_faiths.character_opinion = t["character_opinion"];
        religion->different_faiths.popular_opinion = t["popular_opinion"];
        const bool intermarriage = t["intermarriage"];
        const bool title_usurpation = t["title_usurpation"];
        const bool holy_wars = t["holy_wars"];
        religion->different_faiths.hostility.set(religion_opinion_data::intermarriage, intermarriage);
        religion->different_faiths.hostility.set(religion_opinion_data::title_usurpation, title_usurpation);
        religion->different_faiths.hostility.set(religion_opinion_data::holy_wars, holy_wars);
      }
      
      if (const auto features = table["features"]; features.valid() && features.get_type() == sol::type::table) {
        for (size_t i = 0; i < core::religion_mechanics::count; ++i) {
          const auto name = core::religion_mechanics::names[i];
          const auto proxy = features[name];
          if (proxy.valid() && proxy.get_type() == sol::type::boolean) {
            const bool val = proxy;
            religion->set_mechanic(i, val);
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
      
      // бонусы на своей земле
      // бонусы к отношениям
    }
  }
}
