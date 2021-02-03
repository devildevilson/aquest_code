#ifndef STRING_CONTAINER_H
#define STRING_CONTAINER_H

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include "memory_pool.h"
#include "assert.h"
#include "parallel_hashmap/phmap.h"

// было бы неплохо не использовать эти вещи
// мне определенно дожен потребоваться только первый класс из присутствующих
// его я буду использовать для флагов, остальные вещи должны полагаться
// на конкретные указатели

namespace devils_engine {
  namespace utils {
    // это прежде всего будет использоваться для флагов
    // но нужно ли мне переделывать контейнер переменных?
    class general_string_container {
    public:
      std::string_view insert(const std::string &str);
    private:
      //std::unordered_set<std::string> container;
      phmap::flat_hash_set<std::string> container;
    };
    
    class sequential_string_container {
    public:
      sequential_string_container();
      ~sequential_string_container();
      std::pair<std::string_view, size_t> insert(const std::string &str);
      size_t get(const std::string_view &str) const;
      std::string_view get(const size_t &id) const;
    private:
      memory_pool<std::string, sizeof(std::string)*100> string_pool;
      std::vector<std::string*> ids;
      //std::unordered_map<std::string_view, size_t> container;
      phmap::flat_hash_map<std::string_view, size_t> container;
    };
    
    class numeric_string_container {
    public:
      ~numeric_string_container();
      std::string_view insert(const std::string &str, const size_t &data);
      size_t get(const std::string_view &str) const;
      std::string_view get(const size_t &data) const;
    private:
      memory_pool<std::string, sizeof(std::string)*100> string_pool;
      //std::unordered_map<size_t, std::string*> int_container;
      //std::unordered_map<std::string_view, std::pair<std::string*, size_t>> str_container;
      phmap::flat_hash_map<size_t, std::string*> int_container;
      phmap::flat_hash_map<std::string_view, std::pair<std::string*, size_t>> str_container;
    };
    
    class data_string_container {
    public:
      void insert(const std::string_view &str, const size_t &data);
      size_t get(const std::string_view &str) const;
    private:
      //std::unordered_map<std::string_view, size_t> container;
      phmap::flat_hash_map<std::string_view, size_t> container;
    };
    
    template <size_t N>
    class container_strings {
    public:
      size_t add_string(const size_t &index, const std::string &str) { ASSERT(index < N); container[index].push_back(str); return container[index].size()-1; }
      size_t register_string(const size_t &index) { ASSERT(index < N); container[index].emplace_back(); return container[index].size()-1; }
      size_t register_strings(const size_t &index, const size_t &count) { ASSERT(index < N); container[index].resize(container[index].size() + count); return container[index].size(); }
      void set_string(const size_t &index, const size_t &cont_index, const std::string &str) { ASSERT(index < N); assert(cont_index < container[index].size()); container[index][cont_index] = str; }
      const std::vector<std::string> & get_strings(const size_t &index) const { ASSERT(index < N); return container[index]; }
    private:
      std::array<std::vector<std::string>, N> container;
    };
  }
}

#endif
