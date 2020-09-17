#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include <atomic>

namespace devils_engine {
  namespace utils {
    class sequential_string_container;
    class id {
    public:
      static void set_container(sequential_string_container* cont);
      static id get(const std::string_view &name);
      
      id();
      id(const id &a) = default;
      id(id &&a) = default;
      
      bool valid() const;
      std::string_view name() const;
      size_t num() const;
      
      id & operator=(const id &a) = default;
      id & operator=(id &&a) = default;
    private:
      size_t m_id;
      
      id(const size_t &id);
      
      //static std::atomic<size_t> current_id;
//       static std::vector<std::string> names;
      static sequential_string_container* container;
    };
    
    bool operator==(const id &a, const id &b);
    bool operator!=(const id &a, const id &b);
    bool operator>(const id &a, const id &b);
    bool operator<(const id &a, const id &b);
    bool operator>=(const id &a, const id &b);
    bool operator<=(const id &a, const id &b);
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
