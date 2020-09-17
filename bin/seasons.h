#ifndef SEASONS_H
#define SEASONS_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "map.h"
#include "render/shared_structures.h"

namespace devils_engine {
  namespace core {
    struct seasons {
      static const size_t maximum_biomes = (UINT8_MAX + 1);
      static const size_t maximum_seasons = 64;
      // с чего я взял что это вообще может сработать? сложение работает, но из сложения никак не получить обратно конкретные индексы
      // это не сработает, нужно в общем использовать целый чар для одного тайла
      static const size_t chars_per_biome = (UINT8_MAX + 1) / maximum_biomes;
      static_assert(chars_per_biome == 1);
      static const size_t data_count = map::hex_count_d(map::detail_level) * maximum_seasons * chars_per_biome;
      
      uint8_t current_season;
      uint8_t data[data_count];
      render::biome_data_t biomes[maximum_biomes];
      
      inline seasons() : current_season(0) { memset(data, 0, data_count); memset(biomes, 0, sizeof(biomes[0]) * maximum_biomes); }
      inline void set_current(const uint32_t &index) { if (index >= maximum_seasons) throw std::runtime_error("Bad season index"); current_season = index; }
      inline uint8_t get_tile_biome(const uint32_t &tile_index) const {
        if (tile_index >= map::hex_count_d(map::detail_level)) throw std::runtime_error("Bad tile index");
        const uint32_t current_index = tile_index * maximum_seasons;
        return data[current_index + current_season];
      }
      
      inline void set_tile_biome(const uint32_t &tile_index, const uint32_t &biome_index) {
        if (biome_index >= maximum_biomes) throw std::runtime_error("Bad biome index");
        if (tile_index >= map::hex_count_d(map::detail_level)) throw std::runtime_error("Bad tile index");
        const uint32_t current_index = tile_index * maximum_seasons;
        data[current_index + current_season] = uint8_t(biome_index);
      }
    };
    
    struct season_container_initializer { // вряд ли нужен
      seasons* ptr;
      
      inline season_container_initializer() : ptr(new seasons) {}
      inline ~season_container_initializer() { delete ptr; ptr = nullptr; }
    };
  }
}

#endif
