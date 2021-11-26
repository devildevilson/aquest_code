#ifndef DEVILS_ENGINE_CORE_LAW_H
#define DEVILS_ENGINE_CORE_LAW_H

#include <string>
#include <array>
#include "stats.h"
#include "stat_modifier.h"
#include "declare_structures.h"
#include "realm_mechanics.h"
#include "script/header.h"

// преположительно закон меняет сразу несколько механик, это хорошо с учетом того что 
// у нас несколько бит занимает например отношение к однополой женитьбе, но плохо
// в том что для остального делать эту структуру непрактично
// не уверен что составные законы нужны, но стуктура с законами нужна для игрока
// ко всему прочему законы могут изменить статы (в пользу составных законов)

// но закон еще меняет механику, механика будет привязана к функции луа (не будет наверное)
// источники говорят что феодализм существовал в китае рядом с чиновниками
// и по умолчанию государство раздавало земли крестьянам и чиновникам,
// скопив много земель богатые китайцы видимо превращались в феодалов,
// тут у нас явно какой то закон о запрете наследования земли
// скорее всего законы будут заданы здесь
// есть ряд механик которые может делать игрок, например забирать титулы
// нужно чтобы именно эти механики у игрока забирались и передавались некоему органу 
// (совет, а если могущественный то парламент)
// описал очень много механик связанных с законами
// закон меняет механику лишь однажды, для того чтобы отменить закон нужно принять предыдущий закон
// аттрибуты перевычисляются каждый ход (?) 

// законы накладываются на какие то уже существующие институты
// видимо есть какой то базовый набор прав, нужно ли его как то запоминать? наверное как и в случае со статами будут дефолтные права и вычисленные
// можно ли закон отменить? было бы неплохо
// создание силы в государстве - это закон? нет, это скорее какое то скриптовое событие

namespace devils_engine {
  namespace core {
    struct law {
      static const structure s_type = structure::law;
      static const size_t max_stat_modifiers_count = 16;
      static const size_t max_power_rights_modifiers_count = 16;
      static const size_t max_state_rights_modifiers_count = 16;
      
      std::string id;
      std::string name_id; // у некоторых сущностей имена должны быть определены заранее (ну то есть мы наверное можем сгенерировать закон, но пока нет)
      std::string description_id;
      
      std::array<stat_modifier, max_stat_modifiers_count> attribs;                        // закон тоже может изменить аттрибуты
      std::array<core::power_rights::values, max_power_rights_modifiers_count> state_rights;
      std::array<core::power_rights::values, max_power_rights_modifiers_count> council_rights;
      std::array<core::power_rights::values, max_power_rights_modifiers_count> tribunal_rights;
      std::array<core::power_rights::values, max_power_rights_modifiers_count> assembly_rights;
      std::array<core::power_rights::values, max_power_rights_modifiers_count> clergy_rights;
      std::array<core::state_rights::values, max_state_rights_modifiers_count> secular_rights;
      script::condition potential; // вызываются от реалма?
      script::condition condition;
      
      law();
    };
  }
}

#endif
