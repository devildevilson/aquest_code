#include "globals.h"

namespace devils_engine {
  namespace utils {
    static inline rng::state seed_state(const uint64_t seed) {
      rng::state new_state;
      splitmix64::state splitmix_states[rng::state_size];
      splitmix_states[0] = splitmix64::rng({seed});
      for (uint32_t i = 1; i < rng::state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
      for (uint32_t i = 0; i < rng::state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
      return new_state;
    }
  }
  
  std::string global::root_directory() { return m_root_directory; }
  void global::set_root_directory(const std::string &path) { m_root_directory = path; }
  void global::initialize_state(const size_t &seed) {
    game_state.global_state = utils::seed_state(seed);
  }
  
  size_t global::advance_state() {
    game_state.global_state = utils::rng::next(game_state.global_state);
    return utils::rng::value(game_state.global_state);
  }
  
  utils::rng::state global::get_state() {
    return game_state.global_state;
  }
  
  void global::set_state(const utils::rng::state &state) {
    game_state.global_state = state;
  }
  
  std::string global::m_root_directory;
  global::state global::game_state;
}
