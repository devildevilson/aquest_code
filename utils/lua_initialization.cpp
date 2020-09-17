#include "lua_initialization.h"

#include "globals.h"
#include "bin/core_structures.h"
// #include "bin/helper.h"
#include "utility.h"
#include "bin/logic.h"
#include "input.h"
#include "progress_container.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_package_path(sol::state &lua) {
      {
        const std::string default_path = lua["package"]["path"];
        const std::string new_path = 
          global::root_directory() + "scripts/?.lua;" + 
          global::root_directory() + "scripts/?/init.lua;" + 
          default_path;
        lua["package"]["path"] = new_path;
      }
      
      {
        const std::string default_path = lua["package"]["cpath"];
        const std::string new_path = 
  #ifdef __linux__
          global::root_directory() + "scripts/?.so;" + // этого достаточно?
  #elif _WIN32
          global::root_directory() + "scripts/?.dll;" + 
  #endif
          default_path;
        lua["package"]["cpath"] = new_path;
      }
    }
    
    void setup_lua_constants(sol::state &lua) {
      //auto constants = lua["constants"].get_or_create<sol::table>();
      auto target = lua.create_table_with(
        "time_precision", TIME_PRECISION,
        "one_second", ONE_SECOND,
        "half_second", HALF_SECOND,
        "third_second", THIRD_SECOND,
        "quarter_second", QUARTER_SECOND,
        "fifth_second", FIFTH_SECOND,
        "tenth_second", TENTH_SECOND,
        "time_string", TIME_STRING,
        "app_name", APP_NAME,
        //"app_version_str", APP_VERSION_STR,
        "technical_name", TECHNICAL_NAME,
        "engine_name", ENGINE_NAME,
        "engine_version", ENGINE_VERSION,
        //"engine_version_str", ENGINE_VERSION_STR,
        "pi", PI,
        "pi_2", PI_2,
        "pi_h", PI_H,
        "pi_q", PI_Q,
        "pi_e", PI_E,
        "epsilon", EPSILON,
        "size_max", -1,
        "uint32_max", UINT32_MAX
      );
      
      target.set_function("deg_to_rad", [] (const double &deg) { return DEG_TO_RAD(deg); });
      target.set_function("rad_to_deg", [] (const double &rad) { return RAD_TO_DEG(rad); });
      
      sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
      lua["constants"] = lua.create_table(0, 0, sol::metatable_key, x);
      
//       auto progress = lua.new_usertype<utils::progress_container>("progress_container",
//         "current_progress", &utils::progress_container::current_progress,
//         "steps_count", &utils::progress_container::steps_count,
//         "hint", &utils::progress_container::hint,
//         "is_finished", &utils::progress_container::is_finished
//       );
    }
    
    template <typename T>
    sol::table create_enum(sol::table &core, const std::string &name) {
      sol::table target = core.create(static_cast<int>(T::count), static_cast<int>(0));
      for (size_t i = 0; i < T::count; ++i) {
        target.set(magic_enum::enum_name<T>(static_cast<T>(i)), i);
      }
      
      sol::table x = core.create_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
      sol::table shim = core.create_named(name, sol::metatable_key, x);
      return shim;
    }
    
    void setup_lua_types(sol::state &lua) {
      //lua["ctx"] = &global::get<interface::context>()->ctx;
      
      // тут видимо тоже нужно задать энумы
      auto core = lua["core"].get_or_create<sol::table>();
      core["character_stats"] = create_enum<core::character_stats::values>(core, "character_stats");
      core["troop_stats"] = create_enum<core::troop_stats::values>(core, "troop_stats");
      core["hero_stats"] = create_enum<core::hero_stats::values>(core, "hero_stats");
      core["opinion_stats"] = create_enum<core::opinion_stats::values>(core, "opinion_stats");
      core["faction_stats"] = create_enum<core::faction_stats::values>(core, "faction_stats");
      core["province_stats"] = create_enum<core::province_stats::values>(core, "province_stats");
      core["city_stats"] = create_enum<core::city_stats::values>(core, "city_stats");
      core["army_stats"] = create_enum<core::army_stats::values>(core, "army_stats");
      core["hero_troop_stats"] = create_enum<core::hero_troop_stats::values>(core, "hero_troop_stats");
      
//       {
//         auto enum_table = core.new_enum("character_stats");
//         for (size_t i = 0; i < core::character_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::character_stats::values>(static_cast<core::character_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("troop_stats");
//         for (size_t i = 0; i < core::troop_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::troop_stats::values>(static_cast<core::troop_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("hero_stats");
//         for (size_t i = 0; i < core::hero_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::hero_stats::values>(static_cast<core::hero_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("opinion_stats"); // тут сложнее, из луа добавятся еще некоторые переменные
//         for (size_t i = 0; i < core::opinion_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::opinion_stats::values>(static_cast<core::opinion_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("faction_stats");
//         for (size_t i = 0; i < core::faction_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::faction_stats::values>(static_cast<core::faction_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("province_stats");
//         for (size_t i = 0; i < core::province_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::province_stats::values>(static_cast<core::province_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("city_stats");
//         for (size_t i = 0; i < core::city_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::city_stats::values>(static_cast<core::city_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("army_stats");
//         for (size_t i = 0; i < core::army_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::army_stats::values>(static_cast<core::army_stats::values>(i)), i);
//         }
//       }
//       
//       {
//         auto enum_table = core.new_enum("hero_troop_stats");
//         for (size_t i = 0; i < core::hero_troop_stats::count; ++i) {
//           enum_table.set(magic_enum::enum_name<core::hero_troop_stats::values>(static_cast<core::hero_troop_stats::values>(i)), i);
//         }
//       }
      
      // теперь по идее нужно задать основные типы объектов
      {
        sol::usertype<core::province> province_type = core.new_usertype<core::province>("province",
          sol::no_constructor,
          "title", sol::readonly(&core::province::title),
          "tiles", sol::readonly(&core::province::tiles),
          "neighbours", sol::readonly(&core::province::neighbours),
          "cities_max_count", sol::readonly(&core::province::cities_max_count),
          "cities_count", sol::readonly(&core::province::cities_count),
          "cities", sol::readonly_property([] (const core::province* self) { return std::ref(self->cities); }),
          "modificators", sol::readonly(&core::province::modificators),
          "events", sol::readonly(&core::province::events),
          "flags", sol::readonly(&core::province::flags),
          "modificators_container_size", sol::var(core::province::modificators_container_size),
          "events_container_size", sol::var(core::province::events_container_size),
          "flags_container_size", sol::var(core::province::flags_container_size),
          "cities_max_game_count", sol::var(core::province::cities_max_game_count)
        );
      }
      
      {
        sol::usertype<core::building_type> building_type = core.new_usertype<core::building_type>("building_type",
          sol::no_constructor,
          "id", sol::readonly(&core::building_type::id),
          "name_id", sol::readonly(&core::building_type::name_id),
          "desc_id", sol::readonly(&core::building_type::desc_id),
          "potential", sol::readonly(&core::building_type::potential),
          "conditions", sol::readonly(&core::building_type::conditions),
          "prev_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->prev_buildings); }),
          "limit_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->limit_buildings); }),
          "replaced", sol::readonly(&core::building_type::replaced),
          "upgrades_from", sol::readonly(&core::building_type::upgrades_from),
          "time", sol::readonly(&core::building_type::time),
          "mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->mods); }),
          "unit_mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->unit_mods); }),
          "money_cost", sol::readonly(&core::building_type::money_cost),
          "authority_cost", sol::readonly(&core::building_type::authority_cost),
          "esteem_cost", sol::readonly(&core::building_type::esteem_cost),
          "influence_cost", sol::readonly(&core::building_type::influence_cost),
          "maximum_prev_buildings", sol::var(core::building_type::maximum_prev_buildings),
          "maximum_limit_buildings", sol::var(core::building_type::maximum_limit_buildings),
          "maximum_stat_modifiers", sol::var(core::building_type::maximum_stat_modifiers),
          "maximum_unit_stat_modifiers", sol::var(core::building_type::maximum_unit_stat_modifiers)
        );
      }
      
      {
        sol::usertype<core::city_type> city_type = core.new_usertype<core::city_type>("city_type",
          sol::no_constructor,
          "id", sol::readonly(&core::city_type::id),
          "buildings", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->buildings); }),
          "stats", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->stats); }),
          "name_id", sol::readonly(&core::city_type::name_id),
          "desc_id", sol::readonly(&core::city_type::desc_id),
          "city_image", sol::readonly(&core::city_type::city_image),
          "city_icon", sol::readonly(&core::city_type::city_icon),
          "maximum_buildings", sol::var(core::city_type::maximum_buildings),
          "stats_count", sol::var(core::city_stats::count)
        );
      }
      
      {
        sol::usertype<core::city> city = core.new_usertype<core::city>("city",
          sol::no_constructor,
          "name_id", sol::readonly(&core::city::name_index),
          "province", sol::readonly(&core::city::province),
          "title", sol::readonly(&core::city::title),
          "type", sol::readonly(&core::city::type),
          "available_buildings", sol::readonly(&core::city::available_buildings),
          "complited_buildings", sol::readonly(&core::city::complited_buildings),
          "start_building", sol::readonly(&core::city::start_building),
          "building_index", sol::readonly(&core::city::building_index),
          "tile_index", sol::readonly(&core::city::tile_index),
          "current_stats", sol::readonly_property([] (const core::city* self) { return std::ref(self->current_stats); }),
          "modificators", sol::readonly(&core::city::modificators),
          "modificators_container_size", sol::var(core::city::modificators_container_size),
          "buildings_size", sol::var(core::city_type::maximum_buildings)
        );
      }
      
      {
        sol::usertype<core::titulus> title_type = core.new_usertype<core::titulus>("titulus",
          sol::no_constructor,
          "id", sol::readonly(&core::titulus::id),
          "type", sol::readonly(&core::titulus::type),
          "count", sol::readonly(&core::titulus::count),
          "parent", sol::readonly(&core::titulus::parent),
          "owner", sol::readonly(&core::titulus::owner),
          "name_id", sol::readonly(&core::titulus::name_str),
          "desc_id", sol::readonly(&core::titulus::description_str),
          "adj_id", sol::readonly(&core::titulus::adjective_str),
          "next", sol::readonly(&core::titulus::next),
          "prev", sol::readonly(&core::titulus::prev),
          "events", sol::readonly(&core::titulus::events),
          "flags", sol::readonly(&core::titulus::flags),
          "is_formal", &core::titulus::is_formal,
  //           "set_child", &core::titulus::set_child,
          "get_child", &core::titulus::get_child,
          "get_province", &core::titulus::get_province,
          "get_city", &core::titulus::get_city,
          "events_container_size", sol::var(core::titulus::events_container_size),
          "flags_container_size", sol::var(core::titulus::flags_container_size)
          // наименования
        );
        
        sol::table title_type_table = core["titulus"];
        auto title_type_type = title_type_table.new_enum("type_enum",
          "city", core::titulus::type::city,
          "baron", core::titulus::type::baron,
          "duke", core::titulus::type::duke,
          "king", core::titulus::type::king,
          "imperial", core::titulus::type::imperial
        );
      }
      
      {
        auto character_type = core.new_usertype<core::character>("character",
          sol::no_constructor,
          "base_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->stats); }),
          "current_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->current_stats); }),
          "base_hero_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->hero_stats); }),
          "current_hero_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->current_hero_stats); }),
          "name_number", sol::readonly(&core::character::name_number),
          "born_day", sol::readonly(&core::character::born_day),
          "death_day", sol::readonly(&core::character::death_day),
          "name_id", sol::readonly(&core::character::name_str),
          "nick_id", sol::readonly(&core::character::nickname_str),
          "suzerain", sol::readonly(&core::character::suzerain),
          "imprisoner", sol::readonly(&core::character::imprisoner),
          "next_prisoner", sol::readonly(&core::character::next_prisoner),
          "prev_prisoner", sol::readonly(&core::character::prev_prisoner),
          "next_courtier", sol::readonly(&core::character::next_courtier),
          "prev_courtier", sol::readonly(&core::character::prev_courtier),
          "family", sol::readonly(&core::character::family),
          "relations", sol::readonly(&core::character::relations),
          "culture", sol::readonly(&core::character::culture),
          "religion", sol::readonly(&core::character::religion),
          "hidden_religion", sol::readonly(&core::character::hidden_religion),
          "factions", sol::readonly_property([] (const core::character* self) { return std::ref(self->factions); }),
          "traits", sol::readonly(&core::character::traits),
          "modificators", sol::readonly(&core::character::modificators),
          "events", sol::readonly(&core::character::events),
          "flags", sol::readonly(&core::character::flags),
          "is_independent", &core::character::is_independent,
          "is_prisoner", &core::character::is_prisoner,
          "is_married", &core::character::is_married,
          "is_male", &core::character::is_male,
          "is_hero", &core::character::is_hero,
          "is_player", &core::character::is_player,
          "is_dead", &core::character::is_dead,
          "has_dynasty", &core::character::has_dynasty,
          "is_ai_playable", &core::character::is_ai_playable,
          "get_main_title", &core::character::get_main_title,
          "get_bit", &core::character::get_bit,
          "has_flag", &core::character::has_flag,
          "has_trait", &core::character::has_trait,
          "has_modificator", &core::character::has_modificator,
          "has_event", &core::character::has_event,
          "traits_container_size", sol::var(core::character::traits_container_size),
          "modificators_container_size", sol::var(core::character::modificators_container_size),
          "events_container_size", sol::var(core::character::events_container_size),
          "flags_container_size", sol::var(core::character::flags_container_size),
          "stats_count", sol::var(core::character_stats::count),
          "hero_stats_count", sol::var(core::hero_stats::count)
        );
        
        sol::table character_type_table = core["character"];
        
        auto family = character_type_table.new_usertype<struct core::character::family>("character_family",
          "real_parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->real_parents); }),
          "parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->parents); }),
          "children", sol::readonly(&core::character::family::children),
          "next_sibling", sol::readonly(&core::character::family::next_sibling),
          "prev_sibling", sol::readonly(&core::character::family::prev_sibling),
          "consort", sol::readonly(&core::character::family::consort),
          "previous_consorts", sol::readonly(&core::character::family::previous_consorts),
          "owner", sol::readonly(&core::character::family::owner),
          "concubines", sol::readonly(&core::character::family::concubines),
          "blood_dynasty", sol::readonly(&core::character::family::blood_dynasty),
          "dynasty", sol::readonly(&core::character::family::dynasty)
        );
        
        auto relations = character_type_table.new_usertype<struct core::character::relations>("character_relations",
          "friends", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->friends); }),
          "rivals", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->rivals); }),
          "lovers", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->lovers); })
        );
      }
      
      {
        auto faction_type = core.new_usertype<core::faction>("faction",
          sol::no_constructor,
          "leader", sol::readonly(&core::faction::leader),
          "heir", sol::readonly(&core::faction::heir),
          "liege", sol::readonly(&core::faction::liege),
          "state", sol::readonly(&core::faction::state),
          "council", sol::readonly(&core::faction::council),
          "tribunal", sol::readonly(&core::faction::tribunal),
          "vassals", sol::readonly(&core::faction::vassals),
          "next_vassal", sol::readonly(&core::faction::next_vassal),
          "prev_vassal", sol::readonly(&core::faction::prev_vassal),
          "titles", sol::readonly(&core::faction::titles),
          "main_title", sol::readonly(&core::faction::main_title),
          "courtiers", sol::readonly(&core::faction::courtiers),
          "prisoners", sol::readonly(&core::faction::prisoners),
          "stats", sol::readonly_property([] (const core::faction* self) { return std::ref(self->stats); }),
          "realm_mechanics", sol::readonly(&core::faction::mechanics),
          "is_independent", &core::faction::is_independent,
          "is_state", &core::faction::is_state,
          "is_council", &core::faction::is_council,
          "is_tribunal", &core::faction::is_tribunal,
          "is_self", &core::faction::is_self,
          "stats_count", sol::var(core::faction_stats::count),
          "realm_mechanics_size", sol::var(utils::realm_mechanics::count)
        );
      }
      
      // нужно еще сделать несколько функций: например поиск персонажей, доступные решения и проч
//       core.set_function("player_end_turn", player_end_turn);
    }
    
    void setup_lua_input(sol::state &lua) {
      {
        auto utils = lua["utils"].get_or_create<sol::table>();
        auto id = utils.new_usertype<utils::id>("id",
          "valid", &utils::id::valid,
          "name", &utils::id::name,
          "num", &utils::id::num,
          "get", &utils::id::get
        );
      }
      
      {
        auto target = lua.create_table_with(
          "release", input::release,
          "press", input::press,
          "repeated", input::repeated,
          
          "state_initial", input::state_initial,
          "state_press", input::state_press,
          "state_click", input::state_click,
          "state_double_press", input::state_double_press,
          "state_double_click", input::state_double_click,
          "state_long_press", input::state_long_press,
          "state_long_click", input::state_long_click,
          
          "long_press_time", input::long_press_time,
          "double_press_time", input::double_press_time,
          
          "event_key_slots", input::event_key_slots
        );
        
        target.set_function("check_event", input::check_event);
        target.set_function("timed_check_event", input::timed_check_event);
        target.set_function("input_event_state", input::input_event_state);
        target.set_function("is_event_pressed", input::is_event_pressed);
        target.set_function("is_event_released", input::is_event_released);
        target.set_function("get_event_key_name", input::get_event_key_name);
        target.set_function("get_event", input::get_event);
        target.set_function("get_framebuffer_size", input::get_framebuffer_size);
        target.set_function("get_window_content_scale", input::get_window_content_scale);
        target.set_function("get_monitor_content_scale", input::get_monitor_content_scale);
        target.set_function("get_monitor_physical_size", input::get_monitor_physical_size);
        
        // тут добавятся несколько функций для того чтобы задать клавишу в настройках
        sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        lua["input"] = lua.create_table(0, 0, sol::metatable_key, x);
      }
    }
    
    void setup_lua_game_logic(sol::state &lua) {
      {
        auto target = lua.create_table();
        target.set_function("player_end_turn", game::player_end_turn);
        target.set_function("current_player_turn", game::current_player_turn);
        
        // тут добавятся несколько функций для того чтобы задать клавишу в настройках
        sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        lua["game"] = lua.create_table(0, 0, sol::metatable_key, x);
      }
    }
  }
}