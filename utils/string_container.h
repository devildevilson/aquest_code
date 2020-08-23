#ifndef STRING_CONTAINER_H
#define STRING_CONTAINER_H

#include <string>
#include <unordered_set>
#include <unordered_map>

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
      inline std::string_view insert(const std::string &str) {
        auto itr = container.find(str);
        if (itr == container.end()) itr = container.insert(str).first;
        return std::string_view(*itr);
      }
    private:
      std::unordered_set<std::string> container;
    };
    
    class sequential_string_container {
    public:
      inline sequential_string_container() : current_index(0) {}
      inline std::pair<std::string_view, size_t> insert(const std::string &str) {
        auto itr = container.find(str);
        if (itr == container.end()) {
          const size_t data = current_index;
          ++current_index;
          std::pair<std::string, size_t> copy = std::make_pair(str, data);
          std::string_view strv = copy.first;
          itr = container.try_emplace(std::move(strv), std::move(copy)).first;
        }
        return std::make_pair(std::string_view(itr->first), itr->second.second);
      }
      
      inline size_t get(const std::string_view &str) const {
        auto itr = container.find(str);
        if (itr == container.end()) return SIZE_MAX;
        return itr->second.second;
      }
    private:
      size_t current_index;
      std::unordered_map<std::string_view, std::pair<std::string, size_t>> container;
    };
    
    class numeric_string_container {
    public:
      inline std::string_view insert(const std::string &str, const size_t &data) {
        auto itr = container.find(str);
        if (itr == container.end()) {
          std::pair<std::string, size_t> copy = std::make_pair(str, data);
          std::string_view strv = copy.first;
          itr = container.try_emplace(std::move(strv), std::move(copy)).first;
        }
        return std::string_view(itr->first);
      }
      
      inline size_t get(const std::string_view &str) const {
        auto itr = container.find(str);
        if (itr == container.end()) return SIZE_MAX;
        return itr->second.second;
      }
    private:
      std::unordered_map<std::string_view, std::pair<std::string, size_t>> container;
    };
    
    class data_string_container {
    public:
      inline void insert(const std::string_view &str, const size_t &data) {
        container[str] = data;
      }
      
      inline size_t get(const std::string_view &str) const {
        auto itr = container.find(str);
        if (itr == container.end()) return SIZE_MAX;
        return itr->second;
      }
    private:
      std::unordered_map<std::string_view, size_t> container;
    };
  }
}

#endif
