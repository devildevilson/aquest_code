#ifndef SCRIPT_HEADER_H
#define SCRIPT_HEADER_H

#include <cstdint>
#include <string>
#include <array>
#include <functional>
#include <vector>

#include "utils/assert.h"
#include "core/stats.h"
#include "parallel_hashmap/phmap.h"
#include "condition_commands_macro.h"
#include "action_commands_macro.h"
#include "script_block_commands_macro.h"
#include "utils/linear_rng.h"
#include "utils/sol.h"
#include "target.h"

#define FALSE_BLOCK 0
#define TRUE_BLOCK 1
#define IGNORE_BLOCK 2

namespace devils_engine {
  namespace core {
    struct decision;
  }
  
  namespace script {
    namespace command_type {
      enum values {
        invalid,
        condition,
        action,
        condition_script_block,
        action_script_block,
        general_script_block,
        object_function,
//         complex_variable,
        rvalue,
        // к скоупу мы обратимся через контекст, а может и нет
        scope,
        save_scope,
        scope_value,
      };
    }
    
    namespace number_type {
      enum values {
        boolean,
        number,
        object,
        stat,
        array,
        lvalue,
        lvalue_array,
        string_view,
        string,
        get_scope,
      };
    }
    
    namespace number_compare_type {
      enum values {
        equal,
        not_equal,
        more,
        less,
        more_eq,
        less_eq,
      };
    }
    
    namespace number_update_type {
      enum values {
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
      script_data(const struct target_t &t);
      script_data(const std::string_view &str);
      script_data(const double &value, const uint8_t &compare_type = number_compare_type::more_eq);
      ~script_data();
      
      script_data & operator=(const script_data &copy) = delete;
      script_data & operator=(script_data &&move);
      script_data & operator=(const struct target_t &t);
      script_data & operator=(const std::string_view &str);
      script_data & operator=(const double &value);
      
      void clear();
    };
    
    // стейт будет меняться каждый ход? да, наверное так будет лучше всего
    // нужно замешивать соль от персонажа, 128 стейт достаточно
    struct random_state {
      utils::xoroshiro128starstar::state s;
      
      uint64_t next();
    };
    
    // контекст по идее должен быть доступен и в обычных функциях
    struct context {
      // это нужно убрать, и убрать соответственно все обращения к массиву (массив нинужен, все усложняет)
      //std::vector<script_data> array_data;
      target_t root;
      phmap::flat_hash_map<std::string, script_data> map_data;
      random_state* rnd;
      sol::function* itr_func;
      // хотя все равно не понимаю зачем стак, нужен только prev
      //std::vector<target> targets_stack;
      target_t prev;
      script_data current_value;
      size_t nest_level;
      
      context();
    };
    
    struct turn_off_function {
      context* ctx;
      sol::function* f;
      
      inline turn_off_function(context* ctx) noexcept : ctx(ctx), f(nullptr) { std::swap(ctx->itr_func, f); }
      inline ~turn_off_function() noexcept { std::swap(ctx->itr_func, f); }
    };
    
    struct change_prev_target {
      context* ctx;
      target_t mem;
      
      inline change_prev_target(context* ctx, const target_t &current) noexcept : ctx(ctx), mem(ctx->prev) { ctx->prev = current; }
      inline ~change_prev_target() noexcept { ctx->prev = mem; }
    };
    
    struct increase_nesting {
      context* ctx;
      
      inline increase_nesting(context* ctx) noexcept : ctx(ctx) { ++ctx->nest_level; }
      inline ~increase_nesting() noexcept { --ctx->nest_level; }
    };
    
    namespace condition_function {
      enum values {
#define CONDITION_COMMAND_FUNC(name) name,
        CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC
  
#define STAT_FUNC(name) name,
#define CHARACTER_PENALTY_STAT_FUNC(name) name##_penalty,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

        count
      };
    }
    
    namespace action_function {
      enum values {
#define ACTION_COMMAND_FUNC(name) name,
        ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC

#define STAT_FUNC(name) add_##name,
#define CHARACTER_PENALTY_STAT_FUNC(name) add_##name##_penalty,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

        count
      };
    }
    
    // лучше все же разделить блок функции
    namespace general_block_function {
      enum values {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
        SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
        count
      };
    }
    
    // есть хотя бы одна функция которая доступна в блоке экшонов
    namespace condition_block_function {
      enum values {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
      CONDITION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
        count
      };
    }
    
    namespace action_block_function {
      enum values {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
      ACTION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
        count
      };
    }
    
    // нужно сделать список строгой последовательности функций + инициализации
    // 
    
    // зачем я сделал константный контекст? с учетом того что мне нужно в нем что то постоянно менять (например стек объектов)
    typedef bool (*condition_function_p)        (const target_t&, context*, const uint32_t&, const script_data*);
    typedef void (*action_function_p)           (const target_t&, context*, const uint32_t&, const script_data*);
    typedef int32_t (*block_function_p)         (const target_t&, context*, const uint32_t&, const script_data*);
    typedef int32_t (*general_block_function_p) (const target_t&, context*, const uint32_t&, const script_data*, const uint32_t&);
    typedef void (*function_init_p)             (const uint32_t&, const sol::object&, script_data*); // инициализируем те данные которые мы будем отправлять в функцию
    typedef void (*general_function_init_p)     (const uint32_t&, const sol::object&, script_data*, const uint32_t&);
    
    extern const condition_function_p condition_functions[];
    extern const action_function_p action_functions[];
    extern const block_function_p condition_block_functions[];
    extern const block_function_p action_block_functions[];
    extern const general_block_function_p general_block_functions[];
    
    extern const function_init_p condition_init_functions[];
    extern const function_init_p action_init_functions[];
    extern const function_init_p condition_block_init_functions[];
    extern const function_init_p action_block_init_functions[];
    extern const general_function_init_p general_block_init_functions[];
    
    extern const std::string_view complex_var_regex; // "(\\w+):(\\w+):([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))"
    extern const std::string_view dot_matcher;
    extern const std::string_view colon_matcher;
    extern const std::string_view number_matcher;
    
    // нужно добавить парочку утилити функций для того чтобы удобнее пользоваться было скриптами
    // например инициализация
    void init_condition(const uint32_t &target_type, const sol::object &obj, script_data* data);
    void init_action(const uint32_t &target_type, const sol::object &obj, script_data* data);
    void init_string_from_script(const uint32_t &target_type, const sol::object &obj, script_data* data);
    void init_number_from_script(const uint32_t &target_type, const sol::object &obj, script_data* data);
    
    template <typename T>
    target_t make_target(T* t) {
      return {static_cast<uint32_t>(T::s_type), t};
    }
    
    std::string_view get_string_from_script(const target_t &t, context* ctx, const script_data* data);
    double get_number_from_script(const target_t &t, context* ctx, const script_data* data);
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

// нужно еще сделать функции объекта, то есть ряд вещей которые возвращают что то полезное в качестве лефтвалуе
// для этого нужно более серьезно парсить строки, например у нас может быть строка "context:attacker.top_liege"
// или "prev.faith", что мне нужно для этого? все таки стек, проверять строку я должен не через реджекс
// (точнее мне достаточно проверить наличие символов ':' и '.'), функции объекта через точку, 
// нужно ли добавлять возможность указать входные данные как в функцию? кое что в любом случае нужно будет указать,
// но нужно минимизировать такие вещи, например у меня явно будет что то вроде "character.stats.military"
// по большому счету это означает еще ряд дополнительных функций, и придется видимо немного переписать 
// текстовую составляющую скрипта
// в этом случае у нас есть и lvalue и rvalue, и сделать бы и то и другое
// rvalue должно лежать первым перед возможным кондишеном, у конкретной функции может быть lvalue
// может ли быть пара rvalue и lvalue одновременно? так можно было бы сделать например запоминание скоупа
// например вот так "'scope:attacker' = 'prev.leader'", в принципе само собой у нас появилось разделение
// на скоуп и функции объекта (скоуп через :, функции объекта через .) в чем прикол функции объекта?
// они возвращают некую унифицированную структуру (script_data меня пока что устраивает)

#endif
