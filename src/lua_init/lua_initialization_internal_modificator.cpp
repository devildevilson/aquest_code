#include "lua_initialization_internal.h"

#include "core/modificator.h"
#include "core/stat_modifier.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_modificator(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::modificator>(
          "modificator", sol::no_constructor,
          "id", &core::modificator::id,
          "name_id", &core::modificator::name_id,
          "description_id", &core::modificator::description_id,
          "icon", sol::readonly_property([] (const core::modificator* self) { return self->icon.container; }),
          "attribute", [] (const core::modificator* self, const sol::object &obj) {
            if (!obj.is<size_t>() && obj.get_type() != sol::type::string) throw std::runtime_error("Invalid input for modificator attribute");
            
            if (obj.is<size_t>()) {
              const size_t val = FROM_LUA_INDEX(obj.as<size_t>());
              if (val >= core::modificator_attributes::count) throw std::runtime_error("Bad modificator attribute index " + std::to_string(obj.as<size_t>()));
              return self->get_attrib(val);
            }
            
            const auto str = obj.as<std::string_view>();
            const auto itr = core::modificator_attributes::map.find(str);
            if (itr == core::modificator_attributes::map.end()) throw std::runtime_error("Bad modificator attribute id " + std::string(str));
            return self->get_attrib(itr->second);
          },
          "bonuses", [] (const core::modificator* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::stat_modifier* {
              if (counter >= self->bonuses.size() || self->bonuses[counter].invalid()) return nullptr;
              const auto bonus = &self->bonuses[counter];
              ++counter;
              return bonus;
            };
          },
          "opinion_modificators", [] (const core::modificator* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::opinion_modifier* {
              if (counter >= self->opinion_mods.size() || self->opinion_mods[counter].invalid()) return nullptr;
              const auto op = &self->opinion_mods[counter];
              ++counter;
              return op;
            };
          },
          "attributes_start", sol::var(1),
          "attributes_count", sol::var(core::modificator_attributes::count)
        );
      }
    }
  }
}
