#ifndef DEVILS_ENGINE_CORE_SEASONS_H
#define DEVILS_ENGINE_CORE_SEASONS_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#include "utils/constexpr_funcs.h"

#include "map.h"
#include "render/shared_structures.h"
#include "utils/sol.h"


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
      static const size_t aligned_data_size = align_to(data_count, 8);
      static_assert(data_count == aligned_data_size);
      
      static const size_t biome_data_size = sizeof(render::biome_data_t) * maximum_biomes;
      static const size_t aligned_biome_data_size = align_to(biome_data_size, 8);
      static_assert(aligned_biome_data_size == biome_data_size);
      static const int32_t invalid_biome = UINT8_MAX;
      
      uint8_t seasons_count;
//       uint8_t biomes_count;
      uint8_t current_season;
      uint8_t data[data_count]; // было бы очень неплохо не выделять сразу всю память на это
      //render::biome_data_t biomes[maximum_biomes]; // можно убрать отсюда
      
      inline seasons() : seasons_count(0), current_season(0) { memset(data, invalid_biome, data_count * sizeof(data[0])); } // memset(biomes, 0, sizeof(biomes[0]) * maximum_biomes);
      inline void set_current(const uint32_t &index) { if (index >= maximum_seasons) throw std::runtime_error("Bad season index"); current_season = index; }
      inline uint8_t get_tile_biome(const uint32_t &season_index, const uint32_t &tile_index) const {
        if (season_index >= seasons_count) throw std::runtime_error("Bad season index");
        if (tile_index >= map::hex_count_d(map::detail_level)) throw std::runtime_error("Bad tile index");
        const uint32_t current_index = season_index * map::hex_count_d(map::detail_level) + tile_index;
        //const uint32_t current_index = tile_index * maximum_seasons;
        return data[current_index];
      }
      
      inline void set_tile_biome(const uint32_t &season_index, const uint32_t &tile_index, const uint32_t &biome_index) {
        if (season_index >= seasons_count) throw std::runtime_error("Bad season index");
        if (biome_index >= maximum_biomes) throw std::runtime_error("Bad biome index");
        if (tile_index >= map::hex_count_d(map::detail_level)) throw std::runtime_error("Bad tile index");
        
        const uint32_t current_index = season_index * map::hex_count_d(map::detail_level) + tile_index;
        //const uint32_t current_index = tile_index * maximum_seasons + current_season;
        data[current_index] = uint8_t(biome_index);
      }
      
//       inline void create_biome(const uint32_t &biome_index, const render::biome_data_t &data) {
//         // эта функция должна по идее запускаться только при заполнении данных
//         // а значит мы пока еще не знаем количество биомов, индекс меньше максимума и ладно
//         if (biome_index >= maximum_biomes) throw std::runtime_error("Bad biome index");
//         biomes[biome_index] = data;
//       }
      
//       inline uint32_t add_biome(const sol::table &table) {
//         // тут мы проверим правильно ли заполнена таблица, да наверное сразу ее пихнем в массив
//         // сразу распарсить биом мы не можем - у нас нет текстурок
//         const uint8_t index = biomes_count;
//         const bool ret = utils::validate_biome(index, table);
//         if (!ret) throw std::runtime_error("Invalid biome table");
//         
//         auto cont = global::get<utils::world_serializator>();
//         cont->add_data(utils::world_serializator::biome, );
//         if (cont->get_tables(type).size() >= maximum_biomes) throw std::runtime_error("Too many biomes");
//         
//         ++biomes_count;
//         
//         return cont->add_table(type, table);
//       }
      
      // биом создать нужно не здесь, но при этом требуется получить индексы
//       inline uint32_t allocate_biome() {
//         const uint32_t index = biomes_count;
//         ++biomes_count;
//         if (biomes_count > maximum_biomes) throw std::runtime_error("Too many biomes");
//         return index;
//       }
      
      inline uint8_t allocate_season() {
        const uint8_t index = seasons_count;
        ++seasons_count;
        if (seasons_count > maximum_seasons) throw std::runtime_error("Too many seasons");
        return index;
      }
      
      inline void validate() {
        if (seasons_count == 0) throw std::runtime_error("Seasons do not exist");
//         if (biomes_count == 0)  throw std::runtime_error("Biomes do not exist");
        
        for (uint8_t i = 0; i < seasons_count; ++i) {
          size_t counter = 0;
          for (uint32_t j = 0; j < map::hex_count_d(map::detail_level); ++j) {
            const uint32_t current_index = i * map::hex_count_d(map::detail_level) + j;
            counter += size_t(data[current_index] == invalid_biome); // data[current_index] >= biomes_count || // полная проверка будет чуть дальше вызова этой функции
          }
          
          if (counter != 0) throw std::runtime_error("Season " + std::to_string(i) + " has " + std::to_string(counter) + " tiles with invalid biome");
        }
      }
    };
    
//     struct season_container_initializer { // вряд ли нужен
//       seasons* ptr;
//       
//       inline season_container_initializer() : ptr(new seasons) {}
//       inline ~season_container_initializer() { delete ptr; ptr = nullptr; }
//     };
    
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
