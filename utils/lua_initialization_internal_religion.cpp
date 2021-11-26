#include "lua_initialization_internal.h"

#include "core/religion.h"
#include "core/event.h"
#include "core/realm_mechanics_arrays.h"
#include "lua_container_iterators.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_religion(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::religion_group>(
          "religion_group", sol::no_constructor,
          "id", sol::readonly(&core::religion_group::id),
          "name_id", sol::readonly(&core::religion_group::name_id),
          "description_id", sol::readonly(&core::religion_group::description_id)
        );
        
        core.new_usertype<core::religion>(
          "religion", sol::no_constructor,
          "id", sol::readonly(&core::religion::id),
          "name_id", sol::readonly(&core::religion::name_id),
          "description_id", sol::readonly(&core::religion::description_id),
          "group", sol::readonly(&core::religion::group),
          "parent", sol::readonly(&core::religion::parent),
          "children", [] (const core::religion* rel) {
            auto child = rel->children;
            auto next_child = rel->children;
            return [child, next_child] () mutable {
              auto final_child = next_child;
              next_child = ring::list_next<list_type::faiths>(next_child, child);
              return final_child;
            };
          },
          "reformed", sol::readonly(&core::religion::reformed),
          "aggression", sol::readonly(&core::religion::aggression),
          "crusade_name_id", sol::readonly(&core::religion::crusade_name_id),
          "holy_order_names_table_id", sol::readonly(&core::religion::holy_order_names_table_id),
          "scripture_name_id", sol::readonly(&core::religion::scripture_name_id),
          "good_gods_table_id", sol::readonly(&core::religion::good_gods_table_id),
          "evil_gods_table_id", sol::readonly(&core::religion::evil_gods_table_id),
          "high_god_name_id", sol::readonly(&core::religion::high_god_name_id),
          "piety_name_id", sol::readonly(&core::religion::piety_name_id),
          "priest_title_name_id", sol::readonly(&core::religion::priest_title_name_id),
          "reserved_male_names_table_id", sol::readonly(&core::religion::reserved_male_names_table_id),
          "reserved_female_names_table_id", sol::readonly(&core::religion::reserved_female_names_table_id),
//           "type_index", sol::readonly(&core::religion::type_index),
          "opinion_stat_index", sol::readonly(&core::religion::opinion_stat_index),
          "icon", sol::readonly_property([] (const core::religion* rel) { return rel->image.container; }),
          "color", sol::readonly_property([] (const core::religion* rel) { return rel->color.container; }),
          "flags", &utils::flags_iterator<core::religion>,
          "events", &utils::events_iterator<core::religion>,
          "bonuses", [] (const core::religion* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::stat_modifier* {
              if (counter >= self->bonuses.size() || self->bonuses[counter].invalid()) return nullptr;
              const auto ptr = &self->bonuses[counter];
              ++counter;
              return ptr;
            };
          },
          "flags", [] (const core::religion* self) {
            auto itr = self->flags.begin();
            return [self, itr] (sol::this_state s) mutable {
              if (itr == self->flags.end()) return sol::object(sol::nil);
              const auto current = itr;
              ++itr;
              return sol::make_object(s, current->second);
            };
          },
          "events", [] (const core::religion* self) {
            auto itr = self->events.begin();
            return [self, itr] () mutable -> std::tuple<const core::event*, size_t> {
              if (itr == self->events.end()) return std::make_tuple(nullptr, 0);
              const auto current = itr;
              ++itr;
              return std::make_tuple(current->first, current->second.mtth);
            };
          },
          // особенности религии?
          "get_feature", [] (const core::religion* self, const sol::object &obj) {
            if (!obj.is<size_t>() && obj.get_type() == sol::type::string) throw std::runtime_error("Bad input for religion.get_feature");
            
            if (obj.is<size_t>()) {
              const size_t val = FROM_LUA_INDEX(obj.as<size_t>());
              if (val >= core::religion_mechanics::count) throw std::runtime_error("Invalid religion feature index " + std::to_string(val));
              return self->get_mechanic(val);
            }
            
            const auto &str = obj.as<std::string_view>();
            const auto itr = core::religion_mechanics::map.find(str);
            if (itr == core::religion_mechanics::map.end()) throw std::runtime_error("Could not find religion feature " + std::string(str));
            const size_t index = itr->second;
            return self->get_mechanic(index);
          },
          "features_start", sol::var(1),
          "features_count", sol::var(core::religion_mechanics::count)
        );
        
        core.set_function("each_child_religion", [] (const core::religion* religion, const sol::function &func) {
          if (!func.valid()) throw std::runtime_error("Invalid function input");
          if (religion == nullptr) throw std::runtime_error("Invalid religion input");
          
          auto child = religion->children;
          auto next_child = religion->children;
          while (next_child != nullptr) {
            const auto ret = func(next_child);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
            
            next_child = ring::list_next<list_type::faiths>(next_child, child);
          }
        });
      }
    }
  }
}
