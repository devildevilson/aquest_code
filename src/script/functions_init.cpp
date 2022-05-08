#include "functions_init.h"

#include "core/structures_header.h"
#include "core/realm_mechanics_arrays.h"

#include "system.h"
#include "change_context_commands_macro.h"
#include "condition_commands_macro.h"
#include "action_functions.h"
#include "common_commands.h"
#include "iterator_funcs.h"
#include "core/scripted_types.h"

// template <int64_t... args>
// constexpr size_t check_function() {
//   return sizeof...(args);
// }
// 
// static_assert(check_function<1>() == 1);

namespace devils_engine {
//   static bool is_ai_glob(const core::character* c) {
//     return c->is_ai();
//   }
  
  static bool has_religion_feature(const core::religion* rel, const size_t &feat) {
    return rel->get_mechanic(feat);
  }
  
  static bool has_culture_feature(const core::culture* cul, const size_t &feat) {
    return cul->get_mechanic(feat);
  }
  
  template <typename T>
  static double get_current_stat(const T obj, const int64_t id) {
    return obj->current_stats.get(id);
  }
  
  template <typename T>
  static double get_base_stat(const T obj, const int64_t id) {
    return obj->stats.get(id);
  }
  
  template <typename T>
  static double get_resource(const T obj, const int64_t id) {
    return obj->resources.get(id);
  }
  
  template <typename T>
  static double get_current_hero_stat(const T obj, const int64_t id) {
    return obj->current_hero_stats.get(id);
  }
  
  template <typename T>
  static double get_base_hero_stat(const T obj, const int64_t id) {
    return obj->hero_stats.get(id);
  }
  
  // тут к сожалению id наверное не получится добавить в качестве аргумента
  // добавляем мы к базе? к другому добавлять бесполезно
  template <int64_t stat_id, typename T>
  static void add_stat(T obj, const double val) {
    obj->stats.add(stat_id, val);
  }
  
  template <int64_t stat_id, typename T>
  static void add_resource(T obj, const double val) {
    obj->resources.add(stat_id, val);
  }
  
  template <int64_t stat_id, typename T>
  static void add_hero_stat(T obj, const double val) {
    obj->hero_stats.add(stat_id, val);
  }
  
  namespace script {
    void register_functions(system* sys) {
      sys->register_usertype<character_t>();
      sys->register_usertype<culture_t>();
      sys->register_usertype<religion_t>();
      sys->register_usertype<titulus_t>();
      sys->register_usertype<province_t>();
      sys->register_usertype<city_t>();
      sys->register_usertype<culture_group_t>(); 
      sys->register_usertype<religion_group_t>();
      sys->register_usertype<city_type_t>();
      sys->register_usertype<holding_type_t>();
      sys->register_usertype<building_type_t>();
      sys->register_usertype<casus_belli_t>();
      sys->register_usertype<troop_type_t>();
      sys->register_usertype<trait_t>();
      sys->register_usertype<modificator_t>();
      sys->register_usertype<law_t>();
      sys->register_usertype<realm_t>();
      sys->register_usertype<war_t>();
      sys->register_usertype<army_t>();
      sys->register_usertype<hero_troop_t>();
      sys->register_usertype<troop_t>();
      // появится еще army_type
      
      /* ---------------------------------------------------------------------------------------------------------------------------------- */
      // getters
      
#define GET_ENTITY_FUNC(type, func_type) {                               \
          const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<func_type> init) -> interface* { \
            auto str = v.make_scripted_string(obj);                      \
            return init.init(str);                                       \
          };                                                             \
          sys->REG_USER(void, func_type, type##_t, #type)(f);            \
        }                                                                \
        
#define GET_ENTITY_HELPER_FUNC(type) GET_ENTITY_FUNC(type, get_##type)
      
      GET_ENTITY_HELPER_FUNC(titulus)
      GET_ENTITY_HELPER_FUNC(culture)
      GET_ENTITY_HELPER_FUNC(religion)
      
      GET_ENTITY_HELPER_FUNC(culture_group)
      GET_ENTITY_HELPER_FUNC(religion_group)
      GET_ENTITY_HELPER_FUNC(trait)
      GET_ENTITY_HELPER_FUNC(modificator)
      GET_ENTITY_HELPER_FUNC(casus_belli)
      GET_ENTITY_HELPER_FUNC(building_type)
      GET_ENTITY_HELPER_FUNC(holding_type)
      GET_ENTITY_HELPER_FUNC(city_type)
      GET_ENTITY_HELPER_FUNC(troop_type)
      GET_ENTITY_HELPER_FUNC(law)
      
#undef GET_ENTITY_HELPER_FUNC
#undef GET_ENTITY_FUNC
      
      // вычислять ли тут строку? хороший вопрос, честно говоря не особо нужно
      // было бы неплохо сделать еще alias
      const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<get_titulus> init) -> interface* {
        auto str = v.make_scripted_string(obj);
        return init.init(str);
      };
      sys->REG_USER(void, get_titulus, titulus_t, "title")(f);
      
      /* ---------------------------------------------------------------------------------------------------------------------------------- */
      // basic boolean & number
      //sys->REG_FUNC(core::character, character_t, is_ai, "is_ai")();
      
#define CONDITION_COMMAND_FUNC(name) sys->REG_FUNC(character_t, core::character::name, #name)();
      CHARACTER_GET_BOOL_NO_ARGS_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) sys->REG_FUNC(character_t, core::character::get_##name, #name)();
      CHARACTER_GET_NUMBER_NO_ARGS_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

#define CONDITION_ARG_COMMAND_FUNC(name, input_type, constness, full_input_type) sys->REG_FUNC(character_t, core::character::name, #name)();
      CHARACTER_GET_BOOL_ONE_ARG_COMMANDS_LIST
#undef CONDITION_ARG_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) sys->REG_FUNC(realm_t, core::realm::name, #name)();
      REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

      // пока не сделал
// #define CONDITION_ARG_COMMAND_FUNC(name, input_type, constness, full_input_type) sys->REG_FUNC(core::realm, realm_t, name, #name)();
//       REALM_GET_BOOL_ONE_ARG_COMMANDS_LIST
// #undef CONDITION_ARG_COMMAND_FUNC


      {
        using func_type = scripted_function_args<realm_t, decltype(&core::realm::has_right), &core::realm::has_right, "has_right"_create, size_t>;
        const auto f = [] (system::view, const sol::object &obj, container::delayed_initialization<func_type> init) -> interface* {
          if (obj.get_type() != sol::type::string) throw std::runtime_error("'has_right' function expects right name");
          const auto str = obj.as<std::string_view>();
          const auto itr = core::power_rights::map.find(str);
          if (itr == core::power_rights::map.end()) throw std::runtime_error("Could not find power right " + std::string(str));
          const size_t num = itr->second;
          return init.init(num);
        };
        sys->REG_USER(realm_t, func_type, bool, "has_right")(f);
      }
      
      {
        using func_type = scripted_function_args<realm_t, decltype(&core::realm::has_state_right), &core::realm::has_state_right, "has_state_right"_create, size_t>;
        const auto f = [] (system::view, const sol::object &obj, container::delayed_initialization<func_type> init) -> interface* {
          if (obj.get_type() != sol::type::string) throw std::runtime_error("'has_state_right' function expects right name");
          const auto str = obj.as<std::string_view>();
          const auto itr = core::state_rights::map.find(str);
          if (itr == core::state_rights::map.end()) throw std::runtime_error("Could not find state right " + std::string(str));
          const size_t num = itr->second;
          return init.init(num);
        };
        sys->REG_USER(realm_t, func_type, bool, "has_state_right")(f);
      }
      
      {
        // тут пока непонятно
        using func_type = scripted_function_args<realm_t, decltype(&core::realm::has_enacted_law_with_flag), &core::realm::has_enacted_law_with_flag, "has_enacted_law_with_flag"_create, size_t>;
        const auto f = [] (system::view, const sol::object &obj, container::delayed_initialization<func_type> init) -> interface* {
          if (obj.get_type() != sol::type::string) throw std::runtime_error("'has_enacted_law_with_flag' function expects right name");
          const auto str = obj.as<std::string_view>();
//           const auto itr = core::state_rights::map.find(str);
//           if (itr == core::state_rights::map.end()) throw std::runtime_error("Could not find state right " + std::string(str));
//           const size_t num = itr->second;
//           return init.init(num);
        };
        sys->REG_USER(realm_t, func_type, bool, "has_enacted_law_with_flag")(f);
      }
      
// #define CONDITION_ARG_COMMAND_FUNC(name, input_type, constness, full_input_type) sys->REG_FUNC(core::province, province_t, name, #name)();
//       PROVINCE_GET_BOOL_ONE_ARG_COMMANDS_LIST
// #undef CONDITION_ARG_COMMAND_FUNC

// #define CONDITION_ARG_COMMAND_FUNC(name, input_type, constness, full_input_type) sys->REG_FUNC(core::religion, religion_t, name, #name)();
//       RELIGION_GET_BOOL_ONE_ARG_COMMANDS_LIST
// #undef CONDITION_ARG_COMMAND_FUNC

      {
        using func_type = scripted_function_args<religion_t, decltype(&has_religion_feature), &has_religion_feature, "has_religion_feature"_create, size_t>;
        const auto f = [] (system::view, const sol::object &obj, container::delayed_initialization<func_type> init) -> interface* {
          if (obj.get_type() != sol::type::string) throw std::runtime_error("'has_religion_feature' function expects right name");
          const auto str = obj.as<std::string_view>();
          const auto itr = core::religion_mechanics::map.find(str);
          if (itr == core::religion_mechanics::map.end()) throw std::runtime_error("Could not find state right " + std::string(str));
          const size_t num = itr->second;
          return init.init(num);
        };
        sys->REG_USER(religion_t, func_type, bool, "has_religion_feature")(f);
      }
      
      {
        using func_type = scripted_function_args<culture_t, decltype(&has_culture_feature), &has_culture_feature, "has_culture_feature"_create, size_t>;
        const auto f = [] (system::view, const sol::object &obj, container::delayed_initialization<func_type> init) -> interface* {
          if (obj.get_type() != sol::type::string) throw std::runtime_error("'has_culture_feature' function expects right name");
          const auto str = obj.as<std::string_view>();
          const auto itr = core::religion_mechanics::map.find(str);
          if (itr == core::religion_mechanics::map.end()) throw std::runtime_error("Could not find state right " + std::string(str));
          const size_t num = itr->second;
          return init.init(num);
        };
        sys->REG_USER(culture_t, func_type, bool, "has_culture_feature")(f);
      }
      
// #define CONDITION_COMMAND_FUNC(name) sys->REG_FUNC(core::titulus, titulus_t, name, #name)();
//       TITLE_GET_BOOL_NO_ARGS_COMMANDS_LIST
// #undef CONDITION_COMMAND_FUNC

#define CASUS_BELLI_FLAG_FUNC(name) sys->REG_FUNC(const core::casus_belli*, core::casus_belli::name, #name)();
      CASUS_BELLI_FLAGS_LIST
#undef CASUS_BELLI_FLAG_FUNC

#define CASUS_BELLI_NUMBER_FUNC(name) sys->REG_FUNC(const core::casus_belli*, core::casus_belli::get_##name, #name)();
      CASUS_BELLI_NUMBERS_LIST
#undef CASUS_BELLI_NUMBER_FUNC

      /* ---------------------------------------------------------------------------------------------------------------------------------- */
      // stats
      //sys->register_function<core::character, character_t, decltype(&get_current_stat<character_t>), &get_current_stat<character_t>, "military"_create, core::character_stats::military>();

#define COMMON_GET_STAT_FUNC(name, type_handle, stat_type) \
  sys->register_function<type_handle, decltype(&get_current_stat<type_handle>), &get_current_stat<type_handle>, #name##_create, core::stat_type::name>(); \
  sys->register_function<type_handle, decltype(&get_base_stat<type_handle>), &get_base_stat<type_handle>, "base_"#name##_create, core::stat_type::name>();

#define CHARACTER_PENALTY_STAT_FUNC(name) \
  sys->register_function<character_t, decltype(&get_current_stat<character_t>), \
    &get_current_stat<character_t>, #name"_penalty"_create, core::character_stats::name##_penalty>();  \
  sys->register_function<character_t, decltype(&get_base_stat<character_t>), \
    &get_base_stat<character_t>, "base_"#name"_penalty"_create, core::character_stats::name##_penalty>();  \
    
  
#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, character_t, character_stats)
      CHARACTER_STATS_LIST
#undef STAT_FUNC

#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, realm_t, realm_stats)
      REALM_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, province_t, province_stats)
      PROVINCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, city_t, city_stats)
      CITY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, army_t, army_stats)
      ARMY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, troop_t, troop_stats)
      TROOP_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, hero_troop_t, hero_troop_stats)
      HERO_TROOP_STATS_LIST
#undef STAT_FUNC

#undef COMMON_GET_STAT_FUNC

#define COMMON_GET_STAT_FUNC(name, type_handle, stat_type) \
  sys->register_function<type_handle, decltype(&get_current_hero_stat<type_handle>), &get_current_hero_stat<type_handle>, #name##_create, core::stat_type::name>(); \
  sys->register_function<type_handle, decltype(&get_base_hero_stat<type_handle>), &get_base_hero_stat<type_handle>, "base_"#name##_create, core::stat_type::name>();

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, character_t, hero_stats)
      HERO_STATS_LIST
#undef STAT_FUNC
  
#undef COMMON_GET_STAT_FUNC

#define COMMON_GET_STAT_FUNC(name, type_handle, stat_type) \
  sys->register_function<type_handle, decltype(&get_resource<type_handle>), &get_resource<type_handle>, #name##_create, core::stat_type::name>();
  
#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, character_t, character_resources)
      RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, realm_t, realm_resources)
      RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, city_t, city_resources)
      CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_GET_STAT_FUNC(name, army_t, army_resources)
      ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC
  
#undef COMMON_GET_STAT_FUNC

      // как сделать add? функция принимает Th и double, возвращает воид
#define COMMON_ADD_STAT_FUNC(name, type_handle, stat_type) \
  sys->register_function<type_handle, decltype(&add_stat<core::stat_type::name, type_handle>), &add_stat<core::stat_type::name, type_handle>, "add_"#name##_create>();
  
#define CHARACTER_PENALTY_STAT_FUNC(name) \
  sys->register_function<character_t, decltype(&add_stat<core::character_stats::name##_penalty, character_t>), \
    &add_stat<core::character_stats::name##_penalty, character_t>, "add_"#name"_penalty"_create>();
    
#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, character_t, character_stats)
     CHARACTER_STATS_LIST
#undef STAT_FUNC

#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, realm_t, realm_stats)
      REALM_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, province_t, province_stats)
      PROVINCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, city_t, city_stats)
      CITY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, army_t, army_stats)
      ARMY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, troop_t, troop_stats)
      TROOP_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, hero_troop_t, hero_troop_stats)
      HERO_TROOP_STATS_LIST
#undef STAT_FUNC
  
#undef COMMON_ADD_STAT_FUNC

#define COMMON_ADD_STAT_FUNC(name, type_handle, stat_type) \
  sys->register_function<type_handle, decltype(&add_hero_stat<core::stat_type::name, type_handle>), &add_hero_stat<core::stat_type::name, type_handle>, "add_"#name##_create>();

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, character_t, hero_stats)
      HERO_STATS_LIST
#undef STAT_FUNC
  
#undef COMMON_ADD_STAT_FUNC

#define COMMON_ADD_STAT_FUNC(name, type_handle, stat_type) \
  sys->register_function<type_handle, decltype(&add_resource<core::stat_type::name, type_handle>), &add_resource<core::stat_type::name, type_handle>, "add_"#name##_create>();
  
#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, character_t, character_resources)
      RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, realm_t, realm_resources)
      RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, city_t, city_resources)
      CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_ADD_STAT_FUNC(name, army_t, army_resources)
      ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC
  
#undef COMMON_ADD_STAT_FUNC

      /* ---------------------------------------------------------------------------------------------------------------------------------- */
      // scopes
      
#define GET_SCOPE_COMMAND_FUNC(name, input, output, output_type) sys->REG_FUNC(character_t, core::character::get_##name, #name)();
      CHARACTER_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(name, input, output, output_type) sys->REG_FUNC(realm_t, core::realm::get_##name, #name)();
      REALM_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(name, input, output, output_type) sys->REG_FUNC(titulus_t, core::titulus::get_##name, #name)();
      TITLE_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(name, input, output, output_type) sys->REG_FUNC(army_t, core::army::get_##name, #name)();
      ARMY_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(name, input, output, output_type) sys->REG_FUNC(province_t, core::province::get_##name, #name)();
      PROVINCE_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

      sys->REG_FUNC(city_t, core::city::get_title, "title")();
      sys->REG_FUNC(city_t, core::city::get_city_type, "city_type")(); 

#define GET_SCOPE_COMMAND_FUNC(name, input, output, output_type) sys->REG_FUNC(war_t, core::war::get_##name, #name)();
      WAR_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(name, input, output, output_type) sys->REG_FUNC(const core::trait*, core::trait::get_##name, #name)();
      TRAIT_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

      sys->REG_FUNC(character_t, core::character::get_culture, "culture")();
      sys->REG_FUNC(character_t, core::character::get_religion, "religion")(); 
      sys->REG_FUNC(character_t, core::character::get_culture_group, "culture_group")();
      sys->REG_FUNC(character_t, core::character::get_religion_group, "religion_group")(); 
      sys->REG_FUNC(province_t, core::province::get_culture, "culture")();
      sys->REG_FUNC(province_t, core::province::get_religion, "religion")();
      sys->REG_FUNC(province_t, core::province::get_culture_group, "culture_group")();
      sys->REG_FUNC(province_t, core::province::get_religion_group, "religion_group")();
      
      sys->REG_FUNC(culture_t, core::culture::get_culture_group, "culture_group")();
      
      sys->REG_FUNC(religion_t, core::religion::get_head, "head")(); 
      sys->REG_FUNC(religion_t, core::religion::get_head_heir, "head_heir")();
      sys->REG_FUNC(religion_t, core::religion::get_religion_group, "religion_group")();
      
      /* ---------------------------------------------------------------------------------------------------------------------------------- */
      // тут видимо пойдут итераторы, итераторы требуют функцию итератор, она отдельно пишется
      
//       sys->REG_EVERY(core::character, character_t, each_ancestor, "each_ancestor")();
//       sys->REG_HAS(core::character, character_t, each_ancestor, "each_ancestor")();
//       sys->register_random<core::character, character_t, decltype(&each_ancestor), &each_ancestor, "each_ancestor"_create>();
      
#define COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, handle_type)  \
  sys->REG_EVERY(handle_type, each_##name, "every_"#name)();   \
  sys->REG_HAS(handle_type, each_##name, "has_"#name)();       \
  sys->REG_RANDOM(handle_type, each_##name, "random_"#name)(); \
  
#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, character_t)
      CHARACTER_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, realm_t)
      REALM_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, war_t)
      WAR_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, religion_t)
      RELIGION_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, culture_t)
      CULTURE_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, province_t)
      PROVINCE_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, titulus_t)
      TITLE_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, void*)
      CHANGE_CONTEXT_COMMANDS_GLOBAL_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#undef COMMON_CHANGE_CONTEXT_COMMAND_FUNC
      
      /* ---------------------------------------------------------------------------------------------------------------------------------- */
      // тут место для эффектов, эффекты наверное вручную добавляются
      
#define DECLARE_ADD_FLAG_FUNC(handle_type)        \
      {                                                      \
        using cur_func = add_flag<handle_type>;   \
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<cur_func> init) -> interface* { \
          const auto obj_type = obj.get_type();              \
          interface* flag = nullptr;                         \
          interface* time = nullptr;                         \
          if (obj_type == sol::type::string) {               \
            flag = v.make_scripted_string(obj);              \
          } else if (obj_type == sol::type::table) {         \
            const auto t = obj.as<sol::table>();             \
            const auto str_proxy = t["flag"];                \
            if (!str_proxy.valid()) throw std::runtime_error("Function 'add_flag' expects 'flag' to be a scripted string"); \
            flag = v.make_scripted_string(str_proxy);        \
            if (const auto time_proxy = t["time"]; time_proxy.valid()) { \
              time = v.make_scripted_numeric(time_proxy);    \
            }                                                \
          }                                                  \
          return init.init(flag, time);                      \
        };                                                   \
        sys->REG_USER(handle_type, cur_func, void, "add_flag")(f); \
      }                                                      \
      
      
      DECLARE_ADD_FLAG_FUNC(character_t)
      DECLARE_ADD_FLAG_FUNC(province_t)
      DECLARE_ADD_FLAG_FUNC(city_t)
      DECLARE_ADD_FLAG_FUNC(titulus_t)
      DECLARE_ADD_FLAG_FUNC(culture_t)
      DECLARE_ADD_FLAG_FUNC(religion_t)
      DECLARE_ADD_FLAG_FUNC(realm_t)
      DECLARE_ADD_FLAG_FUNC(war_t)
      DECLARE_ADD_FLAG_FUNC(army_t)
      DECLARE_ADD_FLAG_FUNC(hero_troop_t)
      
#undef DECLARE_ADD_FLAG_FUNC
      
      {
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<add_hook> init) -> interface* {
          const auto obj_type = obj.get_type();
          
          // хук это что? это некоторая возможность повлиять на персонажа
          // для хука нужно понять: его тип (сильный/слабый), против кого этот хук, какой секрет за ним скрывается, и опционально сколько висит
          // 
          
          return init.init(nullptr);
        };
        sys->REG_USER(character_t, add_hook, void, "add_hook")(f);
      }
      
      {
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<add_trait> init) -> interface* {
          auto ent = v.make_scripted_object(type_id<trait_t>(), obj);
          return init.init(ent);
        };
        sys->REG_USER(character_t, add_trait, void, "add_trait")(f);
      }
      
      {
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<marry> init) -> interface* {
          auto ent = v.make_scripted_object(type_id<character_t>(), obj);
          return init.init(ent);
        };
        sys->REG_USER(character_t, marry, void, "marry")(f);
      }
      
      sys->REG_USER(character_t, divorce, void, "divorce")();
      
      {
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<start_war> init) -> interface* {
          if (obj.get_type() != sol::type::table) throw std::runtime_error("Function 'start_war' expects table as input");
          const auto t = obj.as<sol::table>();
          const auto target_proxy = t["target"];
          if (!target_proxy.valid()) throw std::runtime_error("Function 'start_war' expects target to be specified");
          const auto cb_proxy = t["cb"];
          if (!cb_proxy.valid()) throw std::runtime_error("Function 'start_war' expects cb to be specified");
          const auto claimant_proxy = t["claimant"];
          if (!claimant_proxy.valid()) throw std::runtime_error("Function 'start_war' expects claimant to be specified");
          const auto titles_proxy = t["titles"];
          if (!titles_proxy.valid()) throw std::runtime_error("Function 'start_war' expects titles to be specified");
          
          auto target = v.make_scripted_object(type_id<character_t>(), target_proxy);
          auto cb = v.make_scripted_object(type_id<casus_belli_t>(), cb_proxy);
          auto claimant = v.make_scripted_object(type_id<character_t>(), claimant_proxy);
          
          // как быть с титулами? ожидаем таблицу с перечисленными титулами? или ожидаем лист?
          // хороший вопрос, я бы пока наверное таблицу оставил
          if (titles_proxy.get_type() != sol::type::table) throw std::runtime_error("Function 'start_war' expects titles to be table");
          const auto titles_t = titles_proxy.get<sol::table>();
          interface* t_begin = nullptr;
          interface* t_current = nullptr;
          for (const auto &pair : titles_t) {
            const auto first_type = pair.first.get_type();
            if (first_type != sol::type::number) continue; //  && first_type != sol::type::string
            
            auto local = v.make_scripted_object(type_id<titulus_t>(), pair.second);
            if (t_begin == nullptr) t_begin = local;
            if (t_current != nullptr) t_current->next = local;
            t_current = local;
          }
          
          return init.init(target, cb, claimant, t_begin, nullptr, nullptr);
        };
        sys->REG_USER(character_t, start_war, void, "start_war")(f);
      }
      
      {
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<end_war> init) -> interface* {
          if (obj.get_type() != sol::type::string) throw std::runtime_error("Function 'end_war' expects end war type");
          const auto str = obj.as<std::string_view>();
          // ищем тип окончания: победа атакующего, победа обороняющегося, белый мир, невалидная война
          
        };
        sys->REG_USER(war_t, end_war, void, "end_war")(f);
      }
      
      {
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<add_attacker> init) -> interface* {
          auto ent = v.make_scripted_object(type_id<character_t>(), obj);
          return init.init(ent);
        };
        sys->REG_USER(war_t, add_attacker, void, "add_attacker")(f);
      }
      
      {
        const auto f = [] (system::view v, const sol::object &obj, container::delayed_initialization<imprison> init) -> interface* {
          // у нас потенциально может быть несколько тюрем доступно: гостюрьма и селфтюрьма
          // мы стараемся посадить в ту тюрьму к реалму которой принадлежит персонаж
          // посадить мы можем только в тюрьму под управлением текущего персонажа
          auto ent = v.make_scripted_object(type_id<character_t>(), obj);
          return init.init(ent, nullptr);
        };
        sys->REG_USER(war_t, imprison, void, "imprison")(f);
      }
      
      // кажется это все функции которые сейчас более менее работают
    }
  }
}
