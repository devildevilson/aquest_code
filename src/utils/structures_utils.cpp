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
    
    void flags_container::clear_timed_flags() {
      for (auto itr = flags.begin(); itr != flags.end(); ++itr) {
        if (itr->second == SIZE_MAX) itr = flags.erase(itr);
      }
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
    
    void events_container::add_event(const core::event* event, event_data &&data) {
      if (auto itr = events.find(event); itr != events.end()) {
        itr->second = std::move(data);
        return;
      }
      
      events.emplace(event, std::move(data));
    }
    
    void events_container::remove_event(const core::event* event) {
      events.erase(event);
    }
    
    void modificators_container::update_turn() {
      for (auto itr = modificators.begin(); itr != modificators.end(); ++itr) {
        itr->second.turns_count -= itr->second.turns_count == SIZE_MAX ? 0 : 1;
        if (itr->second.turns_count == 0) itr = modificators.erase(itr);
      }
    }
    
    bool modificators_container::has_modificator(const core::modificator* modificator) const {
      return modificators.find(modificator) != modificators.end();
    }
    
    void modificators_container::add_modificator(const core::modificator* modificator, const modificator_data &data) {
      if (auto itr = modificators.find(modificator); itr != modificators.end()) {
        itr->second = data;
        return;
      }
      
      modificators.emplace(modificator, data);
    }
    
    void modificators_container::remove_modificator(const core::modificator* modificator) {
      modificators.erase(modificator);
    }
    
    void hooks_container::update_turn() {
      for (auto itr = hooks.begin(); itr != hooks.end(); ++itr) {
        itr->second.expries -= itr->second.expries == UINT32_MAX ? 0 : 1;
        if (itr->second.expries == 0) itr = hooks.erase(itr);
      }
    }
    
    bool hooks_container::has_hook(const core::character* c) const {
      return hooks.find(c) != hooks.end();
    }
    
    void hooks_container::add_hook(const core::character* c, const struct data &data) {
      if (auto itr = hooks.find(c); itr != hooks.end()) {
        itr->second = data;
        return;
      }
      
      hooks.emplace(c, data);
    }
    
    void hooks_container::remove_hook(const core::character* c) {
      hooks.erase(c);
    }
  }
}
