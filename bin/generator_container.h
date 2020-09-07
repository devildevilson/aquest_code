#ifndef GENERATOR_CONTAINER_H
#define GENERATOR_CONTAINER_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "utils/id.h"
#include "utils/block_allocator.h"
#include "utils/memory_pool.h"
#include "character.h"

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

namespace std {
  template <>
  struct hash<std::pair<devils_engine::core::character*, uint32_t>> {
    size_t operator() (const std::pair<devils_engine::core::character*, uint32_t> &a) const {
      return hash<devils_engine::core::character*>()(a.first);
    }
  };
}

namespace devils_engine {
  namespace map {
    namespace generator {
      enum class data_type {
        uint_t,
        int_t,
        float_t
      };
      
      struct entity_type {
        utils::id id;
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
        struct create_info {
          std::vector<data_type> tile_types;
          std::vector<std::pair<utils::id, std::vector<data_type>>> entities_types;
        };
        container(const create_info &info);
        ~container();
        
        uint32_t add_entity(const uint32_t &type);
        void set_entity_count(const uint32_t &type, const uint32_t &size);
        
        template <typename T>
        void set_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const T &data);
        template <typename T>
        T get_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) const;
        
        void add_child(const uint32_t &type, const uint32_t &index, const uint32_t &child);
        const std::vector<uint32_t> & get_childs(const uint32_t &type, const uint32_t &index) const;
        std::vector<uint32_t> & get_childs(const uint32_t &type, const uint32_t &index);
        
        void add_province_neighbour(const uint32_t &index, const uint32_t &neighbour);
        const std::vector<uint32_t> & get_province_neighbours(const uint32_t &index) const;
        std::vector<uint32_t> & get_province_neighbours(const uint32_t &index);
        
//         void set_name(const uint32_t &type, const uint32_t &index, const std::string &name);
//         std::string get_name(const uint32_t &type, const uint32_t &index) const;
        
        size_t entities_count(const uint32_t &type) const;
        data_type type(const uint32_t &entity, const uint32_t &property) const;
        
//         core::character* create_character();
//         core::titulus* create_titulus(const enum core::titulus::type &t);
//         core::titulus* create_titulus(const enum core::titulus::type &t, const uint32_t &count);
//         uint32_t get_title_index(const core::titulus* title) const;
//         core::titulus* get_title(const uint32_t &index) const;
//         
//         void add_playable_character(core::character* character);
//         void remove_playable_character(core::character* character);
//         const std::unordered_map<core::character*, uint32_t> & get_turn_characters() const;
        
        size_t compute_memory_size() const;
      private:
        struct tile_type tile_type;
        std::vector<tile_data> tiles;
        std::vector<std::pair<entity_type, std::vector<entity>>> entities;
        std::vector<std::vector<uint32_t>> province_neighbours;
        
        // нужно ли помнить всех персонажей? нужно, но наверное невозможно
        // как хранить историю? короче нужно просто проверить какого это будет
//         utils::memory_pool<core::character, sizeof(core::character)*5000> character_pool;
//         utils::memory_pool<core::titulus, sizeof(core::titulus)*5000> titulus_pool;
//         std::vector<core::character*> characters;
//         std::vector<core::titulus*> titles;
//         
//         std::unordered_map<core::character*, uint32_t> turn_characters;
//         
//         std::vector<std::string> string_stack;
        
        // block_allocator? мне кажется что врядли пригодится
      };
    }
  }
}

#endif
