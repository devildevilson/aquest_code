#ifndef DEVILS_ENGINE_CORE_STAT_MODIFIERS_H
#define DEVILS_ENGINE_CORE_STAT_MODIFIERS_H

#include <cstdint>
#include <cstddef>
#include "declare_structures.h"
#include "stats.h"

namespace devils_engine {
  namespace core {
    union stat_container {
      uint32_t uval;
      int32_t ival;
      float fval;
    };
    
    // не думаю что мне это пригодится
    struct bonus {
      float raw_add;
      float raw_mul;
      float fin_add;
      float fin_mul;
      
      inline bonus() : raw_add(0.0f), raw_mul(0.0f), fin_add(0.0f), fin_mul(0.0f) {}
    };
    
    // имеет смысл остановится на этом варианте, тут мы можем добавить еще пару указателей для типов бонусов
    // бонусы для типа отряда?
    struct stat_bonus {
      stat_type::values type;
      uint32_t stat;
      struct bonus bonus;
      
      inline stat_bonus() : type(stat_type::invalid), stat(UINT32_MAX) {}
      inline bool valid() const { return type != stat_type::invalid && stat != UINT32_MAX; }
      inline bool invalid() const { return type == stat_type::invalid || stat == UINT32_MAX; }
    };
    
    // что делать с бонусами к отношениям? отношения могут быть к челику, к религии, к некой группе
    // модификаторы отношений у которых указан таргет чаще всего задаются через скрипт
    struct opinion_modifier {
      uint32_t type; // тип берем откуда? тип в статах
      uint32_t helper; // на всякий случай
      // здесь добавим перечисления дополнительных данных которые нужны для модификатора
      union {
        struct character* character;
        struct realm* realm;
        struct religion* religion;
        struct religion_group* religion_group;
        struct culture* culture;
        struct culture_group* culture_group;
        struct city_type* city_type;
        struct dynasty* dynasty;
        // нужно ли делать ограничение на тип владения в игре?
        // хороший вопрос, вообще было бы неплохо
        struct holding_type* holding_type;
      };
      size_t token;
      //struct bonus bonus;
      float mod; // вообще тут скорее всего нужен инт, но числа тут будут небольшие (от -100 до 100)
      
      inline opinion_modifier() : type(opinion_modifiers::invalid), helper(UINT32_MAX), character(nullptr), token(SIZE_MAX), mod(0.0f) {}
      inline bool valid() const { return type != opinion_modifiers::invalid; }
      inline bool invalid() const { return type == opinion_modifiers::invalid; }
    };
    
    struct stat_modifier {
      stat_type::values type;
      uint32_t stat;
      //stat_container mod;
      float mod;
      
      inline stat_modifier() : type(stat_type::invalid), stat(UINT32_MAX), mod(0.0f) {}
      inline stat_modifier(const stat_type::values &type, const uint32_t &stat, const float &mod) : type(type), stat(stat), mod(mod) {}
      inline bool valid() const { return type != stat_type::invalid && stat != UINT32_MAX; }
      inline bool invalid() const { return type == stat_type::invalid || stat == UINT32_MAX; }
    };
    
    struct unit_stat_modifier {
      const troop_type* type;
      uint32_t stat;
      //stat_container mod;
      float mod;
      
      inline unit_stat_modifier() : type(nullptr), stat(UINT32_MAX), mod(0.0f) {}
      inline bool valid() const { return type != nullptr && stat != UINT32_MAX; }
      inline bool invalid() const { return type == nullptr || stat == UINT32_MAX; }
    };
  }
}

#endif
