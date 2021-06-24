#ifndef HERO_TROOP_H
#define HERO_TROOP_H

#include <array>
#include "declare_structures.h"
#include "utils/structures_utils.h"
#include "ai/path_finding_data.h"

namespace devils_engine {
  namespace core {
    // это герой размещенный на карте (по идее мы должны создать эту структуру для каждого героя)
    // объявил себя героем - получи структуру, анимации передвижения я так понимаю будут только у игрока, 
    // следовательно наверное мне не нужно делать много специальных вещей для этого, компы наверное сразу будут прыгать
    // к последней точке, с другой стороны было бы неплохо сделать возможность почекать как был сделан ход
    struct hero_troop : public ai::path_finding_data, public utils::flags_container, public utils::modificators_container, public utils::events_container {
      static const structure s_type = structure::hero_troop;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      // как определить это число я пока не понимаю
      // слишком много слотов делать не нужно иначе получится слишком много микроменеджмента
      // число слотов должно быть около 7 (как в героях)
      static const size_t max_game_party_size = 8; // думаю что 8 - это то что нужно, входит ли непосредсвенно лидер партии в это число? скорее всего да
      // нужно определиться с интеллектом, по идее интеллект влияет на силу заклинаний, но я бы хотел чтобы еще по нему увеличивалось количество людей в партии
      // думаю что можно сделать так: средние значения взрослого здорового человека это по 10 каждой характеристики
      // и если одна характиристика достигает 20 то персонаж может объявить себя героем
      // характеристика 40 - это максимум,                (может быть все же 15)
      // соответственно 10 интеллекста это размер пати 3, 12 (13?) - размер пати 4, 20 - 5, 25 - 6, 30 - 7, 35 - 8
      // пусть строго каждые пять интеллекта +1 к размеру пати, для того чтобы объявить себя героем с 20 интеллектом
      // персонаж должен быть практикующим магом
      
      // отряд (отряд у героя состоит из персонажей, некоторые из них сгенерированные (копейщик, нанятый в городе))
      uint32_t party_size;
      // видимо у персонажей должен быть по умолчанию средний интеллект (какой?)
      // и по ходу дела персонаж может либо улучшить его либо ухудшить
      // таким образом уменьшив или увеличив размер партии
      uint32_t max_party_size; // я бы хотел добавить возможность прокачать характеристику "размер отряда героя"
      std::array<character*, max_game_party_size> party;
      
      // переменные для анимации: нужна наверное одна float переменная для конкретной позиции (анимации только для игрока?) 
      float current_pos;
      // позиция
      uint32_t tile_index;
      uint32_t army_gpu_slot; // это обязательный аттрибут любой армии или героя, нужно где то сделать поиск в обратную сторону
      // статы
      
//       phmap::flat_hash_map<const modificator*, size_t> modificators;
//       phmap::flat_hash_map<const event*, event_container> events;
//       phmap::flat_hash_map<std::string_view, size_t> flags;
//       modificators_container<modificators_container_size> modificators;
//       events_container<events_container_size> events;
//       flags_container<flags_container_size> flags;
      
      hero_troop();
      ~hero_troop();
    };
  }
}

#endif
