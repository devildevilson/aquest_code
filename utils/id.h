#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include <atomic>

namespace devils_engine {
  namespace utils {
    class id {
    public:
      static id get(const std::string &name);
      
      id();
      
      bool valid() const;
      std::string name() const;
      size_t num() const;
      
      bool operator==(const id &other) const;
      bool operator!=(const id &other) const;
      bool operator>(const id &other) const;
      bool operator<(const id &other) const;
      bool operator>=(const id &other) const;
      bool operator<=(const id &other) const;
    private:
      size_t m_id;
      
      id(const size_t &id);
      
      //static std::atomic<size_t> current_id;
      static std::vector<std::string> names;
    };
  }
}

namespace std {
  template<> 
  struct hash<devils_engine::utils::id> {
    std::size_t operator()(const devils_engine::utils::id& id) const noexcept {
      return id.num();
    }
  };
}

#endif
