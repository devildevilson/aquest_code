#include "id.h"

#include <iostream>
#include <cassert>

#include "string_container.h"

namespace devils_engine {
  namespace utils {
    void id::set_container(sequential_string_container* cont) {
      container = cont;
    }
    
    id id::get(const std::string_view &name) {
      if (container == nullptr) throw std::runtime_error("Trying to get id before setting up the container");
      
      const size_t index = container->get(name);
      if (index != SIZE_MAX) return id(index);
      const auto &data = container->insert(std::string(name));
      return id(data.second);
    }
    
    id::id() : m_id(SIZE_MAX) {}
    bool id::valid() const { return m_id != SIZE_MAX; }
    std::string_view id::name() const { return valid() ? container->get(m_id) : ""; }
    size_t id::num() const { return m_id; }
    id::id(const size_t &id) : m_id(id) {}
    
//     std::vector<std::string> id::names;
    sequential_string_container* id::container = nullptr;
    
    bool operator==(const id &a, const id &b) { return a.num() == b.num(); }
    bool operator!=(const id &a, const id &b) { return a.num() != b.num(); }
    bool operator> (const id &a, const id &b) { return a.num() >  b.num(); }
    bool operator< (const id &a, const id &b) { return a.num() <  b.num(); }
    bool operator>=(const id &a, const id &b) { return a.num() >= b.num(); }
    bool operator<=(const id &a, const id &b) { return a.num() <= b.num(); }
  }
}
