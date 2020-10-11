#ifndef SERIALIZATOR_HELPER_H
#define SERIALIZATOR_HELPER_H

#include <vector>
#include <array>
#include <string>
#include "bin/declare_structures.h"
#include "utility.h"
#include "bin/map.h"

namespace devils_engine {
  namespace core {
    struct seasons;
  }
  
  namespace utils {
    struct cista_serializator;
    
    // я только что понял что название мира может быть задано в utf8 строке
    // сделать его техническим довольно сложно без соответствующей помощи со стороны задающего
    // нужно сделать эти вещи отдельно
//     std::string make_technical(const std::string &s);
    
    // наверное какие то типы данных нужно будет отдельно сериализовывать
    // вообще я так понимаю что все типы в которых нет описания условий или эффектов
    // могут быть сериализованы обычным способом
    class world_serializator {
    public:
      static const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      static const size_t hash_size = 32;
      
      struct tile_data {
        float height;
        uint32_t biome;
        // распространение культур, религий, техов
      };
      
      static const size_t tiles_data_size = sizeof(tile_data) * core::map::hex_count_d(core::map::detail_level);
      static const size_t aligned_tiles_data_size = (tiles_data_size + 8 - 1) / 8 * 8;
      static_assert(tiles_data_size == aligned_tiles_data_size);
      
      struct container {
        std::vector<std::string> data;
      };
      
      world_serializator();
      ~world_serializator();
      void set_name(const std::string_view &name);
      void set_name(std::string &&name);
      void set_technical_name(const std::string_view &name);
      void set_technical_name(std::string &&name);
      void set_settings(const std::string_view &name);
      void set_settings(std::string &&name);
      void set_rand_seed(const uint32_t &seed);
      void set_noise_seed(const uint32_t &seed);
      void add_data(const core::structure &type, const std::string &data);
      void add_data(const core::structure &type, std::string &&data);
      void add_image_data(const std::string &data); // нужно видимо и с картинками примерно так же сделать как с основными данными
      void add_image_data(std::string &&data);      // то есть распределить их по типам 
      void set_world_matrix(const glm::mat4 &mat);
      void set_tile_data(const uint32_t &index, const tile_data &data);
      void copy_seasons(core::seasons* s);
      void serialize(); // нужно ли здесь задавать путь до файла?
      
      void deserialize(const std::string &path); // для того чтобы сделать интерфейс нужно делать для этого класса еще один который десериализует только строки
      std::string_view get_name() const;
      std::string_view get_technical_name() const;
      std::string_view get_settings() const;
      uint32_t get_rand_seed() const;
      uint32_t get_noise_seed() const;
      uint32_t get_data_count(const core::structure &type) const;
      std::string_view get_data(const core::structure &type, const uint32_t &index) const;
      uint32_t get_images_count() const;
      std::string_view get_image_data(const uint32_t &index) const;
      glm::mat4 get_world_matrix() const;
      tile_data get_tile_data(const uint32_t &index) const;
      void fill_seasons(core::seasons* s) const;
      const uint8_t* get_hash() const;
    private:
      std::string world_name; // этот и параметр ниже должны по идее показываться игроку
      std::string technical_name;
      std::string world_settings;
//       uint32_t seed;
//       glm::mat4 mat;
//       std::array<container, static_cast<size_t>(core::structure::count)> data_container;
//       std::array<tile_data, tiles_count> tiles;
      // календарь (календарь получим из глобала), климаты, сид
      
      uint8_t hash[hash_size];
      
      cista_serializator* ptr;
    };
  }
}

#endif
