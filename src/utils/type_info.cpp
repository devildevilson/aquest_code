#include "type_info.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      std::atomic<size_t> type_id_counter::counter(0);
    }
  }
}
