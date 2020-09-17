#ifndef TABLE_CONTAINER_H
#define TABLE_CONTAINER_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <vector>
#include "bin/core_structures.h"

namespace devils_engine {
  namespace utils {
    class table_container {
    public:
      inline size_t register_table(const core::structure &type) {
        ASSERT(type < core::structure::count);
        const size_t index = static_cast<size_t>(type);
        container[index].emplace_back();
        return container[index].size()-1;
      }
      
      inline size_t register_tables(const core::structure &type, const size_t &count) {
        ASSERT(type < core::structure::count);
        const size_t index = static_cast<size_t>(type);
        container[index].resize(container[index].size() + count);
        return container[index].size();
      }
      
      inline size_t add_table(const core::structure &type, const sol::table &table) {
        ASSERT(type < core::structure::count);
        const size_t index = static_cast<size_t>(type);
        container[index].push_back(table);
        return container[index].size()-1;
      }
      
      inline void set_table(const core::structure &type, const size_t &table_index, const sol::table &table) {
        ASSERT(type < core::structure::count);
        const size_t index = static_cast<size_t>(type);
        assert(table_index < container[index].size());
        container[index][table_index] = table;
      }
      
      inline const std::vector<sol::table> & get_tables(const core::structure &type) const {
        ASSERT(type < core::structure::count);
        const size_t index = static_cast<size_t>(type);
        return container[index];
      }
    private:
      std::array<std::vector<sol::table>, static_cast<size_t>(core::structure::count)> container;
    };
  }
}

#endif