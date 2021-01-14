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
    const check_table_value character_table[] = {
      {
        "family",
        check_table_value::type::array_t,
        check_table_value::value_required, 0, 
        {
          {
            "parents",
            check_table_value::type::array_t,
            0, 2, 
            {
              {
                STATS_ARRAY,
                check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          },
          {
            "owner",
            check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "consort",
            check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "dynasty",
            check_table_value::type::int_t,
            0, 0, {}
          }
        }
      },
      {
        "relations",
        check_table_value::type::array_t,
        0, 0, 
        {
          {
            "friends",
            check_table_value::type::array_t,
            0, core::character::relations::max_game_friends, 
            {
              {
                STATS_ARRAY,
                check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          },
          {
            "rivals",
            check_table_value::type::array_t,
            0, core::character::relations::max_game_rivals, 
            {
              {
                STATS_ARRAY,
                check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          },
          {
            "lovers",
            check_table_value::type::array_t,
            0, core::character::relations::max_game_lovers, 
            {
              {
                STATS_ARRAY,
                check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          }
        }
      },
      {
        "hero_stats",
        check_table_value::type::array_t,
        0, core::hero_stats::count, 
        {
          {
            STATS_ARRAY,
            check_table_value::type::int_t,
            core::offsets::hero_stats, core::offsets::hero_stats + core::hero_stats::count, {}
          }
        }
      },
      {
        "hidden_religion",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "suzerain",
        check_table_value::type::int_t,
        0, 0, {}
      },
      {
        "imprisoner",
        check_table_value::type::int_t,
        0, 0, {}
      },
      {
        "stats",
        check_table_value::type::array_t,
        check_table_value::value_required, 0, 
        {
          {
            STATS_ARRAY,
            check_table_value::type::int_t,
            core::offsets::character_stats, core::offsets::faction_stats + core::faction_stats::count, {}
          }
        }
      },
      {
        "liege",
        check_table_value::type::int_t,
        0, 0, {}
      },
      {
        "titles",
        check_table_value::type::array_t,
        0, 0, 
        {
          {
            ID_ARRAY,
            check_table_value::type::string_t,
            0, 0, {}
          }
        }
      }
    };
    
    size_t add_character(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::character), table);
    }
    
    size_t register_character() {
      return global::get<map::creator::table_container_t>()->register_table(static_cast<size_t>(core::structure::character));
    }
    
    size_t register_characters(const size_t &count) {
      return global::get<map::creator::table_container_t>()->register_tables(static_cast<size_t>(core::structure::character), count);
    }
    
    void set_character(const size_t &index, const sol::table &table) {
      global::get<map::creator::table_container_t>()->set_table(static_cast<size_t>(core::structure::character), index, table);
    }
    
    bool validate_character(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "character" + std::to_string(index);
      }
      
      const size_t size = sizeof(character_table) / sizeof(character_table[0]);
      recursive_check(check_str, "character", table, nullptr, character_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_character_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_character(index, table);
      if (!ret) return false;
      
      sol::state_view state(lua);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize character table");
      container->add_data(core::structure::character, std::move(str));
      
      return true;
    }
    
    void parse_character(core::character* character, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      // откуда брать персонажей то и династии
      
      {
        const auto &family_table = table.get<sol::table>("family");
        if (const auto &p = family_table["parents"]; p.valid()) {
          const auto &parents = p.get<sol::table>();
          if (const auto &tmp = parents[0]; tmp.valid()) {
            const size_t index = tmp.get<size_t>();
            auto c = ctx->get_character(index);
            character->family.parents[0] = c;
          }
          
          if (const auto &tmp = parents[1]; tmp.valid()) {
            const size_t index = tmp.get<size_t>();
            auto c = ctx->get_character(index);
            character->family.parents[1] = c;
          }
          
          if (character->family.parents[0] != nullptr && character->family.parents[1] != nullptr) {
            if (uint32_t(character->family.parents[0]->is_male()) + uint32_t(character->family.parents[1]->is_male()) != 1) throw std::runtime_error("Parents must be different genders");
            if (!character->family.parents[0]->is_male()) std::swap(character->family.parents[0], character->family.parents[1]);
          }
        }
        
//         if (const auto &gp = family_table["grand_parents"]; gp.valid()) {
//           const auto &grand_parents = gp.get<sol::table>();
//           size_t counter = 0;
//           for (auto itr = grand_parents.begin(); itr != grand_parents.end(); ++itr) {
//             if (!(*itr).second.is<size_t>()) continue;
//             const size_t index = (*itr).second.as<size_t>();
//             auto c = ctx->get_character(index);
//             if (counter >= 4) throw std::runtime_error("Bad grandparents data");
//             character->family.grandparents[counter] = c;
//             ++counter;
//           }
//           
//           uint32_t male_counter = 0;
//           uint32_t female_counter = 0;
//           for (uint32_t i = 0; i < 4; ++i) {
//             if (character->family.grandparents[i] == nullptr) continue;
//             male_counter += uint32_t(character->family.grandparents[i]->is_male());
//             female_counter += uint32_t(!character->family.grandparents[i]->is_male());
//           }
//           
//           if (male_counter > 2) throw std::runtime_error("Bad grandparents data");
//           if (female_counter > 2) throw std::runtime_error("Bad grandparents data");
//         }
        
        // братьев и сестер мы можем проверить по родителям
        
        if (const auto &o = family_table["owner"]; o.valid()) {
          const size_t index = o.get<size_t>();
          auto c = ctx->get_character(index);
          character->family.owner = c;
        }
        
        if (const auto &c = family_table["consort"]; c.valid()) {
          const size_t index = c.get<size_t>();
          auto ch = ctx->get_character(index);
          character->family.consort = ch;
        }
        
        // в династии чет пока что мало что нужно указывать
        if (const auto &d = family_table["dynasty"]; d.valid()) {
          const size_t index = d.get<size_t>();
          auto dynasty = ctx->get_dynasty(index);
          character->family.dynasty = dynasty;
        }
      }
      
      {
        if (const auto &relations_table = table["relations"]; relations_table.valid()) {
          if (const auto &f = relations_table["friends"]; f.valid()) {
            const auto &friends = f.get<sol::table>();
            size_t counter = 0;
            for (auto itr = friends.begin(); itr != friends.end(); ++itr) {
              if (!(*itr).second.is<size_t>()) continue;
              
              const size_t index = (*itr).second.as<size_t>();
              auto c = ctx->get_character(index);
              if (counter >= core::character::relations::max_game_friends) throw std::runtime_error("Bad relations data");
              character->relations.friends[counter] = c;
              ++counter;
            }
          }
          
          if (const auto &r = relations_table["rivals"]; r.valid()) {
            const auto &rivals = r.get<sol::table>();
            size_t counter = 0;
            for (auto itr = rivals.begin(); itr != rivals.end(); ++itr) {
              if (!(*itr).second.is<size_t>()) continue;
              
              const size_t index = (*itr).second.as<size_t>();
              auto c = ctx->get_character(index);
              if (counter >= core::character::relations::max_game_rivals) throw std::runtime_error("Bad relations data");
              character->relations.rivals[counter] = c;
              ++counter;
            }
          }
          
          if (const auto &l = relations_table["lovers"]; l.valid()) {
            const auto &lovers = l.get<sol::table>();
            size_t counter = 0;
            for (auto itr = lovers.begin(); itr != lovers.end(); ++itr) {
              if (!(*itr).second.is<size_t>()) continue;
              
              const size_t index = (*itr).second.as<size_t>();
              auto c = ctx->get_character(index);
              if (counter >= core::character::relations::max_game_lovers) throw std::runtime_error("Bad relations data");
              character->relations.lovers[counter] = c;
              ++counter;
            }
          }
        }
      }
      
      {
        const auto &stats = table.get<sol::table>("hero_stats");
        for (auto itr = stats.begin(); itr != stats.end(); ++itr) {
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).second.is<double>()) continue;
          
          const auto &stat = (*itr).first.as<std::string>();
          const float value = (*itr).second.as<float>();
          
          if (const auto &val = magic_enum::enum_cast<core::hero_stats::values>(stat); val.has_value()) {
            const size_t stat_id = val.value();
            character->hero_stats[stat_id].ival = value;
          }
        }
      }
      
//       {
//         const std::string &str = table["culture"];
//         const size_t index = to_data->get(str);
//         if (index == SIZE_MAX) throw std::runtime_error("Could not find culture " + str);
//         auto ptr = ctx->get_entity<core::culture>(index);
//         character->culture = ptr;
//       }
//       
//       {
//         const std::string &str = table["religion"];
//         const size_t index = to_data->get(str);
//         if (index == SIZE_MAX) throw std::runtime_error("Could not find religion " + str);
//         auto ptr = ctx->get_entity<core::religion>(index);
//         character->religion = ptr;
//       }
      
      if (const auto &rel = table["hidden_religion"]; rel.valid()) {
        const std::string &str = rel.get<std::string>();
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find religion " + str);
        auto ptr = ctx->get_entity<core::religion>(index);
        character->hidden_religion = ptr;
        ASSERT(character->religion != character->hidden_religion);
      }
      
      // фракции: вассалы, титулы, законы, господин, совет, суд, 
      // к сожалению титулы и вассалы не сходятся один ко одному
      // наследника нужно посчитать по исходным данным
      
      // для начала тут нужно указать форму правления
      // если есть форма правления то есть и вассалы и титулы
    }
    
    void parse_character_goverment(core::character* character, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      // нужно бы проверить есть ли титулы у сюзерена
      if (const auto &suzerain = table["suzerain"]; suzerain.valid()) {
        const auto &index = suzerain.get<size_t>();
        auto c = ctx->get_character(index);
        ASSERT(c->factions[core::character::self] != nullptr);
        character->suzerain = c;
      }
      
      // какая фракция? нужно ли делать тюрьму в элективном органе?
      if (const auto &imprisoner = table["imprisoner"]; imprisoner.valid()) {
        const auto &index = imprisoner.get<size_t>();
        auto c = ctx->get_character(index);
        ASSERT(c->factions[core::character::self] != nullptr);
        character->imprisoner = c->factions[core::character::self];
      }
      
      {
        const auto &stats = table.get<sol::table>("stats");
        for (auto itr = stats.begin(); itr != stats.end(); ++itr) {
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).second.is<double>()) continue;
          
          const auto &stat = (*itr).first.as<std::string>();
          const float value = (*itr).second.as<float>();
          
          if (const auto &val = magic_enum::enum_cast<core::character_stats::values>(stat); val.has_value()) {
            const size_t stat_id = val.value();
            switch (core::character_stats::types[stat_id]) {
              case core::stat_type::float_t: character->stats[stat_id].fval = value; break;
              case core::stat_type::uint_t:  character->stats[stat_id].uval = value; break;
              case core::stat_type::int_t:   character->stats[stat_id].ival = value; break;
            }
            
            continue;
          }
          
          // статы элективной фракции? скорее всего здесь укажем статы именно персонажа
          if (const auto &val = magic_enum::enum_cast<core::faction_stats::values>(stat); val.has_value()) {
            if (character->factions[core::character::self] == nullptr) character->factions[core::character::self] = ctx->create_faction();
            const size_t stat_id = val.value();
            character->factions[core::character::self]->stats[stat_id].fval = value;
            continue;
          }
        }
      }

      if (const auto &liege = table["liege"]; liege.valid()) {
        if (character->factions[core::character::self] == nullptr) character->factions[core::character::self] = ctx->create_faction();
        
        const size_t index = liege.get<size_t>();
        auto c = ctx->get_character(index);
        if (c->factions[core::character::self] == nullptr) c->factions[core::character::self] = ctx->create_faction();
        character->factions[core::character::self]->liege = c->factions[core::character::self]; // наверное нужно указать государство
        // я так понимаю нужно разделить фракцию и стейт, хотя наверное нет
        // мы создаем форму правления для персонажей, нужно разделять парсер?
      }
      
      if (const auto &titles = table["titles"]; titles.valid()) {
        if (character->factions[core::character::self] == nullptr) character->factions[core::character::self] = ctx->create_faction();
        
        std::vector<core::titulus*> character_titles;
        const auto &table = titles.get<sol::table>();
        for (auto itr = table.begin(); itr != table.end(); ++itr) {
          if (!(*itr).second.is<std::string>()) continue;
          
          const std::string &id = (*itr).second.as<std::string>();
          const size_t index = to_data->get(id);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find title " + id);
          auto t = ctx->get_entity<core::titulus>(index);
          character_titles.push_back(t);
        }
        
        ASSERT(!character_titles.empty());
        
        // некоторые титулы будут принадлежать государству (титулы максимального ранга)
        auto char_faction = character->factions[core::character::self];
        for (auto title : character_titles) {
//           title->owner = char_faction;
          char_faction->add_title(title);
        }
      }
    }
  }
}
