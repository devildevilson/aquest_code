#ifndef LOGGING_H
#define LOGGING_H

#include <chrono>
#include <string>

namespace devils_engine {
  namespace utils {
    struct time_log {
      std::chrono::steady_clock::time_point p;
      std::string str;
      
      time_log(const std::string &str);
      ~time_log();
    };
  }
}

#endif
