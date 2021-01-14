#ifndef SHARED_BATTLE_STRUCTURES_H
#define SHARED_BATTLE_STRUCTURES_H

#ifdef __cplusplus

#include "utils/utility.h"
#include "shared_structures.h"

#define BATTLE_MAP_DESCRIPTOR_POOL_NAME "battle_map_descriptor_pool"
#define BATTLE_MAP_DESCRIPTOR_SET_LAYOUT_NAME "battle_map_descriptor_set_layout"

#define INLINE inline
#define INOUT

namespace devils_engine {
  namespace render {
    using uint = uint32_t;
    //using int = int32_t;
    static_assert(sizeof(int) == sizeof(int32_t));
    using ivec4 = glm::ivec4;
    using ivec3 = glm::ivec3;
    using ivec2 = glm::ivec2;
    using uvec4 = glm::uvec4;
    using uvec3 = glm::uvec3;
    using uvec2 = glm::uvec2;
    using  vec4 =  glm::vec4;
    using  vec3 =  glm::vec3;
    using  vec2 =  glm::vec2;
    using glm::uintBitsToFloat;
    using glm::intBitsToFloat;
    using glm::floatBitsToUint;
    using glm::floatBitsToInt;
#else

#define INLINE 
#define INOUT inout   

#endif
    
#define BATTLE_BIOMES_MAX_COUNT (0xff-1)
#define BATTLE_BIOME_INVALID 0xff
    
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
// мне бы еще оставлять трупы на тайле среди прочего
struct battle_map_tile_data_t {
  float height;
  image_t texture1; // "пол"
  image_t texture2; // стенки
  uint biome_index;
  
  //uint architecture[6];
};

struct packed_battle_map_tile_data_t {
  uvec4 data1;
  
  //uint architecture[6];
};

const uint battle_biome_objects_count = 3;

struct battle_biome_object_images_t {
  image_t face;
  image_t top;
};

// возможно тут было бы неплохо сделать побольше объектов и какое то смешивание между ними
struct battle_biome_data_t {
  battle_biome_object_images_t textures[battle_biome_objects_count];
  // я не оставляю попыток нарисовать низ дерева например, и тут на самом деле та же проблема
  // что и на глобальной карте, стволы деревьев почти не будут видны примерно половину времени
  // следовательно зачем они нужны, с другой стороны наверняка игрок захочет приблизить
  // и рассмотреть все это дело, для этого пригодится возможность изменить текстурку объекта
  // при каком то удалении, да думаю что так и нужно сделать
  
  float density;  // сколько всего объектов мы нарисуем на тайле
  // эти перменные отвечают за то какой объект с каким шансом появится среди всех объектов на тайле
  float probabilities[battle_biome_objects_count];
  vec2 scales[battle_biome_objects_count];
  // если текущий зум выше чем это то меняем текстурку и способ отрисовки? 
  // или способ отрисовки как то отдельно контролировать? смысла особого это не имеет
  // возможно стоит еще добавить какие нибудь частицы (начнем хотя бы с того что атака противника может потребовать частицы)
  float zooms[battle_biome_objects_count];
  float dummy;
  
  // было бы классно сюда еще добавить частицы какие нибудь
};

struct packed_battle_biome_data_t {
  vec4 data[5]; // sizeof(battle_biome_data_t)/sizeof(vec4)
};

INLINE battle_map_tile_data_t unpack_data(const packed_battle_map_tile_data_t packed_data) {
  battle_map_tile_data_t data;
  data.height = uintBitsToFloat(packed_data.data1.x);
  data.texture1.container = packed_data.data1.y;
  data.texture2.container = packed_data.data1.z;
  data.biome_index = packed_data.data1.w;
  return data;
}

INLINE battle_biome_data_t unpack_data(const packed_battle_biome_data_t packed_data) {
  battle_biome_data_t data;
  data.textures[0].face.container = floatBitsToUint(packed_data.data[0].x);
  data.textures[0].top.container  = floatBitsToUint(packed_data.data[0].y);
  data.textures[1].face.container = floatBitsToUint(packed_data.data[0].z);
  data.textures[1].top.container  = floatBitsToUint(packed_data.data[0].w);
  data.textures[2].face.container = floatBitsToUint(packed_data.data[1].x);
  data.textures[2].top.container  = floatBitsToUint(packed_data.data[1].y);
  data.density          = packed_data.data[1].z;
  data.probabilities[0] = packed_data.data[1].w;
  data.probabilities[1] = packed_data.data[2].x;
  data.probabilities[2] = packed_data.data[2].y;
  data.scales[0]        = vec2(packed_data.data[2].z, packed_data.data[2].w);
  data.scales[1]        = vec2(packed_data.data[3].x, packed_data.data[3].y);
  data.scales[2]        = vec2(packed_data.data[3].z, packed_data.data[3].w);
  data.zooms[0]         = packed_data.data[4].x;
  data.zooms[1]         = packed_data.data[4].y;
  data.zooms[2]         = packed_data.data[4].z;
  data.dummy            = packed_data.data[4].w;
  return data;
}
    
#ifdef __cplusplus

    static_assert(sizeof(battle_map_tile_data_t) == sizeof(packed_battle_map_tile_data_t));
    static_assert(sizeof(battle_biome_data_t) == sizeof(packed_battle_biome_data_t));
    //static_assert(BATTLE_BIOMES_MAX_COUNT == BATTLE_BIOME_INVALID);
    static_assert((UINT8_MAX+1) > BATTLE_BIOME_INVALID);
    static_assert((UINT8_MAX+1) > BATTLE_BIOMES_MAX_COUNT);
  }
}
#endif

#endif
