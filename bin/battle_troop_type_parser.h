#ifndef BATTLE_TROOP_TYPE_PARSER_H
#define BATTLE_TROOP_TYPE_PARSER_H

#include "utils/sol.h"

namespace devils_engine {
  namespace battle {
    class context;
  }
  
  namespace utils {
    size_t add_battle_troop_type(const sol::table &table);
    bool validate_battle_troop_type(const uint32_t &index, const sol::table &table);
    void load_battle_troop_types(battle::context* ctx, const std::vector<sol::table> &tables);
  }
}

#endif
