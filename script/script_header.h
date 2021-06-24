#ifndef SCRIPT_HEADER_H
#define SCRIPT_HEADER_H

#include <cstdint>
#include <string>
#include "utils/assert.h"
#include <vector>
#include "parallel_hashmap/phmap.h"
#include <array>
#include <functional>
#include "condition_commands_macro.h"
#include "action_commands_macro.h"
#include "script_block_commands_macro.h"
#include "utils/linear_rng.h"
#include "utils/sol.h"

namespace devils_engine {
  namespace script {
    // это всегда должна быть конкретная игровая сущность
    // на вход подавать script_data?
    struct target {
      uint32_t type;
      void* data;
    };
    
    // по идее в какой то такой структуре должны содержаться и команды и данные
    struct script_data {
//       union {
//         struct {
          // тут мы можем хранить тип команды (например сравнение)
          uint16_t command_type;
          // тут мы можем хранить тип значения которое к нам поступает, мне еще нужно указать тип сравнения, по 4 бита выделить?
          uint8_t number_type;
          uint8_t compare_type;
          // тут мы можем хранить непосредственно индекс команды (65000 команд должно хватить на все)
          uint16_t helper1;
          //float value;
          // некие специальные скомпилированные символы? тут можно указать дополнительный тип переменной (например умноженный инком)
          // тут будет храниться тип объекта, который лежит в дате
          uint16_t helper2;
//         };
        // здесь мы храним значение, либо количество элементов массива
        double value;
//       };
      // по идее строки не должно быть, строки должны быть переделаны в последовательность действий
      // которые мы выполним по отношению к объекту, с другой стороны строка может потребоваться дальше
      // как входные данные для функции (или собственно как сама функция)
      //std::string str_value;
      // по строке мы иногда можем указать конкртный объект (например титул)
      // но в строке должна содержаться команда, и то ее нужно будет переделать в скомпилированные действия
      // в строке может храниться флаг, со флагом особо ничего и не сделаешь
      // флаг то поди можно засунуть в массив по указателю ниже
      // и scope id тоже туда же
      
      // указатель по идее может хранить массив данных указанных в скрипте
      void* data;
      
      script_data();
      script_data(const script_data &copy) = delete;
      script_data(script_data &&move);
      script_data(const struct target &t);
      ~script_data();
      
      script_data & operator=(const script_data &copy) = delete;
      script_data & operator=(script_data &&move);
    };
    
    // стейт будет меняться каждый ход? да, наверное так будет лучше всего
    // нужно замешивать соль от персонажа, 128 стейт достаточно
    struct random_state {
      utils::xoroshiro128starstar::state s;
      
      uint64_t next();
    };
    
    // контекст по идее должен быть доступен и в обычных функциях
    struct context {
      std::vector<script_data> array_data;
      phmap::flat_hash_map<std::string, script_data> map_data;
      random_state* rnd;
      mutable sol::function* itr_func;
    };
    
    namespace command_type {
      enum value {
        invalid,
        condition,
        action,
        condition_script_block,
        action_script_block,
        object_function,
        // к скоупу мы обратимся через контекст, а может и нет
        scope,
        save_scope,
        scope_value,
      };
    }
    
    namespace number_type {
      enum value {
        boolean,
        number,
        string,
        object,
        array,
        get_scope,
        stat
      };
    }
    
    namespace number_compare_type {
      enum value {
        equal,
        not_equal,
        more,
        less,
        more_eq,
        less_eq,
      };
    }
    
    namespace number_update_type {
      enum value {
        add,
        diff
      };
    }
    
    namespace data_source_type {
      enum values {
        special,
        context, // тут достаточно указать только индекс или id переменной
        value,
        // поменяем местами это с тем что у нас находится в data_type
        // нам по всей видимости нужно пихнуть сюда все (ну или почти все) статы у персонажа
        // add_money, influence, esteem, authority
        stats_start,
        money, // текущее количество денег
        money_income,
        money_month_income,
        money_yearly_income,
        // все ли статы? вряд ли, ... хотя статов реально много можно сюда добавить
        // возможно будут нужны части статов например для пенальти
        // ну если статы будут названы уникально то почему бы и нет
        
        count
      };
    }
    
    namespace data_type {
      enum values {
        // по идее это вычисляется число, так что нужно просто добавить разный инком для каждого значения
        // comparisons, статическое число? нет, нужно еще variable:more:yearly_income:1.75
        // variable в принципе можно опустить, и наверное ключевое слово special тоже
        // more:число - такая запись должна приводить к тому что мы напрямую сравниваем числа
        // как добавлять отнимать income? variable:yearly_income:-1.75, через variable?
        // было бы наверное неплохо еще как то минимум указать? хотя это уже можно в скрипте задать
        // через строку я в принципе могу задать почти все что я хочу
        
        equal,
        not_equal,
        more,
        less,
        more_eq,
        less_eq,
        index,
      };
    }
    
    namespace condition_function {
      enum value {
  #define CONDITION_COMMAND_FUNC(name) name,
        CONDITION_COMMANDS_LIST
  #undef CONDITION_COMMAND_FUNC
        count
      };
    }
    
    namespace action_function {
      enum value {
#define ACTION_COMMAND_FUNC(name) name,
        ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC
        count
      };
    }
    
    // лучше все же разделить блок функции
    namespace block_function {
      enum value {
        count
      };
    }
    
    // есть хотя бы одна функция которая доступна в блоке экшонов
    namespace condition_block_function {
      enum value {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
      CONDITION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
        count
      };
    }
    
    namespace action_block_function {
      enum value {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
      ACTION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
        count
      };
    }
    
    // нужно сделать список строгой последовательности функций + инициализации
    // 
    
    // тут мы должны учесть еще контекст
    typedef bool (*condition_function_p) (const target &, const context &, const uint32_t &, const script_data*);
    typedef void (*action_function_p)    (const target &, const context &, const uint32_t &, const script_data*);
    typedef int32_t (*block_function_p)  (const target &, const context &, const uint32_t &, const script_data*);
    typedef void (*function_init_p)      (const uint32_t &, const sol::object &, script_data*); // инициализируем те данные которые мы будем отправлять в функцию
    
    extern const condition_function_p condition_functions[];
    extern const action_function_p action_functions[];
    extern const block_function_p condition_block_functions[];
    extern const block_function_p action_block_functions[];
    
    extern const function_init_p condition_init_functions[];
    extern const function_init_p action_init_functions[];
    extern const function_init_p condition_block_init_functions[];
    extern const function_init_p action_block_init_functions[];
    
    extern const std::string_view complex_var_regex; // "(\\w+):(\\w+):([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))"
    
    // в данных должны быть и таргеты
//     bool is_ai(const struct target &target, const uint32_t &count, const script_data* data) {
//       ASSERT(count == 1);
//       ASSERT(data[0].type == data_type::number);
//       ASSERT(data[0].data_type == number_type::boolean);
//       ASSERT(target.type == 1);
//       return target.data->is_ai() == bool(data[0].value);
//     }
//     
//     bool learning_diff(const struct target &target, const uint32_t &count, const script_data* data) {
//       ASSERT(count == 3);
//       ASSERT(data[0].type == data_type::target);
//       ASSERT(data[1].type == data_type::number);
//       ASSERT(data[2].type == data_type::number);
//       ASSERT(data[0].data_type == 1);
//       ASSERT(data[1].data_type == number_type::integer);
//       ASSERT(data[2].data_type == number_type::boolean);
//       //ASSERT(data[1].helper == number_compare_type::more);
//       
//       // стоп, а где тип сравнения? в helper?
//       switch (data[1].helper) {
//         case number_compare_type::more: {
//           
//           break;
//         }
//       }
//       
//       // тут обращаемся к данным двух тагретов
//       // вычитаем одно из другого
//       // сравниваем с data[1].value
//     }
//     
//     // any_faction_member должен генерировать цикл, а для этого нужны новые переменные
//     target any_faction_member(const struct target &target, const uint32_t &count, const script_data* data) {
//       ASSERT(count == 1);
//       // в этом случае нужно понимание что перед нами, по идее мы можем добавить это в хелпер
//       // тут может быть указано конкретное количество челиков которые мы должны обойти
//       // как обходить? по идее берем рандомных челиков по количеству или какую то часть
//       if (data[0].helper == 123) {
//         
//       }
//     }
//     
//     void money(const struct target &target, const uint32_t &count, const script_data* data) {
//       ASSERT(count == 1);
//       ASSERT(data[0].type == data_type::number);
//       ASSERT(data[0].data_type == number_type::integer);
//       
//       switch (data[1].helper) {
//         case number_update_type::add: {
//           target.data->add_stat(data[0].value);
//           break;
//         }
//       }
//     }
//     
//     void start_war(const struct target &target, const uint32_t &count, const script_data* data) {
//       // тут мы должны получить несколько аргументов + массив титулов
//       // как должен быть передан массив титулов? скорее всего мы должны создать массив 
//       // когда пытаемся получить титулы
//       // казус бели кажется тоже нужно создавать, а значит мы должны передать id
//       // таргет мы получим из контекста, клеймет - контекст, массив титулов - тоже несколько id
//       // хотя может и нет, вообще наверное можно и так и так
//       ASSERT(count == 4);
//       ASSERT(data[0].type == data_type::number);
//     }
//     
//     // смена таргета - это всегда рекурсивная функция, смена условия выхода - это рекурсивная функция
//     // if_condition - это скорее всего рекурсивная функция
//     // что такое array? по идее массив script_data + конкретные функции
//     // что такое data? это вектор + мап, из которых мы берем данные по командам
//     bool compute_condition(void* array, void* data, const bool current_bad, const struct target &current_target);
//     void compute_action(void* array, void* data, const struct target &current_target);
//     bool compute_condition_loop(void* array, void* data, const bool current_bad, const struct target &current_target);
//     void compute_action_loop(void* array, void* data, const struct target &current_target);
//     
//     // эта функция должна быть рекурсивна
//     void fire_decision() {
//       enum type {
//         cond,
//         change_target,
//         or_table,
//         end,
//         end_or
//       };
//       
//       struct data {
//         uint32_t type;
//       };
//       
//       std::array<script_data, 16> context;
//       std::array<script_data, 16> scope; 
//       // со скоупом не очень понятно как его собирать
//       // у нас по идее должна быть возможность запомнить всех any_faction_member
//       // тогда нужны инструменты к обходу контекста
//       // короче нужно реализовать несколько тестовых сценариев
//       // и много раз проводить тестирование
//       
//       // короче нужно сделать несколько рекурсивных функций, которые будут выступать вместо стека
//       // и нужно будет реализовать цикл, цикл по идее тоже можно через рекурсивные функции
//       // мне кажется что скорее всего в цк все сделано через рекурсию (???), хотя вряд ли
//       // короч рекурсивные функции какие? переход контекста, эти функции могут быть как и цикльными так и нет
//       // как использоваться в условных блоках, так и в блоках эффектов
//       // нужны видимо 4 разных типов рекурсивных функций
//       // теперь вопрос как передавать туда данные? хороший вопрос, не очень понимаю вообще что такое данные в этом контексте
//       // возможно нужно вводить vector и unordered_map, для разных типов данных
//       
//       size_t loop_index = SIZE_MAX;
//       size_t current_loop_index = 0;
//       //bool current_bad = false;
//       // все стеки можно заменить рекурсией
//       // думаю что так будет лучше
//       // с рекурсией как передать правильно собранный контекст
//       size_t stack_top = 0;
//       std::array<struct target, 16> stack;
//       // все же наверное нужно добавить стек для булеан значений
//       // нам нужно смешать 'или' и 'и' структуры, и придется видимо добавить стек current_bad
//       // нет, скорее только current_bad ...
//       size_t bool_stack_top = 0;
//       std::array<bool, 16> bool_stack;
//       bool_stack[bool_stack_top] = false;
//       std::vector<data> condition;
//       for (size_t i = 0; i < condition.size(); ++i) {
//         const target local_target = stack[stack_top];
//         const bool current_bad = bool_stack[bool_stack_top];
//         
//         switch (condition[i].type) {
//           case type::cond: {
//             script_data d;
//             const bool ret = is_ai(local_target, 1, &d);
//             // вообще тут до первого false, или если мы находимся в 'или' структуре, то до первого true
//             if (ret == current_bad) {
//               if (stack_top == 0) {} // выходим
//               else {} // ищем type::end
//             }
//             break;
//           }
//           
//           case type::or_table: {
//             ++bool_stack_top;
//             bool_stack[bool_stack_top] = true;
//             break;
//           }
//           
//           case type::change_target: {
//             const auto next_target = any_faction_member(local_target, 0, nullptr);
//             ++stack_top;
//             stack[stack_top] = next_target;
//             break;
//           }
//           
//           case type::end: {
//             ASSERT(stack_top != 0);
//             --stack_top;
//             if (loop_index != SIZE_MAX) {
//               i = loop_index-1;
//               ++current_loop_index;
//             }
//             break;
//           }
//           
//           case type::end_or: {
//             --bool_stack_top;
//             break;
//           }
//         }
//       }
//     }
  }
}

// функции отличается количеством входных данных, чаще всего это одна переменная
// если их несколько, то это сопоставление название и переменная
// как это удачно сделать в рамках сипипи, я думал просто набрать структур в юнион,
// но чет это слишком много мороки, нужно передавать массив пременных, 
// в массиве переменные будут лежать в определеном месте, так будет полегче

// как правильно все передать в функцию? есть несколько вещей которые мы должны получить из контекста
// остается вопрос только в одном: что делать со строками? как хранить команды?

// как все это делать в случае эвентов? для эвента нужно запомнить контекст
// контекст можно указать в табличке дополнительной

// нам нужно еще добавить условие 'или'

// я подумал вот что, мне к сожалению нужно сериализовать это дело верно, 
// сериализация происходит простым переводом написанного в строку
// лучше наверное сериализовать тогда через структуру, то есть собрать эвент доконца
// это конечно запарно

// короче для скриптов осталось: разделить по группам функции, добавить функции для каждого типа таргета,
// сохранение скоупа, сделать инициализацию функции, сделать сериализацию структуры, парсинг решения, ???
// кажется все, в принципе до конца разработки буду возиться с этим, дык еще и длс (что с ними?),
// хардкодить длс не хочется, но при этом новые функции нужны, 
// нужно придумать способ сериализовать без самоуничтожений при добавлении новых функций

#endif
