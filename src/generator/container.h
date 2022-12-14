#ifndef GENERATOR_CONTAINER_H
#define GENERATOR_CONTAINER_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "utils/id.h"
#include "utils/block_allocator.h"
#include "utils/memory_pool.h"
//#include "character.h"
#include "utils/sol.h"
// нам где то нужно еще хранить названия сгенерированых сущностей
// то есть тут полный список данных которые нужно положить для сущностей

// нужно хранить детей, название, родителя + некоторое количество данных
// у самих тайлов немного полегче (по сути просто лежат данные)
// как получать тип? он привязан к определенному слоту в сущности

// как то так это выглядит, не очень пока понимаю к чему это я все
// но мне нужен способ дать возможность сделать любую карту
// либо основную часть перенести вообще в луа
// то есть пусть вся эта информация содержится в луа таблицах
// скорее всего в этом случае будет слишком большой расход памяти
// в демо генераторе при 150 разбиении (макс) 650 мб
// (у меня кажется разбиение 256 если сравнивать, и тогда памяти может быть и гиг)
// (бред)
// какая то тупиковая ситуация. расход памяти слишком большой по этим расчетам
// по идее вот так будет расход памяти поменьше

// когда генерировать и где хранить строки?
// со строками сложнее так как нужно скорее всего не просто строки сгенерировать
// но и локализовать их
// локализация конечно интересная штука для генератора

// namespace std {
//   template <>
//   struct hash<std::pair<devils_engine::core::character*, uint32_t>> {
//     size_t operator() (const std::pair<devils_engine::core::character*, uint32_t> &a) const {
//       return hash<devils_engine::core::character*>()(a.first);
//     }
//   };
// }

namespace devils_engine {
  namespace map {
    namespace generator {
      enum class data_type {
        uint_t,
        int_t,
        float_t,
        count
      };
      
      struct entity_type {
//         utils::id id;
        size_t types_count;
        data_type* types;
      };
      
      struct entity {
        //std::string name; 
        std::vector<uint32_t> childs;
        float* data;
      };
      
      struct tile_type {
        size_t types_count;
        data_type* types;
        float* data_container;
      };
      
      struct tile_data {
        float* data;
      };
      
      class container {
      public:
//         struct create_info {
//           std::vector<data_type> tile_types;
//           std::vector<std::pair<utils::id, std::vector<data_type>>> entities_types;
//         };
//         container(const create_info &info);
        container(const uint32_t &tiles_count);
        ~container();
        
        uint32_t add_entity(const uint32_t &type);
        void set_entity_count(const uint32_t &type, const uint32_t &size);
        void clear_entities(const uint32_t &type);
        
        template <typename T>
        void set_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const T &data);
        template <typename T>
        T get_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) const;
        
        uint32_t add_child(const uint32_t &type, const uint32_t &index, const uint32_t &child);
        uint32_t get_child(const uint32_t &type, const uint32_t &index, const uint32_t &array_index) const;
        uint32_t get_childs_count(const uint32_t &type, const uint32_t &index) const;
        const std::vector<uint32_t> & get_childs(const uint32_t &type, const uint32_t &index) const;
        std::vector<uint32_t> & get_childs(const uint32_t &type, const uint32_t &index);
        void clear_childs(const uint32_t &type, const uint32_t &index);
        
        size_t entities_count(const uint32_t &type) const;
        data_type type(const uint32_t &entity, const uint32_t &property) const;
        
        uint32_t add_entity_lua(const uint32_t &type);
        void set_entity_count_lua(const uint32_t &type, const uint32_t &size);
        void clear_entities_lua(const uint32_t &type);
        
        template <typename T>
        void set_data_lua(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const T &data);
        template <typename T>
        T get_data_lua(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) const;
        
        uint32_t add_child_lua(const uint32_t &type, const uint32_t &index, const uint32_t &child);
        uint32_t get_child_lua(const uint32_t &type, const uint32_t &index, const uint32_t &array_index) const;
        uint32_t get_childs_count_lua(const uint32_t &type, const uint32_t &index) const;
        const std::vector<uint32_t> & get_childs_lua(const uint32_t &type, const uint32_t &index) const;
        std::vector<uint32_t> & get_childs_lua(const uint32_t &type, const uint32_t &index);
        void clear_childs_lua(const uint32_t &type, const uint32_t &index);
        
        size_t entities_count_lua(const uint32_t &type) const;
        data_type type_lua(const uint32_t &entity, const uint32_t &property) const;
        
        size_t set_tile_template(const std::vector<data_type> &template_data);
        size_t set_entity_template(const std::vector<data_type> &template_data);
        
        size_t set_tile_template_lua(const sol::table &template_data);
        size_t set_entity_template_lua(const sol::table &template_data);
        
        size_t compute_memory_size() const;
      private:
        uint32_t tiles_count;
        struct tile_type tile_type;
        std::vector<tile_data> tiles;
        std::vector<std::pair<entity_type, std::vector<entity>>> entities;
//         std::vector<std::vector<uint32_t>> province_neighbours;
      };
    }
  }
}

#endif
