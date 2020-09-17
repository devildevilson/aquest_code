#ifndef SERIALIZATOR_HELPER_H
#define SERIALIZATOR_HELPER_H

#include <vector>
#include <array>
#include <string>
#include "bin/declare_structures.h"
#include "utility.h"
#include "bin/map.h"

namespace devils_engine {
  namespace utils {
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
      
      struct container {
        std::vector<std::string> data;
      };
      
      world_serializator();
      void set_name(const std::string_view &name);
      void set_name(std::string &&name);
      void set_technical_name(const std::string_view &name);
      void set_technical_name(std::string &&name);
      void set_settings(const std::string_view &name);
      void set_settings(std::string &&name);
      void set_seed(const uint32_t &seed);
      void add_data(const core::structure &type, const std::string &data);
      void add_data(const core::structure &type, std::string &&data);
      void set_world_matrix(const glm::mat4 &mat);
      void set_tile_data(const uint32_t &index, const tile_data &data);
      void serialize(); // нужно ли здесь задавать путь до файла?
      
      void deserialize(const std::string &path); // для того чтобы сделать интерфейс нужно делать для этого класса еще один который десериализует только строки
      std::string_view get_name() const;
      std::string_view get_technical_name() const;
      std::string_view get_settings() const;
      uint32_t get_seed() const;
      uint32_t get_data_count(const core::structure &type) const;
      std::string_view get_data(const core::structure &type, const uint32_t &index) const;
      glm::mat4 get_world_matrix() const;
      tile_data get_tile_data(const uint32_t &index) const;
    private:
      std::string world_name; // этот и параметр ниже должны по идее показываться игроку
      std::string technical_name;
      std::string world_settings;
      uint32_t seed;
      glm::mat4 mat;
      std::array<container, static_cast<size_t>(core::structure::count)> data_container;
      std::array<tile_data, tiles_count> tiles;
      // календарь (календарь получим из глобала), климаты, сид
      
      char hash[hash_size];
    };
  }
}

#endif
