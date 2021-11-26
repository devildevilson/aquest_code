#ifndef RENDER_MODE_CONTAINER_H
#define RENDER_MODE_CONTAINER_H

#include <functional>
#include <string>
#include "parallel_hashmap/phmap.h"

// эти вещи тоже не помешает вывести, наверное нужно добавить возможность какие то луа функции определять на перезапись цветов
//         plates,
//         elevation,
//         temperature,
//         moisture,

// эти типы - игровые, так или иначе они присутствуют всегда, 
// не думаю что стоит их выводить куда то в луа (или стоит?)

#define RENDER_MODES_LIST \
  RENDER_MODE_FUNC(biome) \
  RENDER_MODE_FUNC(cultures) \
  RENDER_MODE_FUNC(culture_groups) \
  RENDER_MODE_FUNC(religions) \
  RENDER_MODE_FUNC(religion_groups) \
  RENDER_MODE_FUNC(provinces) \
  RENDER_MODE_FUNC(countries) /* самые высокие титулы в этой провинции */ \
  RENDER_MODE_FUNC(duchies_de_jure) \
  RENDER_MODE_FUNC(duchies_de_facto) \
  RENDER_MODE_FUNC(kingdoms_de_jure) \
  RENDER_MODE_FUNC(kingdoms_de_facto) \
  RENDER_MODE_FUNC(empires_de_jure) \
  RENDER_MODE_FUNC(empires_de_facto)
  
// снабжение, развитие, тех

namespace devils_engine {
  namespace render {
    namespace modes {
      enum values {
#define RENDER_MODE_FUNC(val) val,
        RENDER_MODES_LIST
#undef RENDER_MODE_FUNC
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    using mode_func = std::function<void()>;
    using mode_container = std::array<mode_func, modes::count>;
    modes::values get_current_mode();
    void mode(const modes::values &mode);
  }
}

#endif
