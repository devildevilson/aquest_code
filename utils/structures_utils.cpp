#include "structures_utils.h"

namespace devils_engine {
  namespace utils {
    void flags_container::update_turn() {
      for (auto itr = flags.begin(); itr != flags.end(); ++itr) {
        itr->second -= itr->second == SIZE_MAX ? 0 : 1;
        if (itr->second == 0) itr = flags.erase(itr);
      }
    }
    
    bool flags_container::has_flag(const std::string_view &flag) const {
      return flags.find(flag) != flags.end();
    }
    
    void flags_container::add_flag(const std::string_view &flag, const size_t &turns) {
      const auto pair = flags.try_emplace(flag, turns);
      auto itr = pair.first;
      itr->second = turns;
    }
    
    void flags_container::remove_flag(const std::string_view &flag) {
      flags.erase(flag);
    }
    
    bool traits_container::has_trait(const core::trait* trait) const {
      return traits.find(trait) != traits.end();
    }
    
    void traits_container::add_trait(const core::trait* trait) {
      traits.emplace(trait);
    }
    
    void traits_container::remove_trait(const core::trait* trait) {
      traits.erase(trait);
    }
    
    bool events_container::has_event(const core::event* event) const {
      return events.find(event) != events.end();
    }
    
    void events_container::add_event(const core::event* event, const size_t &turns) {
      const auto pair = events.try_emplace(event, turns);
      auto itr = pair.first;
      itr->second = turns;
    }
    
    void events_container::remove_event(const core::event* event) {
      events.erase(event);
    }
    
    void modificators_container::update_turn() {
      for (auto itr = modificators.begin(); itr != modificators.end(); ++itr) {
        itr->second -= itr->second == SIZE_MAX ? 0 : 1;
        if (itr->second == 0) itr = modificators.erase(itr);
      }
    }
    
    bool modificators_container::has_modificator(const core::modificator* modificator) const {
      return modificators.find(modificator) != modificators.end();
    }
    
    void modificators_container::add_modificator(const core::modificator* modificator, const size_t &turns) {
      const auto pair = modificators.try_emplace(modificator, turns);
      auto itr = pair.first;
      itr->second = turns;
    }
    
    void modificators_container::remove_modificator(const core::modificator* modificator) {
      modificators.erase(modificator);
    }
  }
}
