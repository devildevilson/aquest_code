#include "bin/data_parser.h"

#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "utils/serializator_helper.h"

#include "context.h"
#include "stats_table.h"

#include "bin/map_creator.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace core {
    const utils::check_table_value character_table[] = {
      {
        "family",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, 0,
        {
          {
            "parents",
            utils::check_table_value::type::array_t,
            0, 2,
            {
              {
                STATS_ARRAY,
                utils::check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          },
          {
            "owner",
            utils::check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "consort",
            utils::check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "dynasty",
            utils::check_table_value::type::int_t,
            0, 0, {}
          }
        }
      },
      {
        "relations",
        utils::check_table_value::type::array_t,
        0, 0,
        {
          {
            "friends",
            utils::check_table_value::type::array_t,
            0, 8,
            {
              {
                STATS_ARRAY,
                utils::check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          },
          {
            "rivals",
            utils::check_table_value::type::array_t,
            0, 8,
            {
              {
                STATS_ARRAY,
                utils::check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          },
          {
            "lovers",
            utils::check_table_value::type::array_t,
            0, 8,
            {
              {
                STATS_ARRAY,
                utils::check_table_value::type::int_t,
                0, UINT32_MAX, {}
              }
            }
          }
        }
      },
      {
        "hidden_religion",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "suzerain",
        utils::check_table_value::type::int_t,
        0, 0, {}
      },
      {
        "imprisoner",
        utils::check_table_value::type::int_t,
        0, 0, {}
      },
      {
        "stats",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, 0,
        {
          {
            STATS_V2_ARRAY,
            utils::check_table_value::type::int_t,
            core::offsets::character_stats, core::offsets::realm_stats + core::realm_stats::count, {}
          }
        }
      },
      {
        "realm_stats",
        utils::check_table_value::type::array_t,
        0, core::hero_stats::count,
        {
          {
            STATS_V2_ARRAY,
            utils::check_table_value::type::int_t,
            core::offsets::hero_stats, core::offsets::hero_stats + core::hero_stats::count, {}
          }
        }
      },
      {
        "liege",
        utils::check_table_value::type::int_t,
        0, 0, {}
      },
      {
        "titles",
        utils::check_table_value::type::array_t,
        0, 0,
        {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      }
    };

    size_t add_character(const sol::table &table) {
      return global::get<devils_engine::map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::character), table);
    }

    size_t register_character() {
      return global::get<devils_engine::map::creator::table_container_t>()->register_table(static_cast<size_t>(core::structure::character));
    }

    size_t register_characters(const size_t &count) {
      return global::get<devils_engine::map::creator::table_container_t>()->register_tables(static_cast<size_t>(core::structure::character), count);
    }

    void set_character(const size_t &index, const sol::table &table) {
      global::get<devils_engine::map::creator::table_container_t>()->set_table(static_cast<size_t>(core::structure::character), index, table);
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

    bool validate_character_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator*) {
      const bool ret = validate_character(index, table);
      if (!ret) return false;

      sol::state_view state(lua);
      auto str = utils::table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize character table");
      ASSERT(false);
      //container->add_data(core::structure::character, std::move(str));

      return true;
    }

    void parse_character(core::character* character, const sol::table &table) {
      auto ctx = global::get<core::context>();
      
//       const size_t char_index1 = ctx->get_character_debug_index(character);
      const auto dbg = [ctx, character] () {
        return ctx->get_character_debug_index(character);
      };

      // откуда брать персонажей то и династии

      if (auto t = table["name"]; t.valid() && t.get_type() == sol::type::table) {
        const auto id = t[1];
        const auto index = t[2];
        if (!id.valid() || id.get_type() != sol::type::string) throw std::runtime_error("Bad name table id value");

        character->name_table_id = id.get<std::string>();
        if (index.valid()) {
          if (index.get_type() != sol::type::number) throw std::runtime_error("Bad name table index value");

          character->name_index = index.get<uint32_t>();
        }
      }

      if (auto t = table["nickname"]; t.valid() && t.get_type() == sol::type::table) {
        const auto id = t[1];
        const auto index = t[2];
        if (!id.valid() || id.get_type() != sol::type::string) throw std::runtime_error("Bad name table id value");

        character->nickname_table_id = id.get<std::string>();
        if (index.valid()) {
          if (index.get_type() != sol::type::number) throw std::runtime_error("Bad name table index value");

          character->nickname_index = index.get<uint32_t>();
        }
      }

      {
        const auto &family_table = table.get<sol::table>("family");
        if (const auto &p = family_table["parents"]; p.valid()) {
          const auto &parents = p.get<sol::table>();
          if (const auto &tmp = parents[1]; tmp.valid()) {
            const size_t index = FROM_LUA_INDEX(tmp.get<size_t>());
            auto c = ctx->get_character(index);
            if (c == nullptr) throw std::runtime_error("Could not find parent character " + std::to_string(index) + " for character " + std::to_string(dbg()));
            character->family.parents[0] = c;
          }

          if (const auto &tmp = parents[2]; tmp.valid()) {
            const size_t index = FROM_LUA_INDEX(tmp.get<size_t>());
            auto c = ctx->get_character(index);
            if (c == nullptr) throw std::runtime_error("Could not find parent character " + std::to_string(index) + " for character " + std::to_string(dbg()));
            character->family.parents[1] = c;
          }

          if (character->family.parents[0] != nullptr && character->family.parents[1] != nullptr) {
            if (uint32_t(character->family.parents[0]->is_male()) + uint32_t(character->family.parents[1]->is_male()) != 1) throw std::runtime_error("Parents must be different gender");
            if (!character->family.parents[0]->is_male()) std::swap(character->family.parents[0], character->family.parents[1]);
          }
        }

        // братьев и сестер мы можем проверить по родителям

        if (const auto &o = family_table["owner"]; o.valid()) {
          const size_t index = FROM_LUA_INDEX(o.get<size_t>());
          auto c = ctx->get_character(index);
          if (c == nullptr) throw std::runtime_error("Could not find owner character " + std::to_string(index) + " for character " + std::to_string(dbg()));
          character->family.owner = c;
        }

        if (const auto &c = family_table["consort"]; c.valid()) {
          const size_t index = FROM_LUA_INDEX(c.get<size_t>());
          auto ch = ctx->get_character(index);
          if (c == nullptr) throw std::runtime_error("Could not find consort character " + std::to_string(index) + " for character " + std::to_string(dbg()));
          character->family.consort = ch;
        }

        // в династии чет пока что мало что нужно указывать
        if (const auto &d = family_table["dynasty"]; d.valid()) {
          const size_t index = FROM_LUA_INDEX(d.get<size_t>()); // тут по идее нужен id
          auto dynasty = ctx->get_dynasty(index);
          if (dynasty == nullptr) throw std::runtime_error("Could not find dynasty " + std::to_string(index) + " for character " + std::to_string(dbg()));
          character->family.dynasty = dynasty;
        }
      }

      {
//         if (const auto &relations_table = table["relations"]; relations_table.valid()) {
//           if (const auto &f = relations_table["friends"]; f.valid()) {
//             const auto &friends = f.get<sol::table>();
//             size_t counter = 0;
//             for (const auto &pair : friends) {
//               if (pair.second.get_type() != sol::type::number) continue;
// 
//               const size_t index = FROM_LUA_INDEX(pair.second.as<size_t>());
//               auto c = ctx->get_character(index);
//               if (c == nullptr) throw std::runtime_error("Could not find friend character " + std::to_string(index) + " for character " + std::to_string(char_index1));
//               if (counter >= core::character::relations::max_game_friends) throw std::runtime_error("Bad relations data");
//               character->relations.friends[counter] = c;
//               ++counter;
//             }
//           }
// 
//           if (const auto &r = relations_table["rivals"]; r.valid()) {
//             const auto &rivals = r.get<sol::table>();
//             size_t counter = 0;
//             for (const auto &pair : rivals) {
//               if (pair.second.get_type() != sol::type::number) continue;
// 
//               const size_t index = FROM_LUA_INDEX(pair.second.as<size_t>());
//               auto c = ctx->get_character(index);
//               if (c == nullptr) throw std::runtime_error("Could not find rival character " + std::to_string(index) + " for character " + std::to_string(char_index1));
//               if (counter >= core::character::relations::max_game_rivals) throw std::runtime_error("Bad relations data");
//               character->relations.rivals[counter] = c;
//               ++counter;
//             }
//           }
// 
//           if (const auto &l = relations_table["lovers"]; l.valid()) {
//             const auto &lovers = l.get<sol::table>();
//             size_t counter = 0;
//             for (const auto &pair : lovers) {
//               if (pair.second.get_type() != sol::type::number) continue;
// 
//               const size_t index = FROM_LUA_INDEX(pair.second.as<size_t>());
//               auto c = ctx->get_character(index);
//               if (c == nullptr) throw std::runtime_error("Could not find lover character " + std::to_string(index) + " for character " + std::to_string(char_index1));
//               if (counter >= core::character::relations::max_game_lovers) throw std::runtime_error("Bad relations data");
//               character->relations.lovers[counter] = c;
//               ++counter;
//             }
//           }
//         }
      }

      {
        const std::string_view &str = table["culture"].get<std::string_view>();
        auto ptr = ctx->get_entity<core::culture>(str);
        if (ptr == nullptr) throw std::runtime_error("Could not find culture " + std::string(str));
        character->culture = ptr;
      }

      {
        const std::string_view &str = table["religion"].get<std::string_view>();
        auto ptr = ctx->get_entity<core::religion>(str);
        if (ptr == nullptr) throw std::runtime_error("Could not find religion " + std::string(str));
        character->religion = ptr;
      }

      if (const auto &rel = table["hidden_religion"]; rel.valid()) {
        const std::string_view &str = rel.get<std::string_view>();
        auto ptr = ctx->get_entity<core::religion>(str);
        if (ptr == nullptr) throw std::runtime_error("Could not find religion " + std::string(str));
        if (character->religion == ptr) throw std::runtime_error("Hidden religion must not be main religion");
        character->secret_religion = ptr;
      }
      
      if (const auto proxy = table["titles"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto titles = proxy.get<sol::table>();
        // здесь пока что не проверить является ли титул формальным, но при этом мне нужно проверить чтобы у челика не были только формальные титулы
        // это проверяется видимо позже
        for (const auto &pair : titles) {
          if (pair.second.get_type() != sol::type::string) continue;
          
          const auto title_id = pair.second.as<std::string_view>();
          auto title = ctx->get_entity<core::titulus>(title_id);
          if (title == nullptr) throw std::runtime_error("Could not find title " + std::string(title_id));
          // было бы неплохо тут идентифицировать кому мы пытаемся пихнуть чей титул
          if (title->owner.valid()) {
            const size_t char_index2 = ctx->get_character_debug_index(title->owner.get()->leader);
            throw std::runtime_error("Title " + std::string(title_id) + " is already has an owner " + std::to_string(char_index2) + ". Trying to add title to character " + std::to_string(dbg()));
          }
          
          if (!character->self.valid()) {
            character->self = ctx->create_realm();
            character->self->leader = character;
          }
          
          character->add_title(title);
        }
      }
    }

    // фракции: вассалы, титулы, законы, господин, совет, суд,
    // к сожалению титулы и вассалы не сходятся один ко одному
    // наследника нужно посчитать по исходным данным

    // для начала тут нужно указать форму правления
    // если есть форма правления то есть и вассалы и титулы
    
    // как определяется реалм? законами государства, мне нужно задать законы
    // вообще такое чувство что имеет смысл разделить конфиг для реалма и персонажа
    // но с другой стороны как то серьезно отделить вещи друг от друга довольно сложно
    // так или иначе нужно передать таблицу законов и прав
    // но еще нужно создать фракции внутри государства
    void parse_character_goverment(core::character* character, const sol::table &table) {
      auto ctx = global::get<core::context>();
      
      //const size_t char_index1 = ctx->get_character_debug_index(character);
      const auto dbg = [ctx, character] () {
        return ctx->get_character_debug_index(character);
      };

      // нужно бы проверить есть ли титулы у сюзерена
      if (const auto &suzerain = table["suzerain"]; suzerain.valid() && suzerain.get_type() == sol::type::number) {
        const auto &index = FROM_LUA_INDEX(suzerain.get<size_t>());
        auto c = ctx->get_character(index);
        if (c == nullptr) throw std::runtime_error("Could not find character " + std::to_string(index));
        auto r = c->self;
        if (!r.valid()) throw std::runtime_error("Character " + std::to_string(index) + " could not be a suzerain to character " + std::to_string(dbg()));
        r->add_courtier(character);
      }

      // какая фракция? нужно ли делать тюрьму в элективном органе?
      if (const auto &imprisoner = table["imprisoner"]; imprisoner.valid() && imprisoner.get_type() == sol::type::number) {
        const auto &index = FROM_LUA_INDEX(imprisoner.get<size_t>());
        auto c = ctx->get_character(index);
        if (c == nullptr) throw std::runtime_error("Could not find character " + std::to_string(index));
        auto r = c->self;
        if (!r.valid()) throw std::runtime_error("Character " + std::to_string(index) + " could not be an imprisoner to character " + std::to_string(dbg()));
        r->add_prisoner(character);
      }
      
      // статы у персонажа или реалма могут совпадать, разделить статы перса и реалма?
      if (const auto proxy = table["stats"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto &stats = proxy.get<sol::table>();
        for (const auto &pair : stats) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::number) continue;
          
          const auto &stat = pair.first.as<std::string_view>();
          const float value = pair.second.as<double>();
          
          const auto itr = core::stats_list::map.find(stat);
          if (itr == core::stats_list::map.end()) throw std::runtime_error("Could not find stat " + std::string(stat));
          
          if (const uint32_t index = stats_list::to_character_stat(itr->second); index != UINT32_MAX) {
            character->stats.set(index, value);
            continue;
          }
          
          if (const uint32_t index = stats_list::to_hero_stat(itr->second); index != UINT32_MAX) {
            character->hero_stats.set(index, value);
            continue;
          }
          
          if (const uint32_t index = stats_list::to_character_resource(itr->second); index != UINT32_MAX) {
            character->resources.set(index, value);
            continue;
          }
          
          throw std::runtime_error("Stat " + std::string(stat) + " is not belongs to charater stats");
        }
      }
      
      if (const auto proxy = table["realm_stats"]; proxy.valid() && proxy.get_type() == sol::type::table && character->self.valid()) {
        auto r = character->self;
        const auto &stats = proxy.get<sol::table>();
        for (const auto &pair : stats) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::number) continue;
          
          const auto &stat = pair.first.as<std::string_view>();
          const float value = pair.second.as<double>();
          
          const auto itr = core::stats_list::map.find(stat);
          if (itr == core::stats_list::map.end()) throw std::runtime_error("Could not find stat " + std::string(stat));
          
          if (const uint32_t index = stats_list::to_realm_stat(itr->second); index != UINT32_MAX) {
            r->stats.set(index, value);
            continue;
          }
          
          if (const uint32_t index = stats_list::to_realm_resource(itr->second); index != UINT32_MAX) {
            r->resources.set(index, value);
            continue;
          }
          
          throw std::runtime_error("Stat " + std::string(stat) + " is not belongs to realm stats");
        }
      }

      if (const auto &liege = table["liege"]; liege.valid()) {
        if (character->self == nullptr) {
          auto realm = ctx->create_realm();
          character->self = realm;
          character->self->leader = character;
        }

        const size_t index = FROM_LUA_INDEX(liege.get<size_t>());
        auto c = ctx->get_character(index);
        if (c == nullptr) throw std::runtime_error("Could not find character " + std::to_string(index));
        if (!c->self.valid()) throw std::runtime_error("Character " + std::to_string(index) + " does not have any titles, thus he cant own any vassals");
        
        c->add_vassal(character);
      }
    }
  }
}
