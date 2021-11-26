#include "city.h"

#include "utils/globals.h"
#include "bin/game_time.h"
#include "building_type.h"
#include "character.h"

#include "bin/data_parser.h"
#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "utils/serializator_helper.h"
#include "utils/systems.h"

#include "core/context.h"

namespace devils_engine {
  namespace core {
    const structure city::s_type;
    const size_t city::bit_field_size;
    
    city::building_view::building_view() : off_troops_count(0), def_troops_count(0), off_troops(nullptr), def_troops(nullptr) {}
    city::building_view::~building_view() { clear(); }

    void city::building_view::create(const uint32_t &off_troops_count, const uint32_t &def_troops_count) {
      this->def_troops_count = def_troops_count;
      this->off_troops_count = off_troops_count;
      if (def_troops_count > 0) def_troops = new utils::handle<core::troop>[def_troops_count];
      if (off_troops_count > 0) off_troops = new utils::handle<core::troop>[off_troops_count];
    }
    
    void city::building_view::clear() {
      auto ctx = global::get<systems::map_t>()->core_context;
      for (size_t i = 0; i < def_troops_count; ++i) {
        ctx->destroy_troop(def_troops[i].get_token());
      }
      
      for (size_t i = 0; i < off_troops_count; ++i) {
        ctx->destroy_troop(off_troops[i].get_token());
      }
      
      delete [] def_troops;
      delete [] off_troops;
      def_troops = nullptr;
      off_troops = nullptr;
      def_troops_count = 0;
      off_troops_count = 0;
    }
    
    city::city() : 
      province(nullptr), 
      title(nullptr), 
      type(nullptr), 
      start_building(SIZE_MAX), 
      building_index(UINT32_MAX), 
      tile_index(UINT32_MAX),
      state(state::not_exist)
//       troops(nullptr)
    {}
    
    city::~city() {}
    
    bool city::check_build(const character* c, const uint32_t &building_index) const {
      if (building_index >= type->buildings_count) return false;
      
      // нужно быть или владельцем города или хозяином владельца
      // как это проверить?
      const bool ownership = check_ownership(c);
      
      const bool has_constructed = completed_buildings.get(building_index);
      const bool available = available_buildings.get(building_index);
      
      auto b = type->buildings[building_index];
      const bool has_resources = check_resources(b, c);
      
      // условия ниже включены в available, мне нужно описать требуемые здания в интерфейсе
      // я так полагаю нужно сделать еще один бит_фиелд в котором будет все кроме has_prev
      
      return start_building == SIZE_MAX && ownership && !has_constructed && available && has_resources;
    }
    
    bool city::start_build(character* c, const uint32_t &building_index) {
      if (!check_build(c, building_index)) return false;
      
      const float build_cost_mod = current_stats.get(city_stats::build_cost_factor);
      const float money_cost = type->buildings[building_index]->money_cost * build_cost_mod;
      const float influence_cost = type->buildings[building_index]->influence_cost * build_cost_mod;
      const float esteem_cost = type->buildings[building_index]->esteem_cost * build_cost_mod;
      const float authority_cost = type->buildings[building_index]->authority_cost * build_cost_mod;
      
      c->resources.add(character_resources::money, -money_cost);
      c->resources.add(character_resources::influence, -influence_cost);
      c->resources.add(character_resources::esteem, -esteem_cost);
      c->resources.add(character_resources::authority, -authority_cost);
      start_building = global::get<const utils::calendar>()->current_turn();
      this->building_index = building_index;
      
      return true;
    }
    
    size_t city::turns_to_complete() const {
      if (start_building == SIZE_MAX) return 0;
      
      const float build_time_mod = 1.0f + current_stats.get(city_stats::build_time_factor);
      const size_t current_turn = global::get<const utils::calendar>()->current_turn();
      const size_t time_passed = current_turn - start_building;
      const size_t build_time = type->buildings[building_index]->time * build_time_mod;
      return std::max(int64_t(build_time) - int64_t(time_passed), 0l);
    }
    
    bool city::has_building_project() const {
      return start_building != SIZE_MAX;
    }
    
    size_t city::find_building(const building_type* b) const {
      return type->find_building(b);
    }
    
    size_t city::find_building_upgrade(const building_type* b, const size_t &start) const {
      return type->find_building_upgrade(b, start);
    }
    
    bool city::check_ownership(const character* c) const {
      auto r = title->owner;
      ASSERT(r.valid());
      if (r->leader == c) return true;
      
      auto liege = r->liege.get();
      while (liege != nullptr) {
        const bool state = liege->leader == c;
        const bool council = liege->council.valid() && liege->council->leader == c;
        const bool tribunal = liege->tribunal.valid() && liege->tribunal->leader == c;
        const bool assembly = liege->assembly.valid() && liege->assembly->leader == c;
        const bool clergy = liege->clergy.valid() && liege->clergy->leader == c;
        
        if (state || council || tribunal || assembly || clergy) return true;
        
        liege = liege->liege.get();
      }
      
      return false;
    }
    
    bool city::check_limit(const building_type* b) const {
      for (size_t i = 0; i < b->limit_buildings.size() && b->limit_buildings[i] != nullptr; ++i) {
        auto l = b->limit_buildings[i];
        const size_t index = find_building(l);
        if (index == SIZE_MAX) continue;
        
        ASSERT(index < type->buildings_count);
        const bool complet = completed_buildings.get(index);
        if (complet) return false;
      }
      
      return true;
    }
    
    bool city::check_prerequisites(const building_type* b) const {
      for (size_t i = 0; i < b->prev_buildings.size() && b->prev_buildings[i] != nullptr; ++i) {
        auto l = b->prev_buildings[i];
        const size_t index = find_building(l);
        if (index == SIZE_MAX) continue;
        
        ASSERT(index < type->buildings_count);
        const bool complet = completed_buildings.get(index);
        if (!complet) return false;
      }
      
      return true;
    }
    
    bool city::check_resources(const building_type* b, const character* c) const {
      const float build_cost_mod = 1.0f + current_stats.get(city_stats::build_cost_factor); // по идее к этому моменту статы должны быть все расчитаны
      const float money_cost = b->money_cost * build_cost_mod;
      const float influence_cost = b->influence_cost * build_cost_mod;
      const float esteem_cost = b->esteem_cost * build_cost_mod;
      const float authority_cost = b->authority_cost * build_cost_mod;
      
      return c->resources.get(character_resources::money)     >= money_cost && 
             c->resources.get(character_resources::influence) >= influence_cost && 
             c->resources.get(character_resources::esteem)    >= esteem_cost && 
             c->resources.get(character_resources::authority) >= authority_cost;
    }
    
    void city::fill_troops() {
      // сначала заполним армии, но у нас все равно могут быть города в которых нет армии
      update_troops();
    }
    
    void city::update_turn() {
      if (start_building != SIZE_MAX) {
        const float build_time_mod = 1.0f + current_stats.get(city_stats::build_time_factor);
        const size_t build_time = type->buildings[building_index]->time * build_time_mod;
        const size_t current_turn = global::get<const utils::calendar>()->current_turn();
        if (current_turn - start_building >= build_time) {
          completed_buildings.set(building_index, true);
          start_building = SIZE_MAX;
          building_index = UINT32_MAX;
        }
      }
      
      // как проверить что видимо а что нет?
      // почему я не использую available_buildings?
      for (size_t i = 0; i < type->buildings_count; ++i) {
        auto b = type->buildings[i];
        available_buildings.set(i, true);
        visible_buildings.set(i, true);
        
        const bool limit = check_limit(b);
        if (!limit) available_buildings.set(i, false);
        if (!limit) visible_buildings.set(i, false);
        
        // возможно тут имеет смысл дать возможность просмотреть чего не хватает? да было бы неплохо
        const bool prevs = check_prerequisites(b);
        if (!prevs) available_buildings.set(i, false);
        
        if (b->replaced != nullptr) {
          const size_t index = find_building(b->replaced);
          if (index != SIZE_MAX) {
            const bool constr = constructed(index);
            if (constr) available_buildings.set(i, false);
            if (constr) visible_buildings.set(i, false);
          }
        }
        
        if (b->upgrades_from != nullptr) {
          const size_t index = find_building(b->upgrades_from);
          if (index != SIZE_MAX) {
            const bool constr = constructed(index);
            if (!constr) available_buildings.set(i, false);
            if (!constr) visible_buildings.set(i, false);
          }
        }
      }
    }
    
//     template <typename T>
//     size_t vector_find(const std::vector<T> &vec, T obj, const size_t &start = 0) {
//       for (size_t i = start; i < vec.size(); ++i) {
//         if (vec[i] == obj) return i;
//       }
//       
//       return SIZE_MAX;
//     }
    
    void city::update_troops() {
      // как обновить армию? нужно пройтись по армии... по армии ли? или по зданиям?
      // нужно пройтись в общем виде
      auto ctx = global::get<systems::map_t>()->core_context;
      
      // как проверить в походе ли армия?
      
      // что делать если армия ушла в поход? обновлять отряды нужно только тогда когда армия дома
      // или нет? можно наверное сделать какую нибудь механику типа вне города при строительстве
      // выдается побитый отряд, но с другой стороны лучше все же обновить армию когда она придет в домашнюю провинцию
      for (size_t i = 0; i < type->buildings_count; ++i) {
        const bool has_troops = units_view[i].off_troops_count > 0 || units_view[i].def_troops_count > 0;
        if (has_troops) { ASSERT(constructed(i)); }
        if (!constructed(i)) continue;
        
        auto b = type->buildings[i];
        const size_t index = find_building_upgrade(b);
        const bool upgrade_exists = index != SIZE_MAX && constructed(i);
        if (upgrade_exists) {
          units_view[i].clear();
          continue;
        }
        
        const bool building_has_troop_types = b->offensive_units[0] != nullptr || b->defensive_units[0] != nullptr;
        if (building_has_troop_types && !upgrade_exists && !has_troops) {
          uint32_t off_count = 0;
          uint32_t def_count = 0;
          for (; off_count < b->offensive_units.size() && b->offensive_units[off_count] != nullptr; ++off_count);
          for (; def_count < b->defensive_units.size() && b->defensive_units[def_count] != nullptr; ++def_count);
          
          units_view[i].create(off_count, def_count);
          
          for (size_t j = 0; j < b->offensive_units.size() && b->offensive_units[j] != nullptr; ++j) {
            auto t = b->offensive_units[j];
            
            auto troop = ctx->create_troop();
            troop->type = t;
            troop->origin = this;
            troop->provider = b;
            troop->stats = t->stats;
            troop->current_stats = t->stats;
            
            units_view[i].off_troops[j] = troop;
          }
          
          for (size_t j = 0; j < b->defensive_units.size() && b->defensive_units[j] != nullptr; ++j) {
            auto t = b->defensive_units[j];
            
            auto troop = ctx->create_troop();
            troop->type = t;
            troop->origin = this;
            troop->provider = b;
            troop->stats = t->stats;
            troop->current_stats = t->stats;
            
            units_view[i].def_troops[j] = troop;
          }
        }
      }
    }
    
    const utils::check_table_value city_table[] = {
      {
        "province",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "title",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "city_type",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "tile_index",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "constructed_buildings",
        utils::check_table_value::type::array_t,
        0, 0, {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      }
    };
    
    size_t add_city(const sol::table &table) {
      //return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::city), table);
      assert(false);
      (void)table;
      return SIZE_MAX;
    }
    
    bool validate_city(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "city" + std::to_string(index);
      }
      
      const size_t size = sizeof(city_table) / sizeof(city_table[0]);
      recursive_check(check_str, "city", table, nullptr, city_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_city_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator*) {
      const bool ret = validate_city(index, table);
      if (!ret) return false;
      
      sol::state_view state(lua);
      //auto keyallow = state.create_table();
      //keyallow.add("province", "title", "city_type", "tile_index");
      //auto str = table_to_string(lua, table, keyallow);
      auto str = utils::table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize city type table");
      ASSERT(false);
      //container->add_data(core::structure::city, std::move(str));
      
      return true;
    }
    
    void parse_city(core::city* city, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      {
        // индексы у нас приходят из луа, а это значит что все они будут смещены на единичку
        // тут в парсере нужно исправлять это дело, можно ли сделать это унифицированным способом?
        // так чтобы на всякий случай можно было бы быстро переключить? макросы короче говоря
        const size_t index = FROM_LUA_INDEX(table["province"].get<uint32_t>());
        auto province = ctx->get_entity<core::province>(index);
        city->province = province;
      }
      
      { // вообще удобно наверное здесь указать титул
        const std::string str = table["title"];
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find title " + str);
        auto title = ctx->get_entity<core::titulus>(index);
        city->title = title;
      }
      
      {
        const std::string str = table["city_type"];
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find city_type " + str);
        auto city_type = ctx->get_entity<core::city_type>(index);
        city->type = city_type;
        memcpy(city->current_stats.array.data(), city_type->stats.array.data(), sizeof(city_type->stats.array[0]) * core::city_stats::count);
      }
      
      city->tile_index = FROM_LUA_INDEX(table["tile_index"].get<uint32_t>());
      
      // пока все ???
    }
  }
}
