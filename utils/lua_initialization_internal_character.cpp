#include "lua_initialization_internal.h"

#include "core/structures_header.h"
#include "core/stats_table.h"
#include "reserved_lua_table_names.h"
#include "magic_enum_header.h"
#include "lua_container_iterators.h"
#include "lua_initialization_handle_types.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace utils {
    namespace internal {
      float get_base_stat(const core::character* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad character stat index type");
            
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index >= core::offsets::character_stats && final_index < core::offsets::character_stats + core::character_stats::count) {
            const size_t remove_offset = final_index - core::offsets::character_stats;
            return self->stats.get(remove_offset);
          }
          
          if (final_index >= core::offsets::hero_stats && final_index < core::offsets::hero_stats + core::hero_stats::count) {
            const size_t remove_offset = final_index - core::offsets::hero_stats;
            return self->hero_stats.get(remove_offset);
          }
          
          throw std::runtime_error("Bad character stat index " + std::to_string(stat_index));
        }
        
        const std::string_view str = obj.as<std::string_view>();
        if (const auto itr = core::character_stats::map.find(str); itr != core::character_stats::map.end()) {
          const uint32_t index = itr->second;
          return self->stats.get(index);
        }
        
        if (const auto itr = core::hero_stats::map.find(str); itr != core::hero_stats::map.end()) {
          const uint32_t index = itr->second;
          return self->hero_stats.get(index);
        }
        
        throw std::runtime_error("Bad character stat id " + std::string(str));
        return 0.0f;
      }
      
      float get_current_stat(const core::character* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad character stat index type");
            
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index >= core::offsets::character_stats && final_index < core::offsets::character_stats + core::character_stats::count) {
            const size_t remove_offset = final_index - core::offsets::character_stats;
            return self->current_stats.get(remove_offset);
          }
          
          if (final_index >= core::offsets::hero_stats && final_index < core::offsets::hero_stats + core::hero_stats::count) {
            const size_t remove_offset = final_index - core::offsets::hero_stats;
            return self->current_hero_stats.get(remove_offset);
          }
          
          throw std::runtime_error("Bad character stat index " + std::to_string(stat_index));
        }
        
        const std::string_view str = obj.as<std::string_view>();
        if (const auto itr = core::character_stats::map.find(str); itr != core::character_stats::map.end()) {
          const uint32_t index = itr->second;
          return self->current_stats.get(index);
        }
        
        if (const auto itr = core::hero_stats::map.find(str); itr != core::hero_stats::map.end()) {
          const uint32_t index = itr->second;
          return self->current_hero_stats.get(index);
        }
        
        throw std::runtime_error("Bad character stat id " + std::string(str));
        return 0.0f;
      }
      
      float get_resource(const core::character* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad character stat index type");
                                                                 
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::character_resources || final_index >= core::offsets::character_resources + core::character_resources::count) 
            throw std::runtime_error("Bad character stat index " + std::to_string(stat_index));
          return self->resources.get(final_index);
        }
        
        const std::string_view str = obj.as<std::string_view>();
        const auto itr = core::character_resources::map.find(str);
        if (itr == core::character_resources::map.end()) throw std::runtime_error("Bad character stat id " + std::string(str));
        const uint32_t index = itr->second;
        return self->resources.get(index);
      }
      
      auto children(const core::character* self) -> std::function<const core::character*()> {
        auto next_ch = self->family.children;
        auto first_child = self->family.children;
        if (self->is_male()) {
          return [first_child, next_ch] () mutable {
            auto final_ch = next_ch;
            next_ch = ring::list_next<list_type::father_line_siblings>(next_ch, first_child);
            return final_ch;
          };
        }
        
        return [first_child, next_ch] () mutable {
          auto final_ch = next_ch;
          next_ch = ring::list_next<list_type::mother_line_siblings>(next_ch, first_child);
          return final_ch;
        };
      }
      
      auto concubines(const core::character* self) {
        auto next_ch = self->family.concubines;
        auto first_child = self->family.concubines;
        return [first_child, next_ch] () mutable {
          auto final_ch = next_ch;
          next_ch = ring::list_next<list_type::concubines>(next_ch, first_child);
          return final_ch;
        };
      }
      
      void setup_lua_character(sol::state_view lua) {
//         auto t = lua.create_table_with(sol::meta_function::index, [] (sol::table t, sol::object o) {
//           
//         });
        
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        auto abc = core.new_usertype<utils::modificators_container::modificator_data>(
          "modificator_data", sol::no_constructor,
          "turns_count", &utils::modificators_container::modificator_data::turns_count,
          "bonuses", [] (const utils::modificators_container::modificator_data* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::stat_modifier* {
              if (counter >= self->bonuses.size() || self->bonuses[counter].invalid()) return nullptr;
              const auto ptr = &self->bonuses[counter];
              ++counter;
              return ptr;
            };
          },
          "opinion_modificators", [] (const utils::modificators_container::modificator_data* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::opinion_modifier* {
              if (counter >= self->opinion_mods.size() || self->opinion_mods[counter].invalid()) return nullptr;
              const auto ptr = &self->opinion_mods[counter];
              ++counter;
              return ptr;
            };
          }
        );
        
        auto character_type = core.new_usertype<core::character>(
          "character", sol::no_constructor,
          "name_number", sol::readonly(&core::character::name_number),
          "born_day", sol::readonly(&core::character::born_day),
          "death_day", sol::readonly(&core::character::death_day),
          "name_table_id", sol::readonly(&core::character::name_table_id),
          "nick_table_id", sol::readonly(&core::character::nickname_table_id),
          "name_index", sol::readonly(&core::character::name_index),
          "nick_index", sol::readonly(&core::character::nickname_index),
          "culture", sol::readonly(&core::character::culture),
          "religion", sol::readonly(&core::character::religion),
          "hidden_religion", sol::readonly(&core::character::hidden_religion),
          "family", sol::readonly(&core::character::family),
          "relations", sol::readonly(&core::character::relations),
                                                                 
          "suzerain", sol::readonly_property([] (const core::character* c) { return lua_handle_realm(c->suzerain); }),
          "imprisoner", sol::readonly_property([] (const core::character* c) { return lua_handle_realm(c->imprisoner); }),
          "realm", sol::readonly_property([] (const core::character* self) { return lua_handle_realm(self->self); }),
                                                                 
          "stats_start", sol::var(TO_LUA_INDEX(core::offsets::character_stats)),
          "hero_stats_start", sol::var(TO_LUA_INDEX(core::offsets::hero_stats)),
          "resources_start", sol::var(TO_LUA_INDEX(core::offsets::character_resources)),
          "stats_end", sol::var(core::offsets::character_stats + core::character_stats::count),
          "hero_stats_end", sol::var(core::offsets::hero_stats + core::hero_stats::count),
          "resources_end", sol::var(core::offsets::character_resources + core::character_resources::count)
        );
        
        character_type.set_function("is_independent", &core::character::is_independent);
        character_type.set_function("is_prisoner", &core::character::is_prisoner);
        character_type.set_function("is_married", &core::character::is_married);
        character_type.set_function("is_male", &core::character::is_male);
        character_type.set_function("is_hero", [] (const core::character* c) { return core::character::is_hero(c); });
        character_type.set_function("is_player", &core::character::is_player);
        character_type.set_function("is_dead", &core::character::is_dead);
        character_type.set_function("has_dynasty", &core::character::has_dynasty);
        character_type.set_function("is_ai_playable", &core::character::is_ai_playable);
        character_type.set_function("get_main_title", &core::character::get_main_title);
        character_type.set_function("get_base_stat", &get_base_stat);
        character_type.set_function("get_stat", &get_current_stat);
        character_type.set_function("get_resource", &get_resource);
        character_type.set_function("children", &children);
        character_type.set_function("concubines", &concubines);
        character_type.set_function("has_flag", &core::character::has_flag);
        character_type.set_function("has_trait", &core::character::has_trait);
        character_type.set_function("has_modificator", &core::character::has_modificator);
        character_type.set_function("has_event", &core::character::has_event);
        character_type.set_function("flags", &utils::flags_iterator<core::character>);
        character_type.set_function("events", &utils::events_iterator<core::character>);
        character_type.set_function("modificators", &utils::modificators_iterator<core::character>);
        character_type.set_function("traits", &utils::traits_iterator<core::character>);

        sol::table character_type_table = core["character"];

        auto family = character_type_table.new_usertype<struct core::character::family>("character_family",
          "real_parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->real_parents); }),
          "parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->parents); }),
          "consort", sol::readonly(&core::character::family::consort),
//           "previous_consorts", sol::readonly(&core::character::family::previous_consorts),
          "owner", sol::readonly(&core::character::family::owner),
          "blood_dynasty", sol::readonly(&core::character::family::blood_dynasty),
          "dynasty", sol::readonly(&core::character::family::dynasty)
        );

        auto relations = character_type_table.new_usertype<struct core::character::relations>(
          "character_relations", sol::no_constructor,
          "acquaintances", sol::readonly_property([] (const struct core::character::relations* self) { 
            size_t counter = 0;
            return [self, counter] () mutable -> std::tuple<core::character*, int32_t, int32_t> {
              while (self->acquaintances[counter].first == nullptr && counter < self->acquaintances.size()) { ++counter; }
              if (counter < self->acquaintances.size()) {
                return std::make_tuple(
                  self->acquaintances[counter].first,
                  self->acquaintances[counter].second.friendship,
                  self->acquaintances[counter].second.love
                );
              }
              
              return std::make_tuple(nullptr, 0, 0);
            };
          }),
          "acquaintances_max_size", sol::var(core::character::relations::max_game_acquaintance)
        );
        
        core.set_function("each_child", [] (const core::character* character, const sol::function &func) {
          if (character == nullptr) throw std::runtime_error("Invalid input character");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
                          
          auto child = character->family.children;
          auto next_child = character->family.children;
          while (next_child != nullptr) {
            const auto &ret = func(next_child);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
            
            if (character->is_male()) next_child = ring::list_next<list_type::father_line_siblings>(next_child, child);
            else next_child = ring::list_next<list_type::mother_line_siblings>(next_child, child);
          }
        });
        
        core.set_function("each_concubine", [] (const core::character* character, const sol::function &func) {
          if (character == nullptr) throw std::runtime_error("Invalid input character");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
                          
          auto concubine = character->family.concubines;
          auto next_concubine = character->family.concubines;
          while (next_concubine != nullptr) {
            const auto &ret = func(next_concubine);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
            
            next_concubine = ring::list_next<list_type::concubines>(next_concubine, concubine);
          }
        });
        
        core.set_function("each_trait", [] (const core::character* character, const sol::function &func) {
          if (character == nullptr) throw std::runtime_error("Invalid input character");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
          
          const auto &traits = character->traits;
          for (const auto trait : traits) {
            const auto &ret = func(trait);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
        
        core.set_function("each_modificator", [] (const sol::object &obj, const sol::function &func) {
          if (!obj.valid()) throw std::runtime_error("Invalid input object");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
          
          const decltype(utils::modificators_container::modificators)* container = nullptr;
          if (obj.is<const core::character*>()) {
            const auto ptr = obj.as<const core::character*>();
            container = &ptr->modificators;
          } else if (obj.is<const core::realm*>()) {
            const auto ptr = obj.as<const core::realm*>();
            container = &ptr->modificators;
          } else if (obj.is<const core::province*>()) {
            const auto ptr = obj.as<const core::province*>();
            container = &ptr->modificators;
          } else if (obj.is<const core::army*>()) {
            const auto ptr = obj.as<const core::army*>();
            container = &ptr->modificators;
          } else if (obj.is<const core::city*>()) {
            const auto ptr = obj.as<const core::city*>();
            container = &ptr->modificators;
          } else if (obj.is<const core::hero_troop*>()) {
            const auto ptr = obj.as<const core::hero_troop*>();
            container = &ptr->modificators;
          }
          
          assert(container != nullptr);
          const auto &modificators = *container;
          for (const auto &pair : modificators) {
            const auto &ret = func(pair.first, &pair.second);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
        
        core.set_function("each_flag", [] (const sol::object &obj, const sol::function &func) {
          if (!obj.valid()) throw std::runtime_error("Invalid input object");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
          
          const decltype(utils::flags_container::flags)* container = nullptr;
          if (obj.is<const core::character*>()) {
            const auto ptr = obj.as<const core::character*>();
            container = &ptr->flags;
          } else if (obj.is<const core::realm*>()) {
            const auto ptr = obj.as<const core::realm*>();
            container = &ptr->flags;
          } else if (obj.is<const core::province*>()) {
            const auto ptr = obj.as<const core::province*>();
            container = &ptr->flags;
          } else if (obj.is<const core::army*>()) {
            const auto ptr = obj.as<const core::army*>();
            container = &ptr->flags;
          } else if (obj.is<const core::city*>()) {
            const auto ptr = obj.as<const core::city*>();
            container = &ptr->flags;
          } else if (obj.is<const core::culture*>()) {
            const auto ptr = obj.as<const core::city*>();
            container = &ptr->flags;
          } else if (obj.is<const core::religion*>()) {
            const auto ptr = obj.as<const core::city*>();
            container = &ptr->flags;
          } else if (obj.is<const core::war*>()) {
            const auto ptr = obj.as<const core::war*>();
            container = &ptr->flags;
          } else if (obj.is<const core::hero_troop*>()) {
            const auto ptr = obj.as<const core::hero_troop*>();
            container = &ptr->flags;
          } else if (obj.is<const core::titulus*>()) {
            const auto ptr = obj.as<const core::titulus*>();
            container = &ptr->flags;
          }
          
          assert(container != nullptr);
          const auto &flags = *container;
          for (const auto &pair : flags) {
            const auto &ret = func(pair.first, &pair.second);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
        
        core.set_function("each_event", [] (const sol::object &obj, const sol::function &func) {
          if (!obj.valid()) throw std::runtime_error("Invalid input object");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
          
          const decltype(utils::events_container::events)* container = nullptr;
          if (obj.is<const core::character*>()) {
            const auto ptr = obj.as<const core::character*>();
            container = &ptr->events;
          } else if (obj.is<const core::realm*>()) {
            const auto ptr = obj.as<const core::realm*>();
            container = &ptr->events;
          } else if (obj.is<const core::province*>()) {
            const auto ptr = obj.as<const core::province*>();
            container = &ptr->events;
          } else if (obj.is<const core::army*>()) {
            const auto ptr = obj.as<const core::army*>();
            container = &ptr->events;
          } else if (obj.is<const core::city*>()) {
            const auto ptr = obj.as<const core::city*>();
            container = &ptr->events;
          } else if (obj.is<const core::culture*>()) {
            const auto ptr = obj.as<const core::city*>();
            container = &ptr->events;
          } else if (obj.is<const core::religion*>()) {
            const auto ptr = obj.as<const core::city*>();
            container = &ptr->events;
          } else if (obj.is<const core::war*>()) {
            const auto ptr = obj.as<const core::war*>();
            container = &ptr->events;
          } else if (obj.is<const core::hero_troop*>()) {
            const auto ptr = obj.as<const core::hero_troop*>();
            container = &ptr->events;
          } else if (obj.is<const core::titulus*>()) {
            const auto ptr = obj.as<const core::titulus*>();
            container = &ptr->events;
          }
          
          assert(container != nullptr);
          const auto &events = *container;
          for (const auto &pair : events) {
            const auto &ret = func(pair.first, pair.second.mtth);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
        
        core.set_function("each_acquaintance", [] (const core::character* character, const sol::function &func) {
          if (character == nullptr) throw std::runtime_error("Invalid input character");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
                          
          for (const auto &pair : character->relations.acquaintances) {
            if (pair.first == nullptr) continue;
            const auto &ret = func(pair.first, pair.second.friendship, pair.second.love);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
      }
    }
  }
}
