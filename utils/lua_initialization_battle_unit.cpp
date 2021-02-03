#include "lua_initialization.h"

#include "bin/battle_structures.h"

namespace devils_engine {
  namespace utils {
    typedef double (battle::unit::*basic)();
    typedef double (battle::unit::*first_ptr) (const double &);
    typedef double (battle::unit::*second_ptr) (const double &, const double &);
    typedef void (battle::unit::*set_state_ptr) (const std::string_view &);
    
    void setup_lua_battle_unit(sol::state_view lua) {
      basic ptr1 = &battle::unit::random;
      first_ptr ptr2 = &battle::unit::random;
      second_ptr ptr3 = &battle::unit::random;
      set_state_ptr ptr4 = &battle::unit::set_state;
      
      auto battle = lua["battle"].get_or_create<sol::table>();
      auto type = battle.new_usertype<battle::unit>("unit", sol::no_constructor,
        "set_state", ptr4,
        "reset_timer", &battle::unit::reset_timer,
        "state", &battle::unit::state,
        "random", sol::overload(ptr1, ptr2, ptr3)
      );
    }
  }
}
