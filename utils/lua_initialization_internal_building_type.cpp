#include "lua_initialization_internal.h"

#include "core/building_type.h"
#include "lua_initialization.h"
#include "magic_enum_header.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_building_type(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        sol::usertype<core::building_type> building_type = core.new_usertype<core::building_type>("building_type", sol::no_constructor,
          "id", sol::readonly(&core::building_type::id),
          "name_id", sol::readonly(&core::building_type::name_id),
          "description_id", sol::readonly(&core::building_type::description_id),
//           "potential", sol::readonly(&core::building_type::potential),
//           "conditions", sol::readonly(&core::building_type::conditions),
          "prev_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->prev_buildings); }),
          "limit_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->limit_buildings); }),
          "each_prev_building", [] (const core::building_type* self) { 
            size_t counter = 0;
            return [self, counter] () mutable {
              using ptr_type = const core::building_type*;
              if (counter >= core::building_type::maximum_prev_buildings) return ptr_type(nullptr);
              const size_t curr = counter;
              ++counter;
              return self->prev_buildings[curr];
            };
          },
          "each_limit_building", [] (const core::building_type* self) { 
            size_t counter = 0;
            return [self, counter] () mutable {
              using ptr_type = const core::building_type*;
              if (counter >= core::building_type::maximum_limit_buildings) return ptr_type(nullptr);
              const size_t curr = counter;
              ++counter;
              return self->limit_buildings[curr];
            };
          },
          "replaced", sol::readonly(&core::building_type::replaced),
          "upgrades_from", sol::readonly(&core::building_type::upgrades_from),
          "time", sol::readonly(&core::building_type::time),
          "mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->mods); }),
          "unit_mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->unit_mods); }),
          "each_mod", [] (const core::building_type* self) { 
            size_t counter = 0;
            return [self, counter] () mutable {
              using ptr_type = const core::stat_modifier*;
              if (counter >= core::building_type::maximum_stat_modifiers) return ptr_type(nullptr);
              const size_t curr = counter;
              ++counter;
              return self->mods[curr].valid() ? &self->mods[curr] : nullptr;
            };
          },
          "each_unit_mod", [] (const core::building_type* self) {
            size_t counter = 0;
            return [self, counter] () mutable {
              using ptr_type = const core::unit_stat_modifier*;
              if (counter >= core::building_type::maximum_unit_stat_modifiers) return ptr_type(nullptr);
              const size_t curr = counter;
              ++counter;
              return self->unit_mods[curr].valid() ? &self->unit_mods[curr] : nullptr;
            };
          },
          "money_cost", sol::readonly(&core::building_type::money_cost),
          "authority_cost", sol::readonly(&core::building_type::authority_cost),
          "esteem_cost", sol::readonly(&core::building_type::esteem_cost),
          "influence_cost", sol::readonly(&core::building_type::influence_cost),
          "maximum_prev_buildings", sol::var(core::building_type::maximum_prev_buildings),
          "maximum_limit_buildings", sol::var(core::building_type::maximum_limit_buildings),
          "maximum_stat_modifiers", sol::var(core::building_type::maximum_stat_modifiers),
          "maximum_unit_stat_modifiers", sol::var(core::building_type::maximum_unit_stat_modifiers)
        );
        
        core.set_function("each_prev_building", [] (const core::building_type* type, const sol::function &func) {
          if (type == nullptr) throw std::runtime_error("Invalid realm");
          if (!func.valid()) throw std::runtime_error("Invalid function");
                          
          for (size_t i = 0; i < core::building_type::maximum_prev_buildings && type->prev_buildings[i] != nullptr; ++i) {
            auto b = type->prev_buildings[i];
            const auto ret = func(b);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
        
        core.set_function("each_limit_building", [] (const core::building_type* type, const sol::function &func) {
          if (type == nullptr) throw std::runtime_error("Invalid realm");
          if (!func.valid()) throw std::runtime_error("Invalid function");
                          
          for (size_t i = 0; i < core::building_type::maximum_limit_buildings && type->limit_buildings[i] != nullptr; ++i) {
            auto b = type->limit_buildings[i];
            const auto ret = func(b);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
        
        core.set_function("each_mod", [] (const core::building_type* type, const sol::function &func) {
          if (type == nullptr) throw std::runtime_error("Invalid realm");
          if (!func.valid()) throw std::runtime_error("Invalid function");
                          
          for (size_t i = 0; i < core::building_type::maximum_stat_modifiers && type->mods[i].valid(); ++i) {
            auto b = type->mods[i];
            const auto ret = func(b);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
        
        core.set_function("each_limit_building", [] (const core::building_type* type, const sol::function &func) {
          if (type == nullptr) throw std::runtime_error("Invalid realm");
          if (!func.valid()) throw std::runtime_error("Invalid function");
                          
          for (size_t i = 0; i < core::building_type::maximum_limit_buildings && type->unit_mods[i].valid(); ++i) {
            auto b = type->unit_mods[i];
            const auto ret = func(b);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
      }
    }
  }
}
