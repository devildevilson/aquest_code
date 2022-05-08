#include "type_info.h"

#include "utils/utility.h"
#include <string_view>

namespace devils_engine {
  namespace script {
    namespace internal {
      std::atomic<size_t> type_id_container::id(SCRIPT_FIRST_TYPE_ID);
      
      template<>
      size_t type_id_container::get_id<void>() { return 0; }
      
      template<>
      size_t type_id_container::get_id<bool>() { return 1; }
      
      template<>
      size_t type_id_container::get_id<double>() { return 2; }
      
      template<>
      size_t type_id_container::get_id<glm::vec4>() { return 3; }
      
      template<>
      size_t type_id_container::get_id<std::string_view>() { return 4; }
    }
  }
}
