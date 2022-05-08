#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <cstddef>
#include "linear_rng.h"
#include <atomic>

namespace devils_engine {
  class global {
  public:
    using state = utils::xoshiro256starstar::state;
    
    template <typename T>
    static T* get(T* ptr = nullptr) {
      static T* cont = nullptr;
      if (ptr != nullptr) cont = ptr;
      if (reinterpret_cast<size_t>(ptr) == SIZE_MAX) cont = nullptr;
      return cont;
    }
    
    static size_t gen_id();
    static void set_id_gen(const size_t &cur);
    static std::string root_directory();
    void set_root_directory(const std::string &path);
    void initialize_state(const uint64_t &seed);
    static uint64_t advance_state();
    static state get_state();
    void set_state(const state &state); // наверное пригодится когда я буду делать сохранение загрузку
  private:
    static std::string m_root_directory;
    static state game_state;
    static std::atomic<size_t> val;
  };
}

#endif
