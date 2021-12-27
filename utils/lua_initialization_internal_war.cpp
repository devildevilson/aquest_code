#include "lua_initialization_internal.h"

#include "core/war.h"
#include "core/titulus.h"
#include "core/realm.h"
#include "core/character.h"
#include "core/event.h"
#include "core/casus_belli.h"
#include "lua_container_iterators.h"
#include "lua_initialization_handle_types.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_war(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::war>(
          "war", sol::no_constructor,
          "casus_belli", sol::readonly(&core::war::cb),
          "war_opener", sol::readonly(&core::war::war_opener),
          "target_character", sol::readonly(&core::war::target_character),
//           "opener_realm", sol::readonly_property([] (const core::war* self) { return lua_handle_realm(self->opener_realm); }),
//           "target_realm", sol::readonly_property([] (const core::war* self) { return lua_handle_realm(self->target_realm); }),
          "primary_attacker", sol::readonly_property([] (const core::war* self) { return self->war_opener; }),
          "primary_defender", sol::readonly_property([] (const core::war* self) { return self->target_character; }),
          "claimat", sol::readonly_property([] (const core::war* self) { return self->claimant; }),
          "target_titles", [] (const core::war* war) {
            size_t counter = 0;
            return [war, counter] () mutable -> const core::titulus* {
              if (counter >= war->target_titles.size()) return nullptr;
              auto t = war->target_titles[counter];
              ++counter;
              return t;
            };
          },
          "attackers", [] (const core::war* war) {
            size_t counter = 0;
            return [war, counter] () mutable -> core::character* { //lua_handle_realm
              if (counter >= war->attackers.size()) return nullptr;
              auto r = war->attackers[counter];
              ++counter;
              return r;
            };
          },
          "defenders", [] (const core::war* war) {
            size_t counter = 0;
            return [war, counter] () mutable -> core::character* { // lua_handle_realm
              if (counter >= war->defenders.size()) return nullptr;
              auto r = war->defenders[counter];
              ++counter;
              return r;
            };
          },
          "flags", &utils::flags_iterator<core::war>,
          "events", &utils::events_iterator<core::war>
        );
        
        core.set_function("each_target_title", [] (const core::war* war, const sol::function &func) {
          if (!func.valid()) throw std::runtime_error("Invalid input function");
          if (war == nullptr) throw std::runtime_error("Invalid input war object");
                          
          for (const auto t : war->target_titles) {
            const auto ret = func(t);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
        
        core.set_function("each_attacker", [] (const core::war* war, const sol::function &func) {
          if (!func.valid()) throw std::runtime_error("Invalid input function");
          if (war == nullptr) throw std::runtime_error("Invalid input war object");
                          
          for (const auto t : war->attackers) {
            const auto ret = func(t);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
        
        core.set_function("each_defender", [] (const core::war* war, const sol::function &func) {
          if (!func.valid()) throw std::runtime_error("Invalid input function");
          if (war == nullptr) throw std::runtime_error("Invalid input war object");
                          
          for (const auto t : war->defenders) {
            const auto ret = func(t);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
          }
        });
        
        core.new_usertype<lua_handle_war>(
          "war_handle", sol::no_constructor,
          "get", &lua_handle_war::get,
          "valid", &lua_handle_war::valid
        );
      }
    }
  }
}
