#include "lua_initialization.h"

#include "bin/image_parser.h"
#include "bin/data_parser.h"
#include "bin/heraldy_parser.h"
#include "magic_enum_header.h"
#include "serializator_helper.h"
#include "systems.h"
#include "bin/map_creator.h"
#include "globals.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace utils {
    static std::string serialize_table(const sol::table &t) {
      auto creator = global::get<systems::map_t>()->map_creator;
      return creator->serialize_table(t);
    }
    
//     void add_province2(sol::this_state s, const sol::table &t) {
//       const bool ret = validate_province(0, t);
//       if (!ret) throw std::runtime_error("Province validation error");
//       
//       std::string value = serialize_table(s, t);
//       global::get<world_serializator>()->add_data(core::structure::province, std::move(value));
//     }
    
    void load_provinces2(sol::this_state s, const std::string_view &path) {
      // как то валидировать здесь путь? было бы неплохо
      sol::state_view lua = s;
      auto table = lua.create_table(1, 0);
      table.add(path);
      
      std::string value = serialize_table(table);
      global::get<world_serializator>()->add_data(world_serializator::province, std::move(value));
    }
    
    // можно вот как сделать, раньше я запоминал таблицы, так как я напрямую из них брал информацию
    // теперь я их сразу в строку перевожу, значит я сначала дамплю на диск, а потом 
    // как обычная загрузка созданной карты, тут нужно сделать несколько дополнительных функций:
    // ресайз, сет
    
    #define ADD_FUNC(type) size_t add_##type##2(const sol::table &t) {                                \
      const bool ret = validate_##type(global::get<world_serializator>()->get_data_count(world_serializator::type), t); \
      if (!ret) throw std::runtime_error(std::string(#type) + " validation error");                   \
      std::string value = serialize_table(t);                                                         \
      return TO_LUA_INDEX(global::get<world_serializator>()->add_data(world_serializator::type, std::move(value))); \
    }
    
    #define LOAD_FUNC(type) void load_##type##2(sol::this_state s, const std::string_view &path) { \
      sol::state_view lua = s;                                                                     \
      auto table = lua.create_table(1, 0);                                                         \
      table.add(path);                                                                             \
      std::string value = serialize_table(table);                                                  \
      global::get<world_serializator>()->add_data(world_serializator::type, std::move(value));     \
    }
    
    ADD_FUNC(image)
    ADD_FUNC(heraldy_layer)
    ADD_FUNC(biome)
    ADD_FUNC(province)
    ADD_FUNC(building_type)
    ADD_FUNC(city_type)
    ADD_FUNC(city)
//     ADD_FUNC(trait)
//     ADD_FUNC(modificator)
//     ADD_FUNC(troop_type)
//     ADD_FUNC(decision)
//     ADD_FUNC(religion_group)
//     ADD_FUNC(religion)
//     ADD_FUNC(culture)
//     ADD_FUNC(law)
//     ADD_FUNC(event)
    ADD_FUNC(title)
    ADD_FUNC(character)
//     ADD_FUNC(dynasty)
    
    LOAD_FUNC(image)
    LOAD_FUNC(heraldy_layer)
    LOAD_FUNC(biome)
    LOAD_FUNC(province)
    LOAD_FUNC(building_type)
    LOAD_FUNC(city_type)
    LOAD_FUNC(city)
//     LOAD_FUNC(trait)
//     LOAD_FUNC(modificator)
//     LOAD_FUNC(troop_type)
//     LOAD_FUNC(decision)
//     LOAD_FUNC(religion_group)
//     LOAD_FUNC(religion)
//     LOAD_FUNC(culture)
//     LOAD_FUNC(law)
//     LOAD_FUNC(event)
    LOAD_FUNC(title)
    LOAD_FUNC(character)
//     LOAD_FUNC(dynasty)
    
    void setup_lua_utility_map_generator_functions(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name(reserved_lua::utils)].get_or_create<sol::table>();
//       utils.set_function("add_image", &add_image);
//       utils.set_function("load_images", [] (sol::this_state s, const std::string_view &path) {
//         sol::state_view lua = s;
//         auto t = lua.create_table(1, 0);
//         t.add(path);
//         add_image(t);
//       });
//       utils.set_function("add_biome", &add_biome);
//       utils.set_function("load_biomes", [] (sol::this_state s, const std::string_view &path) {
//         sol::state_view lua = s;
//         auto t = lua.create_table(1, 0);
//         t.add(path);
//         add_biome(t);
//       });
//       utils.set_function("add_province", &add_province);
//       utils.set_function("add_building", &add_building);
//       utils.set_function("add_character", &add_character);
//       utils.set_function("add_city", &add_city);
//       utils.set_function("add_city_type", &add_city_type);
// //       utils.set_function("add_event", &add_event);
//       utils.set_function("add_heraldy_layer", &add_heraldy_layer);
//       utils.set_function("add_title", &add_title);
      
//       LOAD_FUNC("load_provinces", add_province)
//       LOAD_FUNC("load_buildings", add_building)
//       LOAD_FUNC("load_characters", add_character)
//       LOAD_FUNC("load_cities", add_city)
//       LOAD_FUNC("load_city_types", add_city_type)
//       LOAD_FUNC("load_heraldy_layers", add_heraldy_layer)
//       LOAD_FUNC("load_titles", add_title)
      
      #define SET_UTILS_ADD_FUNC(type) utils.set_function("add_" + std::string(#type), &add_##type##2);
      #define SET_UTILS_LOAD_FUNC(type) utils.set_function("load_" + std::string(#type) + "s", &load_##type##2);
      
      SET_UTILS_ADD_FUNC(image)
      SET_UTILS_ADD_FUNC(heraldy_layer)
//       SET_UTILS_ADD_FUNC(biome)
      SET_UTILS_ADD_FUNC(province)
      SET_UTILS_ADD_FUNC(building_type)
      SET_UTILS_ADD_FUNC(city_type)
      SET_UTILS_ADD_FUNC(city)
      SET_UTILS_ADD_FUNC(title)
      SET_UTILS_ADD_FUNC(character)
      
      SET_UTILS_LOAD_FUNC(image)
      SET_UTILS_LOAD_FUNC(heraldy_layer)
//       SET_UTILS_LOAD_FUNC(biome)
      SET_UTILS_LOAD_FUNC(province)
      SET_UTILS_LOAD_FUNC(building_type)
      SET_UTILS_LOAD_FUNC(city_type)
      //SET_UTILS_LOAD_FUNC(city)
      utils.set_function("load_cities", &load_city2);
      SET_UTILS_LOAD_FUNC(title)
      SET_UTILS_LOAD_FUNC(character)
      
      utils.set_function("resize_characters", [] (const size_t &size) {
        return global::get<world_serializator>()->resize_data(world_serializator::character, size);
      });
      
      utils.set_function("set_character", [] (const uint32_t &index, const sol::table &t) {
        const bool ret = validate_character(FROM_LUA_INDEX(index), t);
        if (!ret) throw std::runtime_error("character validation error");
        std::string value = serialize_table(t);
        return global::get<world_serializator>()->set_data(world_serializator::character, FROM_LUA_INDEX(index), value);
      });
      
      utils.set_function("load_localization_table", [] (std::string path) {
        global::get<world_serializator>()->add_localization(std::move(path));
      });
    }
  }
}
