#ifndef RENDER_MODE_CONTAINER_H
#define RENDER_MODE_CONTAINER_H

#include <functional>
#include <unordered_map>
#include <string>

namespace devils_engine {
  namespace render {
    namespace modes {
      enum values {
        // эти вещи тоже не помешает вывести, наверное нужно добавить возможность какие то луа функции определять на перезапись цветов
//         plates,
//         elevation,
//         temperature,
//         moisture,
        // эти типы - игровые, так или иначе они присутствуют всегда, 
        // не думаю что стоит их выводить куда то в луа (или стоит?)
        biome,
        cultures,
        culture_groups,
        religions,
        religion_groups,
        provinces,
        countries, // самые высокие титулы в этой провинции
        duchies, // должны быть дэюре и фактические
        kingdoms,
        empires,
        
        // снабжение, развитие, тех
        
        count
      };
    }
    
    using mode_func = std::function<void()>;
    using mode_container = std::array<mode_func, modes::count>;
    modes::values get_current_mode();
    void mode(const modes::values &mode);
  }
}

#endif
