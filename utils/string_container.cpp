#include "string_container.h"
#include "assert.h"

namespace devils_engine {
  namespace utils {
    std::string_view general_string_container::insert(const std::string &str) {
      auto itr = container.find(str);
      if (itr == container.end()) itr = container.insert(str).first;
      return std::string_view(*itr);
    }
    
    sequential_string_container::sequential_string_container() {}
    sequential_string_container::~sequential_string_container() {
      for (auto ptr : ids) {
        string_pool.destroy(ptr);
      }
    }
    
    std::pair<std::string_view, size_t> sequential_string_container::insert(const std::string &str) {
      auto itr = container.find(str);
      if (itr == container.end()) {
        const size_t data = ids.size();
        auto ptr = string_pool.create(str);
        ids.emplace_back(ptr);
        std::string_view strv = *ptr;
        itr = container.try_emplace(strv, data).first;
      }
      return std::make_pair(itr->first, itr->second);
    }
    
    size_t sequential_string_container::get(const std::string_view &str) const {
      auto itr = container.find(str);
      if (itr == container.end()) return SIZE_MAX;
      return itr->second;
    }
    
    std::string_view sequential_string_container::get(const size_t &id) const {
      if (id >= ids.size()) return "";
      //if (id >= ids.size()) throw std::runtime_error("sdasffa");
      return *ids[id];
    }
    
    numeric_string_container::~numeric_string_container() {
      for (const auto &pair : int_container) {
        string_pool.destroy(pair.second);
      }
    }
    
    std::string_view numeric_string_container::insert(const std::string &str, const size_t &data) {
      auto itr = str_container.find(str);
      if (itr == str_container.end()) {
        auto ptr = string_pool.create(str);
        std::pair<std::string*, size_t> copy = std::make_pair(ptr, data);
        std::string_view strv = *ptr;
        itr = str_container.try_emplace(strv, copy).first;
        int_container.try_emplace(data, ptr);
      }
      
      return itr->first;
    }
    
    size_t numeric_string_container::get(const std::string_view &str) const {
      auto itr = str_container.find(str);
      if (itr == str_container.end()) return SIZE_MAX;
      return itr->second.second;
    }
    
    std::string_view numeric_string_container::get(const size_t &data) const {
      auto itr = int_container.find(data);
      if (itr == int_container.end()) return "";
      return *itr->second;
    }
    
    void data_string_container::insert(const std::string_view &str, const size_t &data) {
      ASSERT(data != SIZE_MAX);
      container[str] = data;
    }
    
    size_t data_string_container::get(const std::string_view &str) const {
//       if (str == "imperial14_title") {
//         std::cout << "size " << str.size() << "\n";
//         size_t counter = 0;
//         for (const auto &pair : container) {
//           if (pair.first.find("imp") == std::string_view::npos) continue;
//           std::cout << "id " << pair.first << "\n";
//           ++counter;
// //           if (counter >= 2000) break;
//         }
//         
//         std::cout << "\n";
//       }
      
      auto itr = container.find(str);
      if (itr == container.end()) return SIZE_MAX;
      return itr->second;
    }
  }
}
