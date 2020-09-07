#include "generator_container.h"
#include "map.h"
#include <cstring>
#include <stdexcept>

namespace devils_engine {
  namespace map {
    namespace generator {
      container::container(const create_info &info) : 
        tile_type{info.tile_types.size(), new data_type[info.tile_types.size()], new float[core::map::hex_count_d(core::map::detail_level) * info.tile_types.size()]}, 
        tiles(core::map::hex_count_d(core::map::detail_level)),
        entities(info.entities_types.size())
      {
        ASSERT(info.tile_types.size() != 0);
        memcpy(tile_type.types, info.tile_types.data(), info.tile_types.size() * sizeof(data_type));
        memset(tile_type.data_container, 0, core::map::hex_count_d(core::map::detail_level) * info.tile_types.size() * sizeof(float));
        
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          tiles[i].data = &tile_type.data_container[i * info.tile_types.size()];
        }
        
        for (size_t i = 0; i < info.entities_types.size(); ++i) {
          entities[i].first.id = info.entities_types[i].first;
          entities[i].first.types_count = info.entities_types[i].second.size();
          entities[i].first.types = entities[i].first.types_count == 0 ? nullptr : new data_type[entities[i].first.types_count];
          if (entities[i].first.types != nullptr) memcpy(entities[i].first.types, info.entities_types[i].second.data(), entities[i].first.types_count * sizeof(data_type));
        }
      }
      
      container::~container() {
        delete [] tile_type.types;
        delete [] tile_type.data_container;
        
        for (size_t i = 0; i < entities.size(); ++i) {
          delete [] entities[i].first.types;
          for (size_t j = 0; j < entities[i].second.size(); ++j) {
            delete [] entities[i].second[j].data;
          }
        }
        
//         // скорее всего для персонажа нужно указать династию
//         // и удалять персонажей наверное будем только по династиям
//         // то есть если игровыми средствами до династии не добраться, то ее можно удалить
//         // в каких это случаях происходит?
//         for (auto c : characters) {
//           character_pool.destroy(c);
//         }
//         
//         // нужно ли удалять титулы? титул может никому не принадлежать или быть не создан (наверное это одно и тоже)
//         // если никому не принадлежит специальный титул, то что? наверное пусть остается, 
//         // но нужно будет проделать какие то дополнителльные действия чтобы вновь его получить
//         // к титулам еще необходимо быстро обращаться, использовать id и map?
//         for (auto t : titles) {
//           titulus_pool.destroy(t);
//         }
      }
      
      uint32_t container::add_entity(const uint32_t &type) {
        if (type == 0) throw std::runtime_error("Tiles count is constant value");
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        entities[final_type].second.push_back({{}, new float[entities[final_type].first.types_count]});
        
        if (type == 3) {
          province_neighbours.emplace_back();
        }
        
        return entities[final_type].second.size()-1;
      }
      
      void container::set_entity_count(const uint32_t &type, const uint32_t &size) {
        if (type == 0) throw std::runtime_error("Tiles count is constant value");
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        ASSERT(entities[final_type].second.size() == 0); // щас пока не понятно нужно ли копировать
        entities[final_type].second.resize(size, {{}, nullptr});
        for (size_t i = 0; i < entities[final_type].second.size(); ++i) {
          entities[final_type].second[i].data = new float[entities[final_type].first.types_count];
        }
        
        if (type == 3) {
          province_neighbours.resize(size);
        }
      }
      
      template <>
      void container::set_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const float &data) {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types[parameter_type] != data_type::float_t) throw std::runtime_error("Bad data type");
          
          tiles[index].data[parameter_type] = data;
          return;
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types[parameter_type] != data_type::float_t) throw std::runtime_error("Bad data type");
        
        entities[final_type].second[index].data[parameter_type] = data;
      }
      
      template <>
      void container::set_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const int32_t &data) {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types[parameter_type] != data_type::int_t) throw std::runtime_error("Bad data type");
          
          tiles[index].data[parameter_type] = glm::intBitsToFloat(data);
          return;
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types[parameter_type] != data_type::int_t) throw std::runtime_error("Bad data type");
        
        entities[final_type].second[index].data[parameter_type] = glm::intBitsToFloat(data);
      }
      
      template <>
      void container::set_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const uint32_t &data) {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types[parameter_type] != data_type::uint_t) throw std::runtime_error("Bad data type");
          
          tiles[index].data[parameter_type] = glm::uintBitsToFloat(data);
          return;
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types[parameter_type] != data_type::uint_t) throw std::runtime_error("Bad data type");
        
        entities[final_type].second[index].data[parameter_type] = glm::uintBitsToFloat(data);
      }
      
      template <>
      void container::set_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const glm::vec3 &data) {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types_count < parameter_type+3) throw std::runtime_error("Not enough memory for this type");
          if (tile_type.types[parameter_type+0] != data_type::float_t) throw std::runtime_error("Bad data type");
          if (tile_type.types[parameter_type+1] != data_type::float_t) throw std::runtime_error("Bad data type");
          if (tile_type.types[parameter_type+2] != data_type::float_t) throw std::runtime_error("Bad data type");
          
          tiles[index].data[parameter_type+0] = data.x;
          tiles[index].data[parameter_type+1] = data.y;
          tiles[index].data[parameter_type+2] = data.z;
          return;
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types_count < parameter_type+3) throw std::runtime_error("Not enough memory for this type");
        if (entities[final_type].first.types[parameter_type+0] != data_type::float_t) throw std::runtime_error("Bad data type");
        if (entities[final_type].first.types[parameter_type+1] != data_type::float_t) throw std::runtime_error("Bad data type");
        if (entities[final_type].first.types[parameter_type+2] != data_type::float_t) throw std::runtime_error("Bad data type");
        
        entities[final_type].second[index].data[parameter_type+0] = data.x;
        entities[final_type].second[index].data[parameter_type+1] = data.y;
        entities[final_type].second[index].data[parameter_type+2] = data.z;
      }
      
      template <>
      float container::get_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) const {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types[parameter_type] != data_type::float_t) throw std::runtime_error("Bad data type");
          
          return tiles[index].data[parameter_type];
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types[parameter_type] != data_type::float_t) throw std::runtime_error("Bad data type");
        
        return entities[final_type].second[index].data[parameter_type];
      }
      
      template <>
      int32_t container::get_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) const {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types[parameter_type] != data_type::int_t) throw std::runtime_error("Bad data type");
          
          return glm::floatBitsToInt(tiles[index].data[parameter_type]);
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types[parameter_type] != data_type::int_t) throw std::runtime_error("Bad data type");
        
        return glm::floatBitsToInt(entities[final_type].second[index].data[parameter_type]);
      }
      
      template <>
      uint32_t container::get_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) const {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types[parameter_type] != data_type::uint_t) throw std::runtime_error("Bad data type");
          
          return glm::floatBitsToUint(tiles[index].data[parameter_type]);
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types[parameter_type] != data_type::uint_t) throw std::runtime_error("Bad data type");
        
        return glm::floatBitsToUint(entities[final_type].second[index].data[parameter_type]);
      }
      
      template <>
      glm::vec3 container::get_data(const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) const {
        if (type == 0) {
          if (core::map::hex_count_d(core::map::detail_level) <= index) throw std::runtime_error("Bad entity index");
          if (tile_type.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
          if (tile_type.types_count < parameter_type+3) throw std::runtime_error("Not enough memory for this type");
          if (tile_type.types[parameter_type+0] != data_type::float_t) throw std::runtime_error("Bad data type");
          if (tile_type.types[parameter_type+1] != data_type::float_t) throw std::runtime_error("Bad data type");
          if (tile_type.types[parameter_type+2] != data_type::float_t) throw std::runtime_error("Bad data type");
          
          return glm::vec3(tiles[index].data[parameter_type+0], tiles[index].data[parameter_type+1], tiles[index].data[parameter_type+2]);
        }
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        if (entities[final_type].first.types_count <= parameter_type) throw std::runtime_error("Bad parameter type");
        if (entities[final_type].first.types_count < parameter_type+3) throw std::runtime_error("Not enough memory for this type");
        if (entities[final_type].first.types[parameter_type+0] != data_type::float_t) throw std::runtime_error("Bad data type");
        if (entities[final_type].first.types[parameter_type+1] != data_type::float_t) throw std::runtime_error("Bad data type");
        if (entities[final_type].first.types[parameter_type+2] != data_type::float_t) throw std::runtime_error("Bad data type");
        
        return glm::vec3(entities[final_type].second[index].data[parameter_type+0], entities[final_type].second[index].data[parameter_type+1], entities[final_type].second[index].data[parameter_type+2]);
      }
      
      void container::add_child(const uint32_t &type, const uint32_t &index, const uint32_t &child) {
        if (type == 0) throw std::runtime_error("Tiles dont have childs");
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        
        entities[final_type].second[index].childs.push_back(child);
      }
      
      const std::vector<uint32_t> & container::get_childs(const uint32_t &type, const uint32_t &index) const {
        if (type == 0) throw std::runtime_error("Tiles dont have childs");
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        
        return entities[final_type].second[index].childs;
      }
      
      std::vector<uint32_t> & container::get_childs(const uint32_t &type, const uint32_t &index) {
        if (type == 0) throw std::runtime_error("Tiles dont have childs");
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].second.size() <= index) throw std::runtime_error("Bad entity index");
        
        return entities[final_type].second[index].childs;
      }
      
      void container::add_province_neighbour(const uint32_t &index, const uint32_t &neighbour) {
        if (index >= province_neighbours.size()) throw std::runtime_error("Bad entity index");
        province_neighbours[index].push_back(neighbour);
      }
      
      const std::vector<uint32_t> & container::get_province_neighbours(const uint32_t &index) const {
        if (index >= province_neighbours.size()) throw std::runtime_error("Bad entity index");
        return province_neighbours[index];
      }
      
      std::vector<uint32_t> & container::get_province_neighbours(const uint32_t &index) {
        if (index >= province_neighbours.size()) throw std::runtime_error("Bad entity index");
        return province_neighbours[index];
      }
      
//       void container::set_name(const uint32_t &type, const uint32_t &index, const std::string &name) {
//         if (entities.size() <= type) throw std::runtime_error("Bad entity type");
//         if (entities[type].second.size() <= index) throw std::runtime_error("Bad entity index");
//         
//         entities[type].second[index].name = name;
//       }
//       
//       std::string container::get_name(const uint32_t &type, const uint32_t &index) const {
//         if (entities.size() <= type) throw std::runtime_error("Bad entity type");
//         if (entities[type].second.size() <= index) throw std::runtime_error("Bad entity index");
//         
//         return entities[type].second[index].name;
//       }
      
      size_t container::entities_count(const uint32_t &type) const {
        if (type == 0) return core::map::hex_count_d(core::map::detail_level);
        
        const uint32_t final_type = type-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        return entities[final_type].second.size();
      }
      
      data_type container::type(const uint32_t &entity, const uint32_t &property) const {
        if (entity == 0) {
          if (tile_type.types_count <= property) throw std::runtime_error("Bad property type");
          return tile_type.types[property];
        }
        
        const uint32_t final_type = entity-1;
        if (entities.size() <= final_type) throw std::runtime_error("Bad entity type");
        if (entities[final_type].first.types_count <= property) throw std::runtime_error("Bad property type");
        return entities[final_type].first.types[property];
      }
      
//       core::character* container::create_character() {
//         auto c = character_pool.create();
//         characters.push_back(c);
//         return c;
//       }
//       
//       core::titulus* container::create_titulus(const enum core::titulus::type &t) {
//         auto title = titulus_pool.create(t);
//         titles.push_back(title);
//         return title;
//       }
//       
//       core::titulus* container::create_titulus(const enum core::titulus::type &t, const uint32_t &count) {
//         auto title = titulus_pool.create(t, count);
//         titles.push_back(title);
//         return title;
//       }
//       
//       uint32_t container::get_title_index(const core::titulus* title) const {
//         if (title == nullptr) return UINT32_MAX;
//         
//         for (size_t i = 0; i < titles.size(); ++i) {
//           if (titles[i] == title) return i;
//         }
//         
//         throw std::runtime_error("Could not find title");
//         return UINT32_MAX;
//       }
//       
//       core::titulus* container::get_title(const uint32_t &index) const {
//         if (index >= titles.size()) throw std::runtime_error("Could not find title"); // ошибка ли? скорее да
//         return titles[index];
//       }
//       
//       void container::add_playable_character(core::character* character) {
//         auto itr = turn_characters.find(character);
//         if (itr == turn_characters.end()) {
//           itr = turn_characters.insert(std::make_pair(character, uint32_t(0))).first;
//         }
//         
//         ++itr->second;
//       }
//       
//       void container::remove_playable_character(core::character* character) {
//         auto itr = turn_characters.find(character);
//         if (itr == turn_characters.end()) return;
//         
//         --itr->second;
//         if (itr->second == 0) turn_characters.erase(itr);
//       }
//       
//       const std::unordered_map<core::character*, uint32_t> & container::get_turn_characters() const {
//         return turn_characters;
//       }
      
      size_t container::compute_memory_size() const {
        size_t mem = 0;
        
        mem += sizeof(*this);
        mem += tile_type.types_count * sizeof(data_type);
        mem += tile_type.types_count * core::map::hex_count_d(core::map::detail_level) * sizeof(float);
        mem += tiles.size() * sizeof(tiles[0]);
        
        mem += entities.size() * sizeof(entities[0]);
        for (size_t i = 0; i < entities.size(); ++i) {
          mem += entities[i].second.size() * (sizeof(entities[i].second[0]) + entities[i].first.types_count * sizeof(float)) + entities[i].first.types_count * sizeof(data_type);
        }
        
        return mem;
      }
    }
  }
}
