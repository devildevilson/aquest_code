#include "declare_structures_table.h"

#define MAKE_MAP_PAIR(name) std::make_pair(names[size_t(structure::name)], structure::name)

namespace devils_engine {
  namespace core {
    namespace structure_data {
      const std::string_view names[] = {
#define GAME_STRUCTURE_FUNC(val) #val,
      GAME_STRUCTURES_LIST
#undef GAME_STRUCTURE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, structure> map = {
#define GAME_STRUCTURE_FUNC(val) MAKE_MAP_PAIR(val),
        GAME_STRUCTURES_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == static_cast<size_t>(structure::count));
    }
  }
}
