#ifndef BATTLE_UNIT_STATE_PARSER_H
#define BATTLE_UNIT_STATE_PARSER_H

#include "utils/sol.h"
#include <unordered_set>

namespace devils_engine {
  namespace battle {
    class context;
  }
  
  namespace utils {
    size_t add_battle_unit_state(const sol::table &table);
    bool validate_battle_unit_state(const uint32_t &index, const sol::table &table);
    void load_battle_unit_states(battle::context* ctx, const std::vector<sol::table> &tables, std::unordered_set<std::string> &parsed_scripts);
  }
}

#endif
