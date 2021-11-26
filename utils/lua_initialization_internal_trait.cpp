#include "lua_initialization_internal.h"

#include "core/trait.h"
#include "core/stats.h"
#include "core/structures_header.h"
#include "lua_initialization_handle_types.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_trait(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::stat_bonus>(
          "stat_bonus", sol::no_constructor,
          "type", &core::stat_bonus::type,
          "stat", &core::stat_bonus::stat,
          "raw_add", sol::readonly_property([] (const core::stat_bonus* self) { return self->bonus.raw_add; }),
          "raw_mul", sol::readonly_property([] (const core::stat_bonus* self) { return self->bonus.raw_mul; }),
          "fin_add", sol::readonly_property([] (const core::stat_bonus* self) { return self->bonus.fin_add; }),
          "fin_mul", sol::readonly_property([] (const core::stat_bonus* self) { return self->bonus.fin_mul; })
        );
        
        core.new_usertype<core::stat_modifier>(
          "stat_modifier", sol::no_constructor,
          "type", &core::stat_modifier::type,
          "stat", &core::stat_modifier::stat,
          "mod", &core::stat_modifier::mod
        );
        
        core.new_usertype<core::opinion_modifier>(
          "opinion_modifier", sol::no_constructor,
          "type", &core::opinion_modifier::type,
          //"type", &core::opinion_modifier::type,
          "target", sol::readonly_property([] (sol::this_state s, const core::opinion_modifier* self) {
            switch (self->type) {
              case core::opinion_modifiers::character_opinion:      return sol::make_object(s, self->character);
              case core::opinion_modifiers::realm_opinion:          return sol::make_object(s, lua_handle_realm(self->realm, self->token));
              case core::opinion_modifiers::culture_opinion:        return sol::make_object(s, self->culture);
              case core::opinion_modifiers::culture_group_opinion:  return sol::make_object(s, self->culture_group);
              case core::opinion_modifiers::religion_opinion:       return sol::make_object(s, self->religion);
              case core::opinion_modifiers::religion_group_opinion: return sol::make_object(s, self->religion_group);
              case core::opinion_modifiers::city_type_opinion:      return sol::make_object(s, self->city_type);
              case core::opinion_modifiers::holding_type_opinion:   return sol::make_object(s, self->holding_type);
              case core::opinion_modifiers::other_dynasty_opinion:  return sol::make_object(s, self->dynasty);
              default: break;
            }
            
            return sol::object(sol::nil);
          }),
          "mod", &core::opinion_modifier::mod
//           "raw_add", sol::readonly_property([] (const core::opinion_modifier* self) { return self->bonus.raw_add; }),
//           "raw_mul", sol::readonly_property([] (const core::opinion_modifier* self) { return self->bonus.raw_mul; }),
//           "fin_add", sol::readonly_property([] (const core::opinion_modifier* self) { return self->bonus.fin_add; }),
//           "fin_mul", sol::readonly_property([] (const core::opinion_modifier* self) { return self->bonus.fin_mul; })
        );
        
        core.new_usertype<core::trait>(
          "trait", sol::no_constructor,
          "id", &core::trait::id,
          "name_id", &core::trait::name_id,
          "description_id", &core::trait::description_id,
          "icon", sol::readonly_property([] (const core::trait* self) { return self->icon.container; }),
          "attribute", [] (const core::trait* self, const sol::object &obj) {
            if (!obj.is<size_t>() && obj.get_type() != sol::type::string) throw std::runtime_error("Invalid input for trait attribute");
            
            if (obj.is<size_t>()) {
              const size_t val = FROM_LUA_INDEX(obj.as<size_t>());
              if (val >= core::trait_attributes::count) throw std::runtime_error("Bad trait attribute index " + std::to_string(obj.as<size_t>()));
              return self->get_attrib(val);
            }
            
            const auto str = obj.as<std::string_view>();
            const auto itr = core::trait_attributes::map.find(str);
            if (itr == core::trait_attributes::map.end()) throw std::runtime_error("Bad trait attribute id " + std::string(str));
            return self->get_attrib(itr->second);
          },
          "bonuses", [] (const core::trait* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::stat_modifier* {
              if (counter >= self->bonuses.size() || self->bonuses[counter].invalid()) return nullptr;
              const auto bonus = &self->bonuses[counter];
              ++counter;
              return bonus;
            };
          },
          "opinion_modificators", [] (const core::trait* self) {
            size_t counter = 0;
            return [self, counter] () mutable -> const core::opinion_modifier* {
              if (counter >= self->opinion_mods.size() || self->opinion_mods[counter].invalid()) return nullptr;
              const auto op = &self->opinion_mods[counter];
              ++counter;
              return op;
            };
          },
          "attributes_start", sol::var(1),
          "attributes_count", sol::var(core::trait_attributes::count)
        );
        
        core.set_function("each_bonus", [] (const sol::object &obj, const sol::function &func) {
          if (!func.valid()) throw std::runtime_error("Invalid function input");
          
          if (obj.is<core::trait*>()) {
            const auto trait = obj.as<core::trait*>();
            size_t counter = 0;
//             static_assert(trait->bonuses.size() == core::trait::max_stat_modifiers_count);
            while (counter < trait->bonuses.size() && trait->bonuses[counter].valid()) {
              const auto bonus = &trait->bonuses[counter];
              const auto ret = func(bonus);
              CHECK_ERROR_THROW(ret);
              
              if (ret.get_type() == sol::type::boolean) {
                const bool r = ret;
                if (r) return;
              }
            }
            
            return;
          }
          
          if (obj.is<core::modificator*>()) {
            const auto mod = obj.as<core::modificator*>();
            size_t counter = 0;
//             static_assert(mod->bonuses.size() == core::modificator::max_stat_modifiers_count);
            while (counter < mod->bonuses.size() && mod->bonuses[counter].valid()) {
              const auto bonus = &mod->bonuses[counter];
              const auto ret = func(bonus);
              CHECK_ERROR_THROW(ret);
              
              if (ret.get_type() == sol::type::boolean) {
                const bool r = ret;
                if (r) return;
              }
            }
            
            return;
          }
          
          throw std::runtime_error("Invalid object input");
        });
        
        core.set_function("each_opinion_modificator", [] (const sol::object &obj, const sol::function &func) {
          if (!func.valid()) throw std::runtime_error("Invalid function input");
          
          if (obj.is<core::trait*>()) {
            const auto trait = obj.as<core::trait*>();
            size_t counter = 0;
//             static_assert(trait->opinion_mods.size() == core::trait::max_opinion_modifiers_count);
            while (counter < trait->opinion_mods.size() && trait->opinion_mods[counter].valid()) {
              const auto op = &trait->opinion_mods[counter];
              const auto ret = func(op);
              CHECK_ERROR_THROW(ret);
              
              if (ret.get_type() == sol::type::boolean) {
                const bool r = ret;
                if (r) return;
              }
            }
            
            return;
          }
          
          if (obj.is<core::modificator*>()) {
            const auto mod = obj.as<core::modificator*>();
            size_t counter = 0;
//             static_assert(mod->opinion_mods.size() == core::modificator::max_opinion_modifiers_count);
            while (counter < mod->opinion_mods.size() && mod->opinion_mods[counter].valid()) {
              const auto op = &mod->opinion_mods[counter];
              const auto ret = func(op);
              CHECK_ERROR_THROW(ret);
              
              if (ret.get_type() == sol::type::boolean) {
                const bool r = ret;
                if (r) return;
              }
            }
            
            return;
          }
          
          throw std::runtime_error("Invalid object input");
        });
      }
    }
  }
}
