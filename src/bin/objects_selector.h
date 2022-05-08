#ifndef OBJECTS_SELECTOR_H
#define OBJECTS_SELECTOR_H

#include <cstddef>
#include <cstdint>
#include <vector>

// как теперь сделать выделение? выделение - это щелчок мыши по экрану
// рейкастинг (щелчок) + проверка боксов (выделение), 
// как сделать это в моем случае? мне нужно получить и в том и в другом случае 
// бокс, который будет проверять все объекты рядом, а дальше нужно 
// в центральном процессоре решить что к чему
// селектор можно наверное использовать во всех типах игровых состояний

namespace devils_engine {
  namespace core {
    struct army;
    struct hero_troop;
    struct character;
  }
  
  namespace utils {
    struct objects_selector {
      static const size_t maximum_selection = 256;
      
      struct unit {
        enum type {
          invalid, // ???
          army,
          hero,
//           character, // в принципе мне проще разделить тут вещи
          city, // город и структура наверное тоже должны быть в одной группе (наверное нет)
          structure,
          // ???
        };
        
        enum type type;
        uint32_t index;
        
        unit();
      };
      
      // надо бы как нибудь указать че к чему, типы выделений, например армия и герои
      unit units[maximum_selection]; // тут наверное будет разделение выделения войск (например на кораблях отдельно)
      uint32_t count;
      
      uint32_t clicks; // нужно посчитать количество кликов для выделения и проверить чтобы клики были в тот же тайл
      
      // тут в общем то будет только доступ к тому что выделенно
      // как бы сделать изящно для луа
      
      objects_selector();
      
      bool has(const uint32_t &index) const;
      //core::character* get(const uint32_t &index) const; // какие тут выделения еще будут?
      core::army* get_army(const uint32_t &index) const;
      core::hero_troop* get_troop(const uint32_t &index) const;
      void add(const enum unit::type type, const uint32_t &index);
      void clear();
      
      void copy(std::vector<unit> &copy_selector) const;
      std::vector<unit> copy() const;
    };
  }
}

#endif

