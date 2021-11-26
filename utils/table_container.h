#ifndef TABLE_CONTAINER_H
#define TABLE_CONTAINER_H

#include "sol.h"
#include "assert.h"

#include <vector>
#include "bin/core_structures.h"

namespace devils_engine {
  namespace utils {
    namespace generator_table_container {
      enum class additional_data {
        image = uint32_t(core::structure::count),
        biome,
        heraldy,
        
        count
      };
    }
    
    template <size_t N>
    class table_container {
    public:
      size_t register_table(const size_t &type) {
        ASSERT(type < N);
        const size_t index = static_cast<size_t>(type);
        container[index].emplace_back();
        return container[index].size()-1;
      }
      
      size_t register_tables(const size_t &type, const size_t &count) {
        ASSERT(type < N);
        const size_t index = static_cast<size_t>(type);
        container[index].resize(container[index].size() + count);
        return container[index].size();
      }
      
      size_t add_table(const size_t &type, const sol::table &table) {
        ASSERT(type < N);
        const size_t index = static_cast<size_t>(type);
        container[index].push_back(table);
        return container[index].size()-1;
      }
      
      void set_table(const size_t &type, const size_t &table_index, const sol::table &table) {
        ASSERT(type < N);
        const size_t index = static_cast<size_t>(type);
        assert(table_index < container[index].size());
        container[index][table_index] = table;
      }
      
      const std::vector<sol::table> & get_tables(const size_t &type) const {
        ASSERT(type < N);
        const size_t index = static_cast<size_t>(type);
        return container[index];
      }
      
//       inline size_t register_table(const core::structure &type) {
//         ASSERT(type < core::structure::count);
//         const size_t index = static_cast<size_t>(type);
//         container[index].emplace_back();
//         return container[index].size()-1;
//       }
//       
//       inline size_t register_tables(const core::structure &type, const size_t &count) {
//         ASSERT(type < core::structure::count);
//         const size_t index = static_cast<size_t>(type);
//         container[index].resize(container[index].size() + count);
//         return container[index].size();
//       }
//       
//       inline size_t add_table(const core::structure &type, const sol::table &table) {
//         ASSERT(type < core::structure::count);
//         const size_t index = static_cast<size_t>(type);
//         container[index].push_back(table);
//         return container[index].size()-1;
//       }
//       
//       inline void set_table(const core::structure &type, const size_t &table_index, const sol::table &table) {
//         ASSERT(type < core::structure::count);
//         const size_t index = static_cast<size_t>(type);
//         assert(table_index < container[index].size());
//         container[index][table_index] = table;
//       }
//       
//       inline const std::vector<sol::table> & get_tables(const core::structure &type) const {
//         ASSERT(type < core::structure::count);
//         const size_t index = static_cast<size_t>(type);
//         return container[index];
//       }
      
//       inline size_t register_table(const additional_data &type) {
//         ASSERT(type < additional_data::count);
//         const size_t index = static_cast<size_t>(type);
//         container[index].emplace_back();
//         return container[index].size()-1;
//       }
//       
//       inline size_t register_tables(const additional_data &type, const size_t &count) {
//         ASSERT(type < additional_data::count);
//         const size_t index = static_cast<size_t>(type);
//         container[index].resize(container[index].size() + count);
//         return container[index].size();
//       }
//       
//       inline size_t add_table(const additional_data &type, const sol::table &table) {
//         ASSERT(type < additional_data::count);
//         const size_t index = static_cast<size_t>(type);
//         container[index].push_back(table);
//         return container[index].size()-1;
//       }
//       
//       inline void set_table(const additional_data &type, const size_t &table_index, const sol::table &table) {
//         ASSERT(type < additional_data::count);
//         const size_t index = static_cast<size_t>(type);
//         assert(table_index < container[index].size());
//         container[index][table_index] = table;
//       }
//       
//       inline const std::vector<sol::table> & get_tables(const additional_data &type) const {
//         ASSERT(type < additional_data::count);
//         const size_t index = static_cast<size_t>(type);
//         return container[index];
//       }
    private:
      std::array<std::vector<sol::table>, N> container;
    };
  }
}

#endif
