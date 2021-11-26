#ifndef DEVILS_ENGINE_UTILS_LUA_CONTAINER_ITERATORS_H
#define DEVILS_ENGINE_UTILS_LUA_CONTAINER_ITERATORS_H

#include "sol.h"
#include "structures_utils.h"

namespace devils_engine {
  namespace core {
    struct event;
    struct modificator;
  }
  
  namespace utils {
    template <typename T>
    auto flags_iterator(const T* self) {
      auto itr = self->flags.begin();
      return [self, itr] (sol::this_state s) mutable {
        if (itr == self->flags.end()) return sol::object(sol::nil);
        const auto current = itr;
        ++itr;
        return sol::make_object(s, current->second);
      };
    }
    
    template <typename T>
    auto events_iterator(const T* self) {
      auto itr = self->events.begin();
      return [self, itr] () mutable -> std::tuple<const core::event*, size_t> {
        if (itr == self->events.end()) return std::make_tuple(nullptr, 0);
        const auto current = itr;
        ++itr;
        return std::make_tuple(current->first, current->second.mtth);
      };
    }
    
    template <typename T>
    auto modificators_iterator(const T* self) {
      auto itr = self->modificators.begin();
      return [self, itr] () mutable -> std::tuple<const core::modificator*, const utils::modificators_container::modificator_data*> {
        if (itr == self->modificators.end()) return std::make_tuple(nullptr, nullptr);
        const auto &ptr = *itr;
        ++itr;
        return std::make_tuple(ptr.first, &ptr.second);
      };
    }
    
    template <typename T>
    auto traits_iterator(const T* self) {
      auto itr = self->traits.begin();
      return [self, itr] () mutable -> const core::trait* {
        if (itr == self->traits.end()) return nullptr;
        const auto ptr = *itr;
        ++itr;
        return ptr;
      };
    }
  }
}

#endif
