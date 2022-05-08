#ifndef SERIALIZATOR_TYPES_H
#define SERIALIZATOR_TYPES_H

#include "cista/cista.h"
#include "bin/stats.h"

// вообще эти классы выглядят скорее как не нужные
// и на самом деле можно просто сразу парсить и добавлять в сериализатор строку
// видимо последнее, иначе луа разжирается до каких то космических размеров
// и так то боюсь что будут проблемы с портом на мобилу

namespace devils_engine {
  namespace serialize {
    namespace world {
      constexpr const auto MODE = cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY; // opt. versioning + check sum
      namespace data = cista::offset;
      
      struct province {
        uint32_t max_cities_count;
        data::string title_id;
        data::vector<uint32_t> neighbors;
        data::vector<uint32_t> tiles;
      };
      
      struct title {
        data::string id;
        data::string heraldy;
        data::string parent;
        uint32_t type;
        uint32_t main_color;
        uint32_t border_color1;
        uint32_t border_color2;
      };
      
      struct heraldy_layer {
        data::string id;
        data::string stencil;
        data::string next_layer;
        data::array<uint32_t, 4> colors;
        data::array<float, 4> coords;
        data::array<float, 4> tex_coords;
        bool discard_layer;
        bool continue_layer;
      };
      
      struct character {
        struct family {
          data::pair<uint32_t, uint32_t> parents;
          uint32_t owner;
          uint32_t consort;
          // dynasty
        };
        
        struct relations { // нужно ли это вообще?
          
        };
        
        uint32_t suzerain;
        uint32_t imprisoner;
        uint32_t liege;
        bool male;
        bool dead;
        struct family family;
        data::array<uint32_t, core::character_stats::count> stats;
        data::array<uint32_t, core::hero_stats::count> hero_stats;
        data::vector<data::string> titles;
        data::string religion;
        data::string hidden_religion;
      };
      
      struct building_type {
        data::string id; // важная характеристика
        data::vector<data::string> prerequisites;
        data::vector<data::string> limit_buildings;
        data::string replaced;
        data::string upgrades_from;
        uint64_t time; // важная характеристика
        float money_cost;
        float authority_cost;
        float esteem_cost;
        float influence_cost;
      };
      
      struct city {
        uint32_t province;
        uint32_t tile_index;
        data::string city_type;
        data::string title;
      };
      
      struct city_type {
        data::string id;
        data::vector<data::string> buildings;
        data::array<uint32_t, core::city_stats::count> stats;
        data::string image_top;
        data::string image_face;
        float scale;
      };
      
      struct biome {
        data::string id;
        
      };
    }
  }
}

#endif
