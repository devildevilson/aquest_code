#ifndef BATTLE_MAP_H
#define BATTLE_MAP_H

#include <cstdint>
#include "utils/utility.h"
#include "utils/bit_field.h"

namespace yavf {
  class Buffer;
  class Device;
  class DescriptorSet;
}

// несколько координатных систем: квадратная + гексагональная
// нужно ли для координатных систем делать отдельные типы координат?
// желательно конечно нет, мне что нужно правильно переводить из кубических в квадратные и наоборот
// кубические плохи тем что они единственные кто требуют 3 переменные + нам нужно чисто индексом тайла оперировать
// по идее положение тайла зависит от координаты, оно легко расчитывается по довольно простым формулам
// осталось только придумать быстрый способ перевода из координат в индекс и наоборот
// на боевой карте было бы неплохо подумать о переходах между биомами
// 3д объекты на карте? было бы неплохо, мне же еще делать осады, в этом случае 3д объекты (или замаскированные 2д)
// было бы неплохо добавить, с другой стороны если камеру запретить крутить, то полегче будет скрывать 2д объекты
// то есть нужно сделать орнамент на краях тайлов + текстурировать каждый тайл, тайлы должны быть визуально довольно большими
// содержать в себе отряд + должны быть четко видны текстуры + кустики/деревья/трава(?) + оформление границ тайлов
// но при том что тайлы должны быть большими, подъемы от тайла к тайлу должны быть мелкими, но частыми (наверное)

namespace devils_engine {
  namespace battle {
    struct map {
      enum class orientation {
        odd_flat,
        even_flat,
        odd_pointy,
        even_pointy
      };
      
      enum class coordinate_system {
        square,
        hexagonal
      };
      
      using cube_vec = glm::ivec3;
      using axial_vec = glm::ivec2;
      using square_vec = glm::ivec2;
      using double_vec = glm::ivec2;
      
      static axial_vec cube_to_axial(const cube_vec &cube) { return axial_vec(cube.x, cube.z); };
      static cube_vec axial_to_cube(const axial_vec &axial) { return cube_vec(axial.x, -axial.x-axial.y, axial.y); };
      
      static square_vec cube_to_oddr(const cube_vec &cube) { return square_vec(cube.x + (cube.z - (cube.z & 1)) / 2, cube.z); }
      static cube_vec oddr_to_cube(const square_vec &square) { 
        const int32_t x = square.x - (square.y - (square.y & 1)) / 2; 
        return cube_vec(x, -x-square.y, square.y);
      }
      
      static square_vec cube_to_evenr(const cube_vec &cube) { return square_vec(cube.x + (cube.z + (cube.z & 1)) / 2, cube.z); }
      static cube_vec evenr_to_cube(const square_vec &square) { 
        const int32_t x = square.x - (square.y + (square.y & 1)) / 2; 
        return cube_vec(x, -x-square.y, square.y);
      }
      
      static square_vec cube_to_oddq(const cube_vec &cube) { return square_vec(cube.x, cube.z + (cube.x - (cube.x & 1)) / 2); }
      static cube_vec oddq_to_cube(const square_vec &square) { 
        const int32_t z = square.y - (square.x - (square.x & 1)) / 2; 
        return cube_vec(square.x, -square.x-z, z);
      }
      
      static square_vec cube_to_evenq(const cube_vec &cube) { return square_vec(cube.x, cube.z + (cube.x + (cube.x & 1)) / 2); }
      static cube_vec evenq_to_cube(const square_vec &square) { 
        const int32_t z = square.y - (square.x + (square.x & 1)) / 2; 
        return cube_vec(square.x, -square.x-z, z);
      }
      
      static cube_vec doubleheight_to_cube(const double_vec &doubleh) {
        const int32_t z = (doubleh.y - doubleh.x) / 2;
        return cube_vec(doubleh.x, -doubleh.x-z, z);
      }
      
      static double_vec cube_to_doubleheight(const cube_vec &cube) {
        return double_vec(cube.x, 2 * cube.z + cube.x);
      }
      
      static cube_vec doublewidth_to_cube(const double_vec &doubleh) {
        const int32_t x = (doubleh.x - doubleh.y) / 2;
        return cube_vec(x, -x-doubleh.y, doubleh.y);
      }
      
      static double_vec cube_to_doublewidth(const cube_vec &cube) {
        return double_vec(2 * cube.x + cube.z, cube.z);
      }
      
      // ориентироваться будем на примерно 128х128
      uint32_t width;
      uint32_t height;
      utils::bit_field_32<1> type;
      uint32_t tiles_count;
      
      yavf::Device* device;
      
      // нужен наверное какой нибудь юниформ буфер, буфер оффсетов это не юниформ буфер? вряд ли
      // как проверять эти тайлы с фрустумом? (сфера = скорость, но тогда проверять каждый тайл?)
      // в циве 5 на самой большой карте используется 128×80 тайлов (10240 всего), это хорошее число
      // думаю что на этот предел и надо ориентироваться, другое дело что удастся ли уместить все отряды на такую карту
      // или придется ее расширить? с этим проблем быть особенно не должно, но в базовой игре нужно постараться уместить
      yavf::Buffer* tiles_buffer;
      yavf::Buffer* offsets_buffer;
      yavf::DescriptorSet* set;
      
      // как при отрисовке определить положение тайла? у нас есть базовый тайл в (0,0)
      // по координатам тайла мы должны посчитать положение
      
      struct create_info {
        yavf::Device* device;
      };
      map(const create_info &info);
      ~map();
      
      void create_tiles(const uint32_t &width, const uint32_t &height, const coordinate_system &system, const orientation &orient);
      
      inline bool is_square() const {
        return type.get(0);
      };
      
      inline bool is_flat() const {
        return type.get(1);
      };
      
      inline bool is_odd() const {
        return type.get(2);
      };
    };
  }
}

#endif
