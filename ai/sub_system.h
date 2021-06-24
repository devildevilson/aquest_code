#ifndef SUB_SYSTEM_H
#define SUB_SYSTEM_H

// у каждой подсистемы один набор характеристик
// эти характеристики нужны чтобы вычислять очередность и значимость вычислений
// + они должны взаимодействовать с какими то переменными у персонажа (веса и поведение)
// подсистема должна контролировать нагрузку, нагрузка зависит от "сложности" функции process
// ну и конечно нужно понять какие подсистемы можно запускать параллельно
// несколько типов подсистем: прежде всего строительство
// для кого то кроме персонажей должно это запускаться? вряд ли
// какие системы не будут треадсейф? скорее всего что то связанное с перемещением
// причем найти путь то поди можно в мультипотоке
// что еще? эвенты? в эвентах мы должны получать статы, треиты, модификаторы
// другое дело что нам могут прийти все эти вещи от эвентов у других персонажей
// и тут может случится гонка, нужно тогда добавить мьютекс всем персам

#include <cstdint>
#include "utils/bit_field.h"
// #include "utils/utility.h"
#include "core/declare_structures.h"

#define SUB_SYSTEM_SKIP UINT32_MAX

namespace devils_engine {
  namespace ai {
    class sub_system {
    public:
      enum attribs { // думал что аттрибутов будет много
        attrib_threadsave_check,
        attrib_threadsave_process,
        attrib_not_dead_characters, // для мертвых наверное вообще ничего не должно запускаться
        attrib_playable_characters,
        // наверное добавятся еще некоторые быстрые аттрибуты
      };
      
      virtual ~sub_system() = default;
      
      // тут мы должны проверить персонажа, нужно ли ему вычисляться в этой конкретной системе, наверное нужно вернуть какой нибудь код
      // эта функция должна запускаться параллельно, скорее всего должна как то влиять на веса у персонажа (хотя может и нет)
      // тут еще нас может интересовать генератор случайных чисел, причем видимо нужно как то синхронизовать с мультиплеером
      // синхронизация это такая тема которую я не знаю как делать, но думаю что если мы присобачим к персонажу некое число и будем его обновлять
      // тогда можно будет что то придумать с синхронизацией, для каждой подсистемы должны быть определены список переменных с которым она работает
      virtual uint32_t check(core::character* c) const = 0;
      // как контролировать нагрузку? мы должны определиться с тем как часто мы запускаем эту функцию
      // это по идее происходит в функции check: ее мы запускаем параллельно для всех персонажей, собираем ответ 
      virtual void process(core::character* c, const uint32_t &data) = 0;
      
      inline bool get_attrib(const uint32_t &attrib) const { ASSERT(attrib < 32); return attribs.get(attrib); }
    protected:
      utils::bit_field<1> attribs;
    };
  }
}

#endif
