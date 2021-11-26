#ifndef DEVILS_ENGINE_CORE_TILE_H
#define DEVILS_ENGINE_CORE_TILE_H

#include "declare_structures.h"
#include "render/shared_structures.h"

namespace devils_engine {
  namespace core {
    struct tile {
      static const structure s_type = structure::tile;
      
      render::image_t texture;
      render::color_t color;
      float height;
      
      uint32_t center;
      uint32_t points[6];
      uint32_t neighbors[6];
      uint32_t borders_data;
      
      uint32_t province;
      uint32_t city;
      uint32_t struct_index;
      uint32_t biome_index;
      
      // к сожалению собирать 500к тайлов из хоста довольно долго, придется делать это в гпу
      
      // армии и герои где? возможно что и тут, армий будет откровенно немного
      // с учетом видимости, так что возможно имеет смысл сделать вычисления в одном потоке
      // но вот поиск пути то наверное стоит вынести в отдельный?
      
      // атомарность?
      size_t army_token; // токен? скорее всего теперь тут будет храниться токен
      size_t hero_token;
      // или фиг с ним? вообще нет, хотя армии если только свои ходят в этот ход
      // кажется я сам себя передумал, но может быть вообще вот какая ситуация:
      // 2 (или даже больше) армии двигались примерно по одному маршруту, 
      // в конечной точке кто то стоит, следовательно армия не может встать на это место,
      // одна из армий встает на место предыдущее, вторая армия должна где то найти место
      // и тип можно сделать чтобы армия вставала когда нибудь близко, или на несколько
      // тайлов назад по пути, может быть лучше хранить то куда армия по итогу хочет придти?
      // короче в пошаговой игре не нужно никаких особых телодвижений предпринимать если
      // часть пути до конца занята, мы просто останавливаемся где придется
      // тогда нужно ли делать так чтобы армии приходили в некую область?
      // это скорее всего зависит от интерфейса
      size_t movement_token; // чей?
      
      // темплейт, будет ли он вообще когда нибудь нужен? вряд ли
      tile();
      
      uint32_t neighbors_count() const;
    };
  }
}

#endif
