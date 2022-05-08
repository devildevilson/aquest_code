#include "logging.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    time_log::time_log(const std::string &str) : p(std::chrono::steady_clock::now()), str(str) {}
    time_log::~time_log() {
      auto end = std::chrono::steady_clock::now() - p;
      auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
      std::cout << str << " took " << mcs << " mcs" << "\n";
    }
  }
}
