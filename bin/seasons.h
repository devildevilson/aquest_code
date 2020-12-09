#ifndef SEASONS_H
#define SEASONS_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "map.h"
#include "render/shared_structures.h"
#include <vector>
#include "utils/sol.h"
#include "utils/table_container.h"
#include "utils/globals.h"

namespace devils_engine {
  namespace core {
    struct seasons {
      static const size_t maximum_biomes = (MAX_BIOMES_COUNT);
      static const size_t maximum_seasons = 64;
      // с чего я взял что это вообще может сработать? сложение работает, но из сложения никак не получить обратно конкретные индексы
      // это не сработает, нужно в общем использовать целый чар для одного тайла
      static const size_t chars_per_biome = (UINT8_MAX + 1) / maximum_biomes;
      static_assert(chars_per_biome == 1);
      static const size_t data_count = map::hex_count_d(map::detail_level) * maximum_seasons * chars_per_biome;
      static const size_t aligned_data_size = (data_count + 8 - 1) / 8 * 8;
      static_assert(data_count == aligned_data_size);
      
      static const size_t biome_data_size = sizeof(render::biome_data_t) * maximum_biomes;
      static const size_t aligned_biome_data_size = (biome_data_size + 8 - 1) / 8 * 8;
      static_assert(aligned_biome_data_size == biome_data_size);
      static const int32_t invalid_biome = UINT8_MAX;
      
      uint8_t current_season;
      uint8_t data[data_count];
      render::biome_data_t biomes[maximum_biomes];
      
      inline seasons() : current_season(0) { memset(data, invalid_biome, data_count); memset(biomes, 0, sizeof(biomes[0]) * maximum_biomes); }
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
      
      inline void create_biome(const uint32_t &biome_index, const render::biome_data_t &data) {
        if (biome_index >= maximum_biomes) throw std::runtime_error("Bad biome index");
        biomes[biome_index] = data;
      }
      
      inline uint32_t add_biome(const sol::table &table) {
        auto cont = global::get<utils::table_container>();
        if (cont->get_tables(utils::table_container::additional_data::biome).size() == maximum_biomes) throw std::runtime_error("Too many biomes");
        return cont->add_table(utils::table_container::additional_data::biome, table);
      }
    };
    
    struct season_container_initializer { // вряд ли нужен
      seasons* ptr;
      
      inline season_container_initializer() : ptr(new seasons) {}
      inline ~season_container_initializer() { delete ptr; ptr = nullptr; }
    };
    
    // мне нужно задать данные биомов
    // они как и все остальное задаются через луа таблицы
    // нужно их задать через тот же интерфейс как и картинки
    // но лучше сделать примерно такой же интерфейс как у seasons
//     struct loading_seasons {
//       uint8_t current_season;
//       uint8_t data[seasons::data_count];
//       std::vector<sol::table> biome_tables;
//     };
  }
}

#endif
