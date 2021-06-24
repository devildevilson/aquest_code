#include "lua_initialization_internal.h"

#include "core/character.h"
#include "core/realm.h"
#include "core/culture.h"
#include "core/religion.h"
#include "core/titulus.h"
#include "lua_initialization.h"
#include "magic_enum_header.h"
#include "stats_table.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_character(sol::state_view lua) {
//         auto t = lua.create_table_with(sol::meta_function::index, [] (sol::table t, sol::object o) {
//           
//         });
        
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        auto character_type = core.new_usertype<core::character>("character", sol::no_constructor,
          // мы не можем вернуть массив потому что используем юнион
//           "base_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->stats); }),
//           "current_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->current_stats); }),
//           "base_hero_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->hero_stats); }),
//           "current_hero_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->current_hero_stats); }),
          "get_base_stat", [] (const core::character* self, const sol::object &obj) -> float {
            if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad character stat index type");
            
            if (obj.get_type() == sol::type::number) {
              const size_t stat_index = obj.as<size_t>();
              const size_t final_index = FROM_LUA_INDEX(stat_index);
              if (final_index > core::character_stats::count) throw std::runtime_error("Bad character stat index " + std::to_string(stat_index));
              return self->base_stat(final_index);
            }
            
            const std::string_view str = obj.as<std::string_view>();
            // это наверное долго
            //const auto val = magic_enum::enum_cast<core::character_stats::values>(str);
            //if (!val.has_value()) throw std::runtime_error("Bad character stat index " + std::string(str));
            //const uint32_t index = val.value();
            const auto itr = character_stats_map.find(str);
            if (itr == character_stats_map.end()) throw std::runtime_error("Bad character stat index " + std::string(str));
            const uint32_t index = itr->second;
            return self->base_stat(index);
          },
          "get_current_stat", [] (const core::character* self, const sol::object &obj) -> float {
            if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad character stat index type");
            
            if (obj.get_type() == sol::type::number) {
              const size_t stat_index = obj.as<size_t>();
              const size_t final_index = FROM_LUA_INDEX(stat_index);
              if (final_index > core::character_stats::count) throw std::runtime_error("Bad character stat index " + std::to_string(stat_index));
              return self->stat(final_index);
            }
            
            const std::string_view str = obj.as<std::string_view>();
            const auto itr = character_stats_map.find(str);
            if (itr == character_stats_map.end()) throw std::runtime_error("Bad character stat index " + std::string(str));
            const uint32_t index = itr->second;
            return self->stat(index);
          },
          "get_base_hero_stat", [] (const core::character* self, const sol::object &obj) -> float {
            if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad character stat index type");
            
            if (obj.get_type() == sol::type::number) {
              const size_t stat_index = obj.as<size_t>();
              const size_t final_index = FROM_LUA_INDEX(stat_index);
              if (final_index > core::character_stats::count) throw std::runtime_error("Bad character stat index " + std::to_string(stat_index));
              return self->base_hero_stat(final_index);
            }
            
            const std::string_view str = obj.as<std::string_view>();
            const auto itr = hero_stats_map.find(str);
            if (itr == hero_stats_map.end()) throw std::runtime_error("Bad character stat index " + std::string(str));
            const uint32_t index = itr->second;
            return self->base_hero_stat(index);
          },
          "get_current_hero_stat", [] (const core::character* self, const sol::object &obj) -> float {
            if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad character stat index type");
            
            if (obj.get_type() == sol::type::number) {
              const size_t stat_index = obj.as<size_t>();
              const size_t final_index = FROM_LUA_INDEX(stat_index);
              if (final_index > core::character_stats::count) throw std::runtime_error("Bad character stat index " + std::to_string(stat_index));
              return self->hero_stat(final_index);
            }
            
            const std::string_view str = obj.as<std::string_view>();
            const auto itr = hero_stats_map.find(str);
            if (itr == hero_stats_map.end()) throw std::runtime_error("Bad character stat index " + std::string(str));
            const uint32_t index = itr->second;
            return self->hero_stat(index);
          },
          "name_number", sol::readonly(&core::character::name_number),
          "born_day", sol::readonly(&core::character::born_day),
          "death_day", sol::readonly(&core::character::death_day),
          "name_table_id", sol::readonly(&core::character::name_table_id),
          "nick_table_id", sol::readonly(&core::character::nickname_table_id),
          "name_index", sol::readonly(&core::character::name_index),
          "nick_index", sol::readonly(&core::character::nickname_index),
          "suzerain", sol::readonly(&core::character::suzerain),
          "imprisoner", sol::readonly(&core::character::imprisoner),
          "next_prisoner", sol::readonly(&core::character::next_prisoner),
          "prev_prisoner", sol::readonly(&core::character::prev_prisoner),
          "next_courtier", sol::readonly(&core::character::next_courtier),
          "prev_courtier", sol::readonly(&core::character::prev_courtier),
          "family", sol::readonly(&core::character::family),
          "relations", sol::readonly(&core::character::relations),
          "culture", sol::readonly(&core::character::culture),
          "religion", sol::readonly(&core::character::religion),
          "hidden_religion", sol::readonly(&core::character::hidden_religion),
          //"factions", sol::readonly_property([] (const core::character* self) { return std::ref(self->factions); }),
          "realm", sol::readonly_property([] (const core::character* self) { return self->realms[core::character::self]; }),
//           "traits", sol::readonly(&core::character::traits),
//           "modificators", sol::readonly(&core::character::modificators),
//           "events", sol::readonly(&core::character::events),
//           "flags", sol::readonly(&core::character::flags),
          "is_independent", &core::character::is_independent,
          "is_prisoner", &core::character::is_prisoner,
          "is_married", &core::character::is_married,
          "is_male", &core::character::is_male,
          "is_hero", &core::character::is_hero,
          "is_player", &core::character::is_player,
          "is_dead", &core::character::is_dead,
          "has_dynasty", &core::character::has_dynasty,
          "is_ai_playable", &core::character::is_ai_playable,
          "get_main_title", &core::character::get_main_title,
          "get_bit", &core::character::get_bit,
          "has_flag", &core::character::has_flag,
          "has_trait", &core::character::has_trait,
          "has_modificator", &core::character::has_modificator,
          "has_event", &core::character::has_event,
          "traits_container_size", sol::var(core::character::traits_container_size),
          "modificators_container_size", sol::var(core::character::modificators_container_size),
          "events_container_size", sol::var(core::character::events_container_size),
          "flags_container_size", sol::var(core::character::flags_container_size),
          "stats_count", sol::var(core::character_stats::count),
          "hero_stats_count", sol::var(core::hero_stats::count)
        );

        sol::table character_type_table = core["character"];

        auto family = character_type_table.new_usertype<struct core::character::family>("character_family",
          "real_parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->real_parents); }),
          "parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->parents); }),
          "children", sol::readonly(&core::character::family::children),
          "next_sibling", sol::readonly(&core::character::family::next_sibling),
          "prev_sibling", sol::readonly(&core::character::family::prev_sibling),
          "consort", sol::readonly(&core::character::family::consort),
          "previous_consorts", sol::readonly(&core::character::family::previous_consorts),
          "owner", sol::readonly(&core::character::family::owner),
          "concubines", sol::readonly(&core::character::family::concubines),
          "blood_dynasty", sol::readonly(&core::character::family::blood_dynasty),
          "dynasty", sol::readonly(&core::character::family::dynasty)
        );

        auto relations = character_type_table.new_usertype<struct core::character::relations>("character_relations",
          "friends", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->friends); }),
          "rivals", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->rivals); }),
          "lovers", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->lovers); })
        );
      }
    }
  }
}
