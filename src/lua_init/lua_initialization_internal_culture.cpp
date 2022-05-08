#include "lua_initialization_internal.h"

#include "core/culture.h"
#include "core/event.h"
#include "core/realm_mechanics_arrays.h"
#include "utils/lua_container_iterators.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    namespace internal {
      static bool get_feature(const core::culture* c, const sol::object &obj) {
        if (!obj.is<size_t>() && obj.get_type() == sol::type::string) throw std::runtime_error("Bad input for culture::get_feature");
        
        if (obj.is<size_t>()) {
          const size_t val = FROM_LUA_INDEX(obj.as<size_t>());
          if (val >= core::culture_mechanics::count) throw std::runtime_error("Invalid culture feature index " + std::to_string(val));
          return c->get_mechanic(val);
        }
        
        const auto &str = obj.as<std::string_view>();
        const auto itr = core::culture_mechanics::map.find(str);
        if (itr == core::culture_mechanics::map.end()) throw std::runtime_error("Could not find culture feature " + std::string(str));
        const size_t index = itr->second;
        return c->get_mechanic(index);
      }
      
      void setup_lua_culture(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::culture_group>(
          "culture_group", sol::no_constructor,
          "id", sol::readonly(&core::culture_group::id),
          "name_id", sol::readonly(&core::culture_group::name_id),
          "description_id", sol::readonly(&core::culture_group::description_id)
        );
        
        core.new_usertype<core::culture>(
          "culture", sol::no_constructor,
          "id", sol::readonly(&core::culture::id),
          "name_id", sol::readonly(&core::culture::name_id),
          "description_id", sol::readonly(&core::culture::description_id),
          "names_table", sol::readonly(&core::culture::names_table_id),
          "patronim_table", sol::readonly(&core::culture::patronims_table_id),
          "additional_table", sol::readonly(&core::culture::additional_table_id),
          "group", sol::readonly(&core::culture::group),
          "parent", sol::readonly(&core::culture::parent),
          "children", [] (const core::culture* self) {
            auto child = self->children;
            auto next_child = self->children;
            return [child, next_child] () mutable {
              auto final_child = next_child;
              next_child = ring::list_next<list_type::sibling_cultures>(next_child, child);
              return final_child;
            };
          },
          "grandparent_name_chance", sol::readonly(&core::culture::grandparent_name_chance),
          "icon", sol::readonly_property([] (const core::culture* self) { return self->image.container; }), // лучше придумать какой нибудь формат для картинок
          "color", sol::readonly_property([] (const core::culture* self) { return self->color.container; }),
          "bonuses", [] (const core::culture* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::stat_modifier* {
              if (counter >= self->bonuses.size() || self->bonuses[counter].invalid()) return nullptr;
              const auto ptr = &self->bonuses[counter];
              ++counter;
              return ptr;
            };
          },
          // как то так можно пробежать все флаги и эвенты, вроде итератор мы не теряем
          // естественно нужно отключить возможность добавлять какой то флаг во время интерфейса
          "flags", &utils::flags_iterator<core::culture>,
          "events", &utils::events_iterator<core::culture>,
          "get_feature", &get_feature,
          "features_start", sol::var(1),
          "features_count", sol::var(core::culture_mechanics::count)
        );
        
        core.set_function("each_child_culture", [] (const core::culture* culture, const sol::function &func) {
          if (!func.valid()) throw std::runtime_error("Invalid function input");
          if (culture == nullptr) throw std::runtime_error("Invalid culture input");
          
          auto child = culture->children;
          auto next_child = culture->children;
          while (next_child != nullptr) {
            const auto ret = func(next_child);
            CHECK_ERROR_THROW(ret);
            
            if (ret.get_type() == sol::type::boolean) {
              if (const bool r = ret; r) return;
            }
            
            next_child = ring::list_next<list_type::sibling_cultures>(next_child, child);
          }
        });
      }
    }
  }
}
