#ifndef STRING_MAP_H
#define STRING_MAP_H

#include <cstddef>
#include <string>
#include <unordered_map>

// мы можем удалить всю эту мишуру после загрузки
// я пока что не вижу зачем нам это вообще может пригодиться во время игры (кроме вызова в консоле)
// но это довольно полезная вещь при загрузке
// поторопился: хранить это можно в контексте

namespace devils_engine {
  namespace utils {
    class string_map {
    public:
      inline size_t get(const std::string &id) {
        auto itr = map.find(id);
        if (itr == map.end()) {
          const size_t current = next_index;
          ++next_index;
          itr = map.insert(std::make_pair(id, current)).first;
        }
        
        return itr->second;
      }
    private:
      size_t next_index;
      std::unordered_map<std::string, size_t> map;
    };
  }
}

#endif
