#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <cstddef>
#include "linear_rng.h"

namespace devils_engine {
  class global {
  public:
    struct state {
      utils::rng::state global_state;
      
      void initialize_state(const size_t seed);
      size_t advance_state();
    };
    
    template <typename T>
    static T* get(T* ptr = nullptr) {
      static T* cont = nullptr;
      if (ptr != nullptr) cont = ptr;
      if (reinterpret_cast<size_t>(ptr) == SIZE_MAX) cont = nullptr;
      return cont;
    }

    static std::string root_directory();
    void set_root_directory(const std::string &path);
    void initialize_state(const size_t &seed);
    static size_t advance_state();
    static utils::rng::state get_state();
    void set_state(const utils::rng::state &state); // наверное пригодится когда я буду делать сохранение загрузку
  private:
    static std::string m_root_directory;
    static state game_state;
  };
}

#endif
