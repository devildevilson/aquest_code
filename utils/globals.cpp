#include "globals.h"

namespace devils_engine {
//   namespace utils {
//     static inline rng::state seed_state(const uint64_t seed) {
//       rng::state new_state;
//       splitmix64::state splitmix_states[rng::state_size];
//       splitmix_states[0] = splitmix64::rng({seed});
//       for (uint32_t i = 1; i < rng::state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
//       for (uint32_t i = 0; i < rng::state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
//       return new_state;
//     }
//   }
  
  using namespace utils::xoshiro256starstar;
  
  size_t global::gen_id() { return val.fetch_add(1); }
  void global::set_id_gen(const size_t &cur) { val = cur; }
  std::string global::root_directory() { return m_root_directory; }
  void global::set_root_directory(const std::string &path) { m_root_directory = path; }
  void global::initialize_state(const uint64_t &seed) {
    game_state = init(seed);
  }
  
  uint64_t global::advance_state() {
    game_state = rng(game_state);
    return get_value(game_state);
  }
  
  global::state global::get_state() {
    return game_state;
  }
  
  void global::set_state(const global::state &state) {
    game_state = state;
  }
  
  std::string global::m_root_directory;
  global::state global::game_state;
  std::atomic<size_t> global::val(0);
}
