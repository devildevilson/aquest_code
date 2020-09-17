#ifndef STRING_CONTAINER_H
#define STRING_CONTAINER_H

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include "memory_pool.h"

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
      std::unordered_set<std::string> container;
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
      std::unordered_map<std::string_view, size_t> container;
    };
    
    class numeric_string_container {
    public:
      std::string_view insert(const std::string &str, const size_t &data);
      size_t get(const std::string_view &str) const;
    private:
      std::unordered_map<std::string_view, std::pair<std::string, size_t>> container;
    };
    
    class data_string_container {
    public:
      void insert(const std::string_view &str, const size_t &data);
      size_t get(const std::string_view &str) const;
    private:
      std::unordered_map<std::string_view, size_t> container;
    };
  }
}

#endif
