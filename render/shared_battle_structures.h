#ifndef SHARED_BATTLE_STRUCTURES_H
#define SHARED_BATTLE_STRUCTURES_H

#ifdef __cplusplus

#include "utils/utility.h"

#define BATTLE_MAP_DESCRIPTOR_SET_LAYOUT_NAME "battle_map_descriptor_set_layout"

namespace devils_engine {
  namespace render {
    using uint = uint32_t;
    //using int = int32_t;
    static_assert(sizeof(int) == sizeof(int32_t));
    using ivec4 = glm::ivec4;
    using ivec3 = glm::ivec3;
    using ivec2 = glm::ivec2;
    using vec4 = glm::vec4;
    using vec3 = glm::vec3;
    using vec2 = glm::vec2;
    using glm::uintBitsToFloat;
    using glm::intBitsToFloat;
    using glm::floatBitsToUint;
    using glm::floatBitsToInt;
#endif
    
    
// соседей тут указывать не нужно
// точки тут указывать не нужно (?)
// можно указать текстурки для каждой стороны (но пока что не вижу практического смысла)
// положение и точки должны вычисляться из положения тайла и типа карты
// тут по крайней мере нужно указать верхнюю текстурку и текстурку для стенок
// + доп данные тип биом, рендер каких то дополнительных штук (дороги, реки - это прям самые больные темы для меня)
// биом работает примерно по тем же правилам что и на карте мира, только возможно действительно придется 
// делать ствол + крону дерева отдельно, осады потребуют какого то оформления, нужно будет делать тайлы-стены,
// на этих тайлах должна быть расположена "ограда", наверное нужно сделать возможность нарисовать чего нибудь 6 штук
// относительно центра тайла + примерно то же самое на границах тайлов (плавные переходы от биома к биому)
// переходы от биома к биому как раз могут требовать разных картинок для стенок, нужно ли сделать пологость
// тайла отдельно (как в туториале)? это было бы неплохим решением проблемы переходов между разными высотами
struct battle_map_tile_data_t {
  uint texture1;
  uint texture2;
  uint biom_index;
  float height;
  
  uint architecture[6];
};
    
#ifdef __cplusplus
    
  }
}
#endif

#endif
