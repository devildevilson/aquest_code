#ifndef CONCURRENT_VECTOR_H
#define CONCURRENT_VECTOR_H

#include <mutex>
#include <vector>
#include "random_engine.h"

namespace devils_engine {
  namespace utils {
    template <typename T>
    struct concurrent_vector {
      static T default_value;
      std::mutex mutex;
      std::vector<T> vector;

      concurrent_vector() {}
      concurrent_vector(const size_t &size) : vector(size) {}

      void push_back(const T& obj) {
        std::unique_lock<std::mutex> lock(this->mutex);
        vector.push_back(obj);
      }

      T get_random(random_engine_st &engine) {
        std::unique_lock<std::mutex> lock(mutex);
        if (vector.size() == 0) return default_value;
        const size_t index = engine.index(vector.size());
        T obj = std::move(vector[index]);
        vector[index] = vector.back();
        vector.pop_back();
        return obj;
      }

      size_t size() {
        std::unique_lock<std::mutex> lock(mutex);
        return vector.size();
      }
    };
    
    template <typename T>
    T concurrent_vector<T>::default_value;
  }
}

#endif
