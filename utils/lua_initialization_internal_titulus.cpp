#include "lua_initialization_internal.h"

#include "core/titulus.h"
#include "core/realm.h"
#include "core/province.h"
#include "core/city.h"
#include "core/event.h"
#include "lua_initialization.h"
#include "magic_enum.hpp"
#include "lua_container_iterators.h"
#include "lua_initialization_handle_types.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_titulus(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        sol::usertype<core::titulus> title_type = core.new_usertype<core::titulus>(
          "titulus", sol::no_constructor,
          "id", sol::readonly(&core::titulus::id),
          "type", sol::readonly_property([] (const core::titulus* self) { return TO_LUA_INDEX(static_cast<size_t>(self->type())); }),
          "parent", sol::readonly(&core::titulus::parent),
          "owner", sol::readonly_property([] (const core::titulus* self) { return lua_handle_realm(self->owner); }),
          "name_id", sol::readonly(&core::titulus::name_id),
          "desc_id", sol::readonly(&core::titulus::description_id),
          "adj_id", sol::readonly(&core::titulus::adjective_id),
          "has_flag", [] (const core::titulus* self, const std::string_view &flag) { return self->has_flag(flag); },
          "is_formal", &core::titulus::is_formal,
          "children", [] (const core::titulus* self) {
            auto child = self->children;
            auto next_child = self->children;
            return [child, next_child] () mutable -> const core::titulus* {
              const auto final_child = next_child;
              next_child = ring::list_next<list_type::sibling_titles>(next_child, child);
              return final_child;
            };
          },
          "flags", &utils::flags_iterator<core::titulus>,
          "events", &utils::events_iterator<core::titulus>
        );
        
        //core["titulus_type"] = t;
        core.new_enum("titulus_type",
          "city",     TO_LUA_INDEX(static_cast<size_t>(core::titulus::type::city)),
          "baron",    TO_LUA_INDEX(static_cast<size_t>(core::titulus::type::baron)),
          "duke",     TO_LUA_INDEX(static_cast<size_t>(core::titulus::type::duke)),
          "king",     TO_LUA_INDEX(static_cast<size_t>(core::titulus::type::king)),
          "imperial", TO_LUA_INDEX(static_cast<size_t>(core::titulus::type::imperial))
        );

        core.set_function("each_child_title", [] (const core::titulus* self, const sol::function &func) {
          if (self == nullptr) throw std::runtime_error("Invalid input title");
          if (!func.valid()) throw std::runtime_error("Invalid input function");
                          
          auto next_child = self->children;
          while (next_child != nullptr) {
            const auto ret = func(next_child);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
            
            next_child = ring::list_next<list_type::sibling_titles>(next_child, self->children);
          }
        });
      }
    }
  }
}
