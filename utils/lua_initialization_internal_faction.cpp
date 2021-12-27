#include "lua_initialization_internal.h"

#include "core/realm.h"
#include "core/character.h"
#include "core/titulus.h"
#include "core/stats_table.h"
#include "core/realm_mechanics_arrays.h"
#include "core/modificator.h"
#include "core/religion.h"
#include "core/city.h"
#include "core/province.h"

#include "lua_initialization.h"
#include "magic_enum_header.h"
#include "lua_container_iterators.h"
#include "lua_initialization_handle_types.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    namespace internal {
      typedef bool (utils::handle<core::realm>::*compare_func) (const utils::handle<core::realm> &second) const;
      
      template <typename T>
      static sol::object make_object(sol::this_state s, T obj) {
        if constexpr (std::is_same_v<T, utils::handle<core::realm>>) return sol::make_object(s, lua_handle_realm(obj));
        else if constexpr (std::is_same_v<T, utils::handle<core::army>>) return sol::make_object(s, lua_handle_army(obj));
        else if constexpr (std::is_same_v<T, utils::handle<core::hero_troop>>) return sol::make_object(s, lua_handle_hero_troop(obj));
        else if constexpr (std::is_same_v<T, utils::handle<core::war>>) return sol::make_object(s, lua_handle_war(obj));
        else if constexpr (std::is_same_v<T, utils::handle<core::troop>>) return sol::make_object(s, lua_handle_troop(obj));
        else if constexpr (std::is_pointer_v<T>) return sol::make_object(s, obj);
        return sol::nil;
      }
      
      static float get_stat(const core::realm* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad realm stat index obj");
                                                        
        if (obj.get_type() == sol::type::number) {
          const size_t num = FROM_LUA_INDEX(obj.as<size_t>());
          if (num < core::offsets::realm_stats || num >= core::offsets::realm_stats + core::realm_stats::count) throw std::runtime_error("Bad realm stat index " + std::to_string(num));
          const size_t final_index = num - core::offsets::realm_stats;
          return self->current_stats.get(final_index);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::realm_stats::map.find(str);
        if (itr == core::realm_stats::map.end()) throw std::runtime_error("Bad realm stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->current_stats.get(final_index);
      }
      
      static float get_base_stat(const core::realm* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad realm stat index obj");
                                                        
        if (obj.get_type() == sol::type::number) {
          const size_t num = FROM_LUA_INDEX(obj.as<size_t>());
          if (num < core::offsets::realm_stats || num >= core::offsets::realm_stats + core::realm_stats::count) throw std::runtime_error("Bad realm stat index " + std::to_string(num));
          const size_t final_index = num - core::offsets::realm_stats;
          return self->stats.get(final_index);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::realm_stats::map.find(str);
        if (itr == core::realm_stats::map.end()) throw std::runtime_error("Bad realm stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->stats.get(final_index);
      }
      
      static float get_resource(const core::realm* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad realm resource index obj");
                                                        
        if (obj.get_type() == sol::type::number) {
          const size_t num = FROM_LUA_INDEX(obj.as<size_t>());
          if (num < core::offsets::realm_resources || num >= core::offsets::realm_resources + core::realm_resources::count) throw std::runtime_error("Bad realm resource index " + std::to_string(num));
          const size_t final_index = num - core::offsets::realm_resources;
          return self->resources.get(final_index);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::realm_resources::map.find(str);
        if (itr == core::realm_resources::map.end()) throw std::runtime_error("Bad realm resource id " + std::string(str));
        const size_t final_index = itr->second;
        return self->resources.get(final_index);
      }
      
      static bool has_right(const core::realm* self, const sol::object &obj) {
        if (!obj.is<size_t>() && obj.get_type() != sol::type::string) throw std::runtime_error("Bad realm right index obj");
                                                        
        if (obj.is<size_t>()) {
          const size_t num = FROM_LUA_INDEX(obj.as<size_t>());
          if (num >= core::power_rights::offset && num < core::power_rights::count) {
            return self->get_power_mechanic(num);
          }
          
          if (num >= core::state_rights::offset && num < core::state_rights::count) {
            return self->get_state_mechanic(num);
          }
          
          throw std::runtime_error("Bad realm right index " + std::to_string(num));
        }
        
        const auto str = obj.as<std::string_view>();
        if (const auto itr = core::power_rights::map.find(str); itr != core::power_rights::map.end()) {
          const size_t final_index = itr->second;
          return self->get_power_mechanic(final_index);
        }
        
        if (const auto itr = core::state_rights::map.find(str); itr != core::state_rights::map.end()) {
          const size_t final_index = itr->second;
          return self->get_state_mechanic(final_index);
        }
        
        throw std::runtime_error("Bad realm right id " + std::string(str));
        return false;
      }
      
      void setup_lua_realm(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        auto faction_type = core.new_usertype<core::realm>(
          "realm", sol::no_constructor,
          "leader", sol::readonly(&core::realm::leader),
          "heir", sol::readonly(&core::realm::heir),
//           "liege", sol::readonly(&core::realm::liege),
//           "state", sol::readonly(&core::realm::state),
//           "council", sol::readonly(&core::realm::council),
//           "tribunal", sol::readonly(&core::realm::tribunal),
//           "assembly", sol::readonly(&core::realm::assembly),
//           "clergy", sol::readonly(&core::realm::clergy),
          "liege", sol::readonly_property([] (const core::realm* r) { return lua_handle_realm(r->liege); }),
          "council", sol::readonly_property([] (const core::realm* r) { return lua_handle_realm(r->council); }),
          "tribunal", sol::readonly_property([] (const core::realm* r) { return lua_handle_realm(r->tribunal); }),
          "assembly", sol::readonly_property([] (const core::realm* r) { return lua_handle_realm(r->assembly); }),
          "clergy", sol::readonly_property([] (const core::realm* r) { return lua_handle_realm(r->clergy); }),
          "vassals", [] (const core::realm* self) {
            auto first_vassal = self->vassals != nullptr ? utils::handle<core::realm>(self->vassals, self->vassals->object_token) : utils::handle<core::realm>();
            auto next_vassal = self->vassals != nullptr ? utils::handle<core::realm>(self->vassals, self->vassals->object_token) : utils::handle<core::realm>();
            return [first_vassal, next_vassal] (sol::this_state s) mutable -> sol::object {
              // добавил проверки токенов, должно работать даже если объект разрушен
              if (!first_vassal.valid()) throw std::runtime_error("Invalid realm");
              if (!next_vassal.valid()) throw std::runtime_error("Invalid realm");
              auto final_vassal = next_vassal;
              auto vassal_ptr = ring::list_next<list_type::vassals>(next_vassal.get(), first_vassal.get());
              ASSERT(vassal_ptr->object_token != SIZE_MAX);
              next_vassal = vassal_ptr != nullptr ? utils::handle<core::realm>(vassal_ptr, vassal_ptr->object_token) : utils::handle<core::realm>();
              // по идее лучше вернуть здесь хэндл, как выглядит хэндл? указатель + токен, 
              // единственный метод get(), который может вернуть nullptr
              return final_vassal.valid() ? sol::make_object(s, lua_handle_realm(final_vassal)) : sol::object();
            };
          },
          "titles", [] (const core::realm* self) {
            auto first_title = self->titles;
            auto next_title = self->titles;
            return [first_title, next_title] () mutable {
              auto final_title = next_title;
              next_title = ring::list_next<list_type::titles>(next_title, first_title);
              return final_title;
            };
          },
          "main_title", sol::readonly(&core::realm::main_title),
          "courtiers", [] (const core::realm* self) {
            auto first_courtier = self->courtiers;
            auto next_courtier = self->courtiers;
            return [first_courtier, next_courtier] () mutable {
              auto final_courtier = next_courtier;
              next_courtier = ring::list_next<list_type::courtiers>(next_courtier, first_courtier);
              return final_courtier;
            };
          },
          "prisoners", [] (const core::realm* self) {
            auto first_prisoner = self->prisoners;
            auto next_prisoner = self->prisoners;
            return [first_prisoner, next_prisoner] () mutable {
              auto final_prisoner = next_prisoner;
              next_prisoner = ring::list_next<list_type::prisoners>(next_prisoner, first_prisoner);
              return final_prisoner;
            };
          },
          "get_stat", &get_stat,
          "get_base_stat", &get_base_stat,
          "get_resource", &get_resource,
          "has_right", &has_right,
//           "is_independent", &core::realm::is_independent,
//           "is_state", &core::realm::is_state,
//           "is_council", &core::realm::is_council,
//           "is_tribunal", &core::realm::is_tribunal,
//           "is_assembly", &core::realm::is_assembly,
//           "is_clergy", &core::realm::is_clergy,
//           "is_self", &core::realm::is_self,
          "flags", &utils::flags_iterator<core::realm>,
          "modificators", &utils::modificators_iterator<core::realm>,
          "stats_start", sol::var(TO_LUA_INDEX(core::offsets::realm_stats)),
          "power_rights_start", sol::var(TO_LUA_INDEX(core::power_rights::offset)),
          "state_rights_start", sol::var(TO_LUA_INDEX(core::state_rights::offset)),
          "resources_start", sol::var(TO_LUA_INDEX(core::offsets::realm_resources)),
          "stats_end", sol::var(core::offsets::realm_stats + core::realm_stats::count),
          "power_rights_end", sol::var(core::power_rights::offset + core::power_rights::count),
          "state_rights_end", sol::var(core::state_rights::offset + core::state_rights::count),
          "resources_end", sol::var(core::offsets::realm_resources + core::realm_resources::count)
        );
        
#define GET_SCOPE_COMMAND_FUNC(name, a, b, type) core.set_function("get_"#name, [] (sol::this_state s, const lua_handle_realm &self) { \
          auto obj = self.get()->get_##name(); \
          return make_object(s, obj); \
        }); \
        
        
        REALM_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) core.set_function(#name, &core::realm::name);
        REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

// #define CONDITION_ARG_COMMAND_FUNC(name, unused1, unused2, type) bool name(const type &data) const noexcept;
//         REALM_GET_BOOL_EXISTED_ARG_COMMANDS_LIST
// #undef CONDITION_ARG_COMMAND_FUNC
        
        core.set_function("each_title", [] (const core::realm* f, const sol::function &function) {
          if (f == nullptr) throw std::runtime_error("Invalid realm");
          if (!function.valid()) throw std::runtime_error("Invalid function");
                          
          auto title = f->titles;
          while (title != nullptr) {
            const auto ret = function(title);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
                          
            title = ring::list_next<list_type::titles>(title, f->titles);
          }
        });
        
        core.set_function("each_vassal", [] (const core::realm* f, const sol::function &function) {
          if (f == nullptr) throw std::runtime_error("Invalid realm");
          if (!function.valid()) throw std::runtime_error("Invalid function");
                          
          auto vassal = f->vassals;
          while (vassal != nullptr) {
            const auto ret = function(lua_handle_realm(vassal, vassal->object_token));
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
            
            vassal = ring::list_next<list_type::vassals>(vassal, f->vassals);
          }
        });
        
        core.set_function("each_courtier", [] (const core::realm* f, const sol::function &function) {
          if (f == nullptr) throw std::runtime_error("Invalid realm");
          if (!function.valid()) throw std::runtime_error("Invalid function");
                          
          auto courtier = f->courtiers;
          while (courtier != nullptr) {
            const auto ret = function(courtier);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
                          
            courtier = ring::list_next<list_type::courtiers>(courtier, f->courtiers);
          }
        });
        
        core.set_function("each_prisoner", [] (const lua_handle_realm f, const sol::function &function) {
          if (!f.valid()) throw std::runtime_error("Invalid realm");
          if (!function.valid()) throw std::runtime_error("Invalid function");
                          
          auto prisoner = f.get()->prisoners;
          while (prisoner != nullptr) {
            const auto ret = function(prisoner);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
                          
            prisoner = ring::list_next<list_type::prisoners>(prisoner, f.get()->prisoners);
          }
        });
        
//         compare_func f = &utils::handle<core::realm>::operator==;
//         core.new_usertype<utils::handle<core::realm>>(
//           "realm_handle", sol::no_constructor,
// //           "get", &utils::handle<core::realm>::get,
// //           "valid", &utils::handle<core::realm>::valid
//           sol::meta_function::equal_to, f
//         );
        
        auto handle = core.new_usertype<lua_handle_realm>(
          "realm_handle", sol::no_constructor,
          "get", &lua_handle_realm::get,
          "valid", &lua_handle_realm::valid
        );
        
//         handle.set_function("get", &lua_handle::get);
      }
    }
  }
}
