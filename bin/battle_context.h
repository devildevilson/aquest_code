#ifndef BATTLE_CONTEXT_H
#define BATTLE_CONTEXT_H

#include <cstdint>
#include <cstddef>

// что тут должно быть? он должен быть похож на кор контекст, то есть тут должно быть описание всех объектов битвы
// юниты, отряды, типы юнитов отрядов, постройки, биомы (?), словом почти все кроме непосредственно данных тайла
// 

namespace devils_engine {
  namespace battle {
    class context {
    public:
      
    private:
      struct container {
        size_t count;
        void* memory;
      };
      
      // отряды и юниты - их не константное количество
      // но при этом очень редко они исчезают полностью
    };
  }
}

#endif
