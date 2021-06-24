#include "lua_initialization.h"
#include "lua_initialization_internal.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_types(sol::state_view lua) {
      internal::setup_lua_enums(lua);
      internal::setup_lua_province(lua);
      internal::setup_lua_building_type(lua);
      internal::setup_lua_city_type(lua);
      internal::setup_lua_city(lua);
      internal::setup_lua_titulus(lua);
      internal::setup_lua_character(lua);
      internal::setup_lua_faction(lua);
      internal::setup_lua_army(lua);

  //     // теперь по идее нужно задать основные типы объектов
  //     {
  //       sol::usertype<core::province> province_type = core.new_usertype<core::province>("province",
  //         sol::no_constructor,
  //         "title", sol::readonly(&core::province::title),
  //         "tiles", sol::readonly(&core::province::tiles),
  //         "neighbours", sol::readonly(&core::province::neighbours),
  //         "cities_max_count", sol::readonly(&core::province::cities_max_count),
  //         "cities_count", sol::readonly(&core::province::cities_count),
  //         "cities", sol::readonly_property([] (const core::province* self) { return std::ref(self->cities); }),
  //         "modificators", sol::readonly(&core::province::modificators),
  //         "events", sol::readonly(&core::province::events),
  //         "flags", sol::readonly(&core::province::flags),
  //         "modificators_container_size", sol::var(core::province::modificators_container_size),
  //         "events_container_size", sol::var(core::province::events_container_size),
  //         "flags_container_size", sol::var(core::province::flags_container_size),
  //         "cities_max_game_count", sol::var(core::province::cities_max_game_count)
  //       );
  //     }
  //
  //     {
  //       sol::usertype<core::building_type> building_type = core.new_usertype<core::building_type>("building_type",
  //         sol::no_constructor,
  //         "id", sol::readonly(&core::building_type::id),
  //         "name_id", sol::readonly(&core::building_type::name_id),
  //         "desc_id", sol::readonly(&core::building_type::desc_id),
  //         "potential", sol::readonly(&core::building_type::potential),
  //         "conditions", sol::readonly(&core::building_type::conditions),
  //         "prev_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->prev_buildings); }),
  //         "limit_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->limit_buildings); }),
  //         "replaced", sol::readonly(&core::building_type::replaced),
  //         "upgrades_from", sol::readonly(&core::building_type::upgrades_from),
  //         "time", sol::readonly(&core::building_type::time),
  //         "mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->mods); }),
  //         "unit_mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->unit_mods); }),
  //         "money_cost", sol::readonly(&core::building_type::money_cost),
  //         "authority_cost", sol::readonly(&core::building_type::authority_cost),
  //         "esteem_cost", sol::readonly(&core::building_type::esteem_cost),
  //         "influence_cost", sol::readonly(&core::building_type::influence_cost),
  //         "maximum_prev_buildings", sol::var(core::building_type::maximum_prev_buildings),
  //         "maximum_limit_buildings", sol::var(core::building_type::maximum_limit_buildings),
  //         "maximum_stat_modifiers", sol::var(core::building_type::maximum_stat_modifiers),
  //         "maximum_unit_stat_modifiers", sol::var(core::building_type::maximum_unit_stat_modifiers)
  //       );
  //     }
  //
  //     {
  //       sol::usertype<core::city_type> city_type = core.new_usertype<core::city_type>("city_type",
  //         sol::no_constructor,
  //         "id", sol::readonly(&core::city_type::id),
  //         "buildings", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->buildings); }),
  //         "stats", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->stats); }),
  //         "name_id", sol::readonly(&core::city_type::name_id),
  //         "desc_id", sol::readonly(&core::city_type::desc_id),
  //         "city_image", sol::readonly(&core::city_type::city_image),
  //         "city_icon", sol::readonly(&core::city_type::city_icon),
  //         "maximum_buildings", sol::var(core::city_type::maximum_buildings),
  //         "stats_count", sol::var(core::city_stats::count)
  //       );
  //     }
  //
  //     {
  //       sol::usertype<core::city> city = core.new_usertype<core::city>("city",
  //         sol::no_constructor,
  //         "name_id", sol::readonly(&core::city::name_index),
  //         "province", sol::readonly(&core::city::province),
  //         "title", sol::readonly(&core::city::title),
  //         "type", sol::readonly(&core::city::type),
  //         "available_buildings", sol::readonly(&core::city::available_buildings),
  //         "complited_buildings", sol::readonly(&core::city::complited_buildings),
  //         "start_building", sol::readonly(&core::city::start_building),
  //         "building_index", sol::readonly(&core::city::building_index),
  //         "tile_index", sol::readonly(&core::city::tile_index),
  //         "current_stats", sol::readonly_property([] (const core::city* self) { return std::ref(self->current_stats); }),
  //         "modificators", sol::readonly(&core::city::modificators),
  //         "modificators_container_size", sol::var(core::city::modificators_container_size),
  //         "buildings_size", sol::var(core::city_type::maximum_buildings)
  //       );
  //     }
  //
  //     {
  //       sol::usertype<core::titulus> title_type = core.new_usertype<core::titulus>("titulus",
  //         sol::no_constructor,
  //         "id", sol::readonly(&core::titulus::id),
  //         "type", sol::readonly(&core::titulus::type),
  //         "count", sol::readonly(&core::titulus::count),
  //         "parent", sol::readonly(&core::titulus::parent),
  //         "owner", sol::readonly(&core::titulus::owner),
  //         "name_id", sol::readonly(&core::titulus::name_str),
  //         "desc_id", sol::readonly(&core::titulus::description_str),
  //         "adj_id", sol::readonly(&core::titulus::adjective_str),
  //         "next", sol::readonly(&core::titulus::next),
  //         "prev", sol::readonly(&core::titulus::prev),
  //         "events", sol::readonly(&core::titulus::events),
  //         "flags", sol::readonly(&core::titulus::flags),
  //         "is_formal", &core::titulus::is_formal,
  // //           "set_child", &core::titulus::set_child,
  //         "get_child", &core::titulus::get_child,
  //         "get_province", &core::titulus::get_province,
  //         "get_city", &core::titulus::get_city,
  //         "events_container_size", sol::var(core::titulus::events_container_size),
  //         "flags_container_size", sol::var(core::titulus::flags_container_size)
  //         // наименования
  //       );
  //
  //       sol::table title_type_table = core["titulus"];
  //       auto title_type_type = title_type_table.new_enum("type_enum",
  //         "city", core::titulus::type::city,
  //         "baron", core::titulus::type::baron,
  //         "duke", core::titulus::type::duke,
  //         "king", core::titulus::type::king,
  //         "imperial", core::titulus::type::imperial
  //       );
  //     }
  //
  //     {
  //       auto character_type = core.new_usertype<core::character>("character",
  //         sol::no_constructor,
  //         "base_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->stats); }),
  //         "current_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->current_stats); }),
  //         "base_hero_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->hero_stats); }),
  //         "current_hero_stats", sol::readonly_property([] (const core::character* self) { return std::ref(self->current_hero_stats); }),
  //         "name_number", sol::readonly(&core::character::name_number),
  //         "born_day", sol::readonly(&core::character::born_day),
  //         "death_day", sol::readonly(&core::character::death_day),
  //         "name_id", sol::readonly(&core::character::name_str),
  //         "nick_id", sol::readonly(&core::character::nickname_str),
  //         "suzerain", sol::readonly(&core::character::suzerain),
  //         "imprisoner", sol::readonly(&core::character::imprisoner),
  //         "next_prisoner", sol::readonly(&core::character::next_prisoner),
  //         "prev_prisoner", sol::readonly(&core::character::prev_prisoner),
  //         "next_courtier", sol::readonly(&core::character::next_courtier),
  //         "prev_courtier", sol::readonly(&core::character::prev_courtier),
  //         "family", sol::readonly(&core::character::family),
  //         "relations", sol::readonly(&core::character::relations),
  //         "culture", sol::readonly(&core::character::culture),
  //         "religion", sol::readonly(&core::character::religion),
  //         "hidden_religion", sol::readonly(&core::character::hidden_religion),
  //         "factions", sol::readonly_property([] (const core::character* self) { return std::ref(self->factions); }),
  //         "traits", sol::readonly(&core::character::traits),
  //         "modificators", sol::readonly(&core::character::modificators),
  //         "events", sol::readonly(&core::character::events),
  //         "flags", sol::readonly(&core::character::flags),
  //         "is_independent", &core::character::is_independent,
  //         "is_prisoner", &core::character::is_prisoner,
  //         "is_married", &core::character::is_married,
  //         "is_male", &core::character::is_male,
  //         "is_hero", &core::character::is_hero,
  //         "is_player", &core::character::is_player,
  //         "is_dead", &core::character::is_dead,
  //         "has_dynasty", &core::character::has_dynasty,
  //         "is_ai_playable", &core::character::is_ai_playable,
  //         "get_main_title", &core::character::get_main_title,
  //         "get_bit", &core::character::get_bit,
  //         "has_flag", &core::character::has_flag,
  //         "has_trait", &core::character::has_trait,
  //         "has_modificator", &core::character::has_modificator,
  //         "has_event", &core::character::has_event,
  //         "traits_container_size", sol::var(core::character::traits_container_size),
  //         "modificators_container_size", sol::var(core::character::modificators_container_size),
  //         "events_container_size", sol::var(core::character::events_container_size),
  //         "flags_container_size", sol::var(core::character::flags_container_size),
  //         "stats_count", sol::var(core::character_stats::count),
  //         "hero_stats_count", sol::var(core::hero_stats::count)
  //       );
  //
  //       sol::table character_type_table = core["character"];
  //
  //       auto family = character_type_table.new_usertype<struct core::character::family>("character_family",
  //         "real_parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->real_parents); }),
  //         "parents", sol::readonly_property([] (const struct core::character::family* self) { return std::ref(self->parents); }),
  //         "children", sol::readonly(&core::character::family::children),
  //         "next_sibling", sol::readonly(&core::character::family::next_sibling),
  //         "prev_sibling", sol::readonly(&core::character::family::prev_sibling),
  //         "consort", sol::readonly(&core::character::family::consort),
  //         "previous_consorts", sol::readonly(&core::character::family::previous_consorts),
  //         "owner", sol::readonly(&core::character::family::owner),
  //         "concubines", sol::readonly(&core::character::family::concubines),
  //         "blood_dynasty", sol::readonly(&core::character::family::blood_dynasty),
  //         "dynasty", sol::readonly(&core::character::family::dynasty)
  //       );
  //
  //       auto relations = character_type_table.new_usertype<struct core::character::relations>("character_relations",
  //         "friends", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->friends); }),
  //         "rivals", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->rivals); }),
  //         "lovers", sol::readonly_property([] (const struct core::character::relations* self) { return std::ref(self->lovers); })
  //       );
  //     }
  //
  //     {
  //       auto faction_type = core.new_usertype<core::faction>("faction",
  //         sol::no_constructor,
  //         "leader", sol::readonly(&core::faction::leader),
  //         "heir", sol::readonly(&core::faction::heir),
  //         "liege", sol::readonly(&core::faction::liege),
  //         "state", sol::readonly(&core::faction::state),
  //         "council", sol::readonly(&core::faction::council),
  //         "tribunal", sol::readonly(&core::faction::tribunal),
  //         "vassals", sol::readonly(&core::faction::vassals),
  //         "next_vassal", sol::readonly(&core::faction::next_vassal),
  //         "prev_vassal", sol::readonly(&core::faction::prev_vassal),
  //         "titles", sol::readonly(&core::faction::titles),
  //         "main_title", sol::readonly(&core::faction::main_title),
  //         "courtiers", sol::readonly(&core::faction::courtiers),
  //         "prisoners", sol::readonly(&core::faction::prisoners),
  //         "stats", sol::readonly_property([] (const core::faction* self) { return std::ref(self->stats); }),
  //         "realm_mechanics", sol::readonly(&core::faction::mechanics),
  //         "is_independent", &core::faction::is_independent,
  //         "is_state", &core::faction::is_state,
  //         "is_council", &core::faction::is_council,
  //         "is_tribunal", &core::faction::is_tribunal,
  //         "is_self", &core::faction::is_self,
  //         "stats_count", sol::var(core::faction_stats::count),
  //         "realm_mechanics_size", sol::var(utils::realm_mechanics::count)
  //       );
  //     }

      // нужно еще сделать несколько функций: например поиск персонажей, доступные решения и проч
//       core.set_function("player_end_turn", player_end_turn);
    }
  }
}
