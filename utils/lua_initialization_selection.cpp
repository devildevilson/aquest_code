#include "lua_initialization_hidden.h"

#include "globals.h"
#include "systems.h"
#include "bin/objects_selection.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace utils {
    void setup_lua_selection(sol::state_view lua) {
      auto utils = lua["utils"].get_or_create<sol::table>();
      utils.new_usertype<systems::core_t::selection_containers>("selection_t", sol::no_constructor,
        "primary", &systems::core_t::selection_containers::primary,
        "secondary", &systems::core_t::selection_containers::secondary,
        "copy", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->copy(s.primary);
        },
        "copy_armies", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->copy_armies(s.primary);
        },
        "copy_heroes", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->copy_heroes(s.primary);
        },
        "copy_cities", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->copy_cities(s.primary);
        },
        "copy_structures", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->copy_structures(s.primary);
        },
        "copy_units", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->copy_units(s.primary);
        },
        "copy_buildings", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->copy_buildings(s.primary);
        },
        
        "add", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->add(s.primary);
        },
        "add_armies", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->add_armies(s.primary);
        },
        "add_heroes", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->add_heroes(s.primary);
        },
        "add_cities", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->add_cities(s.primary);
        },
        "add_structures", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->add_structures(s.primary);
        },
        "add_units", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->add_units(s.primary);
        },
        "add_buildings", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->add_buildings(s.primary);
        },
        
        "remove", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->remove(s.primary);
        },
        "remove_armies", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->remove_armies(s.primary);
        },
        "remove_heroes", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->remove_heroes(s.primary);
        },
        "remove_cities", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->remove_cities(s.primary);
        },
        "remove_structures", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->remove_structures(s.primary);
        },
        "remove_units", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->remove_units(s.primary);
        },
        "remove_buildings", [] () {
          auto &s = global::get<systems::core_t>()->selection;
          s.secondary->remove_buildings(s.primary);
        }
      );
      
//       int64_t has(const sol::object &obj) const;
//       sol::object get(const int64_t &index) const;
//       int64_t add(const sol::object &obj);
//       int64_t raw_add(const sol::object &obj);
//       bool remove(const int64_t &index);
//       void clear();
//       
//       void sort(const sol::function &predicate);
      
      utils.new_usertype<utils::objects_selection>("objects_selection", sol::no_constructor,
        "count", sol::readonly_property([] (const utils::objects_selection* self) { return self->count; }),
        "array", sol::readonly_property([] (const utils::objects_selection* self) { return std::ref(self->objects); }),
        "clear", &utils::objects_selection::clear,
        "sort", &utils::objects_selection::sort,
        "has", [] (const utils::objects_selection* self, const sol::object &obj) {
          const int64_t index = self->has(obj);
          return index < 0 ? index : TO_LUA_INDEX(index);
        },
        "add", [] (utils::objects_selection* self, const sol::object &obj) {
          const int64_t index = self->add(obj);
          return index < 0 ? index : TO_LUA_INDEX(index);
        },
        "get", [] (const utils::objects_selection* self, const int64_t &index) {
          return self->get(FROM_LUA_INDEX(index));
        },
        "remove", [] (utils::objects_selection* self, const int64_t &index) {
          return self->remove(FROM_LUA_INDEX(index));
        },
        "has_army", &utils::objects_selection::has_army,
        "has_hero", &utils::objects_selection::has_hero,
        "has_city", &utils::objects_selection::has_city,
        //"has_structure", &utils::objects_selection::has_structure,
        "has_unit", &utils::objects_selection::has_unit,
        "has_building", &utils::objects_selection::has_building
      );
      
      lua["selection"] = &global::get<systems::core_t>()->selection;
    }
  }
}
