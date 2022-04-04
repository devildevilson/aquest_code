#ifndef BATTLE_TROOP_PARSER_H
#define BATTLE_TROOP_PARSER_H

#include "utils/sol.h"

namespace devils_engine {
  namespace battle {
    class context;
  }
  
  namespace utils {
    size_t add_battle_troop(const sol::table &table);
    bool validate_battle_troop(const uint32_t &index, const sol::table &table);
    void load_battle_troops(battle::context* ctx, const std::vector<sol::table> &tables);
  }
}

#endif
