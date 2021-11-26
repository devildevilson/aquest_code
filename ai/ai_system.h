#ifndef AI_SYSTEM_H
#define AI_SYSTEM_H

#include "sub_system.h"
#include "utils/typeless_container.h"
#include <vector>

namespace devils_engine {
  using ai::sub_system;
  namespace systems {
    class ai {
    public:
      inline ai(const size_t &size) : container(size, 8) {}
      inline ~ai() {
        for (auto s : sub_systems) {
          container.destroy(s);
        }
      }
      
      template <typename T, typename... Args>
      T* add(Args&& ...args) {
        auto ptr = container.create<T>(std::forward<Args>(args)...);
        sub_systems.push_back(ptr);
        return ptr;
      }
      
      inline const std::vector<sub_system*> & get_subsystems() const { return sub_systems; }
    private:
      utils::typeless_container container;
      std::vector<sub_system*> sub_systems;
    };
  }
}

#endif
