#include "globals.h"

namespace devils_engine {
  std::string global::root_directory() { return m_root_directory; }
  void global::set_root_directory(const std::string &path) { m_root_directory = path; }
  void global::initialize_state(const size_t &seed) {
    static_assert(utils::rng::next == utils::xoroshiro128starstar::rng);
    const size_t state1 = utils::splitmix64::rng(seed);
    const size_t state2 = utils::splitmix64::rng(state1);
    game_state.global_state = {
      utils::splitmix64::get_value(state1),
      utils::splitmix64::get_value(state2)
    };
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
