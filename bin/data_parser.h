#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <algorithm>
//#include "core_structures.h"
#include <unordered_map>
#include <unordered_set>
#include <any>
#include <stack>
#include "declare_structures.h"

#define UINT_VALUE_NOT_FOUND SIZE_MAX
#define UINT_VALUE_NOT_VALID (UINT_VALUE_NOT_FOUND-1)

// я хочу из входных данных вида
// {
//   "id": "event1",
//   "type": "event",
//   "target": {
//     "character": {
//       "age": 45
//     }
//   },
//   "mean_time_to_happen": 400,
//   "name": "name2",
//   "desc": "desc1",
//   "picture": "image_id.7",
//   "options": [
//     {
//       "highlight": true,
//       "name": "option_name1",
//       "set_country_flag": "qwqrwqr",
//       "country_id": {
//         "declare_war_with_cb": {
//           "who": "target_or_suzerain",
//           "casus_belli": "cb_independence_war"
//         }
//       }
//     },
//     {
//       
//     }
//   ]
// }
// получались функции проверки эвента (?)
// то есть func(event, target) -> bool 
// наверное можно составить этот скрипт в луа
// local event_table = {
//   id = "string_id",
//   type = event,
//   target = character,
//   potential = {
//     -- свойства персонажа
//     age = 45, -- если есть то по идее достаточно проверять раз в год
//     has_trait = "illness",
//     has_flag = "dfqwf",
//     is_player = 2, -- 0 - не игрок, 1 - игрок, 2 - неважно (по умолчанию)
//     is_alive = 1, -- по умолчанию
//   },
//   mean_time_to_happen = 400,
//   name = "event_name_id",
//   desc = "event_desc_id",
//   picture = "image_id.7",
//   important = true,
//   trigger_once = true,
//   options = {
//     {
//       name = "option_name1",
//       add_character_flag = "qwdqwd",
//       add_trait = "illness"
//     },
//     {
//       visible_if = {
//         target = {
//           health = 5
//         }
//       }
//     }
//   }
// }
// 
// local decision_table = {
//   id = "string_id",
//   type = decision,
//   availability = {
//     
//   },
//   allow = {
//     money = year_income
//   },
//   effect = {
//     remove_money = year_income,
//     add_prestige = 10
//   }
// }

// короче я так понял нужно весь конфиг сделать в луа (его так проще написать)
// 

struct nk_context;

#define ID_ARRAY "id_array"
#define STATS_ARRAY "stats_array"
#define MODIFIERS_ARRAY "modifiers_array"
#define STAT_ID "stat"
#define INDEX_ARRAY "index_array"

namespace devils_engine {
  namespace utils {
    class world_serializator;
    
    // желательно все строки все же убрать
    // я могу это сделать и более менее даже нормально сериализавать если сохранять последовательность загрузки данных
    // то есть у меня нескоторые эвенты приходят из конфига и конфиг мы загружаем первым, затем мы генерируем карту 
    // или грузим ее с диска, что загрузка, что генерация должны сохранять последовательность того что мы делаем 
    // по уникальному сиду (другое дело что при генерации сгенерируются страны и персонажи которые нам особенно не нужны)
    // в функции check_data_type_func мы можем проверить дает ли строка какой нибудь индекс
    // цепочку функций мы можем развернуть в один последовательный массив, мало того мы можем успешно использовать стек,
    // стек мы сможем еще передавать в последовательные вызовы эвентов
    // нам потребуется для этого несколько операций: непосредственно операция условий или эффекта, смена контекста, смена операции сравнения
    
    struct target_data {
      enum class type {
        character,
        province,
        city,
//         faction,
        army,
        hero_troop,
//         decision,
        religion,
        culture,
//         law,
        dynasty,
//         event,
        troop,
//         party_member,
        count
      };
      
      enum type type;
      union {
        struct core::character* character;
        struct core::province* province;
        struct core::city* city;
//         struct core::faction* faction;
        struct core::army* army;
        struct core::hero_troop* hero_troop;
//         struct core::decision* decision;
        struct core::religion* religion;
        struct core::culture* culture;
//         struct core::law* law;
        struct core::dynasty* dynasty;
//         struct core::event* event;
        struct core::troop* troop;
//         struct core::party_member* party_member;
      };
    };
    
    // этот контейнер должен не изменяться никогда в игре, то есть указатели на объекты должны быть доступны всегда
    // мы можем просто создать рекурсивные функции прямо в functions_container и составлять массив прямо в них
    // мы можем использовать вместо мапы вектор и так сделать приоритизацию 
    // (другое дело что в некоторых функциях потребуется искать по id это означает проверку всего массива)
    struct functions_container {
      union data_container {
        double dval;
        size_t uval;
        int64_t ival;
        void* pval;
        //std::string_view view; // нельзя запихать в union
      };
      
      using change_context_func = std::function<target_data(const target_data &)>;
      using description_func = std::function<void(const target_data &, nk_context*, const uint32_t &, const data_container*)>;
      using change_context_description_func = std::function<target_data(const target_data &, nk_context*)>;
      using condition_func = std::function<bool(const target_data &, const uint32_t &, const data_container*)>; // нам же данные еще сюда передать нужно
      using effect_func = std::function<void(const target_data &, const uint32_t &, const data_container*)>;
      using check_data_type_func = std::function<data_container(const sol::object &)>;
      
      enum class stack_operation {
        nothing,
        retrieve1_from_stack,
        retrieve1_from_stack_end,
        retrieve2_from_stack,
        retrieve2_from_stack_end,
        retrieve3_from_stack,
        retrieve3_from_stack_end,
        retrieve4_from_stack,
        retrieve4_from_stack_end,
        place_to_stack,
        place_to_local_stack,
        place_to_local_stack_end,
        place_values_to_stack,
        place_values_to_stack_end,
        count
      };
      
      enum class logic_operation {
        op_or,
        op_or_end,
        op_and,
        op_and_end,
        op_nand,
        op_nand_end,
        op_nor,
        op_nor_end,
        op_eq, // ?
        op_eq_end,
        op_neq, // ?
        op_neq_end,
        count
      };
      
      struct condition_cont;
      struct effect_cont;
      struct change_context_cont;
      
      struct operation {
        uint32_t type;
        enum logic_operation logic_operation;
        enum stack_operation stack_operation;
        union {
          const condition_cont* condition;
          const effect_cont* effect;
          const change_context_cont* context;
        };
        
        data_container data;
      };
      
      using create_condition = std::function<void(const std::string &, const sol::table &, const enum target_data::type &, std::vector<operation> &)>;
      using create_effect = std::function<void(const std::string &, const sol::table &, const enum target_data::type &, std::vector<operation> &)>;
      using create_context_changer = std::function<void(const std::string &, const sol::table &, const enum target_data::type &, std::vector<operation> &)>;
      using creation_func = std::function<void(const std::string &, const sol::table &, const enum target_data::type &, std::vector<operation> &)>;
      using validation_func = std::function<bool(const std::string &, const sol::table &, const enum target_data::type &, size_t &)>;
      
      struct condition_cont {
        enum logic_operation logic_operation;
        enum target_data::type type;
        condition_func func;
        check_data_type_func check;
        description_func desc; // эти функции возможно еще должны вернуть размер?
      };
      
      struct effect_cont {
        enum stack_operation stack_operation;
        enum target_data::type type;
        effect_func func;
        check_data_type_func check;
        description_func desc;
      };
      
      struct change_context_cont {
        enum stack_operation stack_operation;
        enum target_data::type type;
        enum target_data::type new_type;
        change_context_func func;
        check_data_type_func check;
        change_context_description_func desc;
      };
      
      struct complex_func {
        enum logic_operation logic_operation;
        enum stack_operation stack_operation;
//         enum target_data::type type; // если переключение контекста
//         enum target_data::type new_type;
        creation_func func;
        validation_func validator;
      };
      
//       std::unordered_map<std::string, condition_cont> conditions;
//       std::unordered_map<std::string, change_context_cont> change_context;
//       std::unordered_map<std::string, effect_cont> effects;
      std::vector<std::pair<std::string, condition_cont>> conditions;
      std::vector<std::pair<std::string, change_context_cont>> change_context;
      std::vector<std::pair<std::string, effect_cont>> effects;
      std::unordered_map<std::string, complex_func> creation_funcs;
      
      functions_container();
    };
    
//     struct operation {
//       uint32_t type;
//       enum functions_container::logic_operation logic_operation;
//       enum functions_container::stack_operation stack_operation;
//       union {
//         const functions_container::condition_cont* condition;
//         const functions_container::effect_cont* effect;
//         const functions_container::change_context_cont* context;
//       };
//       
//       functions_container::data_container data;
//     };
    
    using action_container = const functions_container;
    
    struct check_table_value {
      enum class type {
        bool_t,
        int_t,
        float_t,
        string_t,
        array_t,
      };
      
      enum flags_bits {
        value_required = (1 << 0),
        
      };
      
      const std::string_view key;
      const type value_type;
      const uint32_t flags;
      const uint32_t max_count;
      const std::initializer_list<check_table_value> nested_array_data;
    };
    
//     #define SAME_TYPE 1
//     #define DIFFERENT_TYPE 0
//     #define DONT_CARE 2
//     int check_sol_type(const sol::type &t, const sol::object &obj);
    void recursive_check(const std::string_view &id, const std::string_view &data_type, const sol::table &table, const check_table_value* current_check, const check_table_value* array_check, const size_t &size, size_t &counter);
    
    // думаю что мы вообще ничего не валидируем сначала, после окончания генерации сразу проверяем все объекты
    size_t add_event(const sol::table &table);
    bool validate_event(const sol::table &table); // только проверка на валидность
    bool validate_event_and_save(sol::this_state lua, const sol::table &table); // сохранить результат в сериализаторе
    void parse_event(core::event* event, const sol::table &table); // распарсить эвент
    
    std::string table_to_string(sol::this_state lua, const sol::table &table, const sol::table &keyallow);
    
    size_t add_building(const sol::table &table);
    bool validate_building(const size_t &index, const sol::table &table);
    bool validate_building_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_building(core::building_type* building_type, const sol::table &table);
    
    size_t add_city_type(const sol::table &table);
    bool validate_city_type(const size_t &index, const sol::table &table);
    bool validate_city_type_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_city_type(core::city_type* city_type, const sol::table &table);
    
    size_t add_city(const sol::table &table);
    bool validate_city(const size_t &index, const sol::table &table);
    bool validate_city_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_city(core::city* city, const sol::table &table);
    
    size_t add_province(const sol::table &table);
    size_t register_province();
    size_t register_provinces(const size_t &count);
    void set_province(const size_t &index, const sol::table &table);
    bool validate_province(const size_t &index, const sol::table &table);
    bool validate_province_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_province(core::province* province, const sol::table &table);
    
    size_t add_title(const sol::table &table);
    bool validate_title(const size_t &index, const sol::table &table);
    bool validate_title_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_title(core::titulus* title, const sol::table &table);
    
    // может сначало зарегистрировать персонажа
    // а потом присвоить табличку в индекс
    // нужно ли регистрировать сразу нескольких персонажей? это было бы полезно в том числе не только для персонажей
    size_t add_character(const sol::table &table);
    size_t register_character();
    size_t register_characters(const size_t &count);
    void set_character(const size_t &index, const sol::table &table);
    bool validate_character(const size_t &index, const sol::table &table);
    bool validate_character_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_character(core::character* character, const sol::table &table);
    void parse_character_goverment(core::character* character, const sol::table &table);
  }
}

// как то так выглядит, персонаж самая сложная часть
// нужно потом еще сделать систему правления (набор механик)
// теперь у нас более менее ясная картина того как это все выглядит
// теперь по идее мне всего хватает для того чтобы сделать первых персонажей
// и первый ход, мне не дают покоя константы в классе персонажа (размеры контейнеров)

#endif
