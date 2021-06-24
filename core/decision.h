#ifndef DECISION_H
#define DECISION_H

#include <string>
#include <cstdint>
#include "core/declare_structures.h"
#include "script/script_header.h"

namespace devils_engine {
  namespace core {
    namespace target_type {
      enum value {
        province,
        city,
        religion,
        culture,
        title,
        character,
        dynasty,
        realm,
        hero_troop,
        army,
        boolean,
        number,
        string
      };
    }
    
    // (например призвать ко двору нового аристократа, по аналогии с цк2)
    // тут какая то полугенерация, есть основные действия которые не изменяются, 
    // и есть действия сгенерированные становятся доступны после каких нибудь событий в игре
    // нужно проверить как в цк2 (реконкиста помоему одно из таких действий)
    // (нет, реконкиста привязана к культуре)
    // решения можно практически полностью описать в json (по аналогии с цк или европой)
    // есть блок контролирующий появление решения, блок условий и блок эффектов
    // соответственно мы можем задать множество функций на проверку всех этих условий
    // это позволит нам сгенерировать классные описания 
    // может быть еще вещью которая появляется по правой кнопке на персонаже или городе (тип decision)
    // в таких решениях целей может быть несколько (2, +1 в некоторых случаях)
    struct decision {
      enum class type {
        // правая кнопка (дипломатические?), мажорные, минорные, ???
        diplomatic,
        major,
        minor
      };
      
      static const structure s_type = structure::decision;
      std::string id;
      // как вернуть имя и описание по скрипту?
      std::string name_id;
      std::string description_id;
      enum type type; // какие типы?
      
      // было бы неплохо както сократить количество проверяемых decision
      
      // как задать входные данные?
      size_t input_count;
      std::array<script::script_data, 16> input_array;
      // это наверное будет храниться непосредственно в script::script_data
      script::script_data potential;
      script::script_data condition;
      script::script_data effect;
      
      decision();
      ~decision();
      
      // зачем она мне нужна? я хотел что бы я просто запомнил инпут где нибудь и переиспользовал его
      // но это не особ нужно, можно в функции вычислить все что нужно
      //void setup_input(const uint32_t &count, const script::script_data* &input); // функция для проверки входных данных
      
      bool check_shown_condition(const script::target &root, const script::target &helper) const;
      bool check_condition(const script::context &ctx) const;
      // я могу сохранить "скомпилированный" интерфейс, хотя тут не обязательно
      // нужно добавить еще итерацию по потентиал, там нужно указать два объекта 
      // (или больше? например для женитьбы так то нужно 3 объекта)
      void iterate_conditions(const script::context &ctx) const; // вставляем функцию луа
      void iterate_actions(const script::context &ctx) const;
      
      bool run(const script::context &ctx) const;
      
      std::string_view get_name(const script::context &ctx) const;
      std::string_view get_description(const script::context &ctx) const;
      
      bool check_ai(const script::context &ctx) const;
      bool run_ai(const script::context &ctx) const;
      
      // возвращаем корневого персонажа
      script::target check_input(const script::context &ctx) const;
    };
  }
}

#endif
