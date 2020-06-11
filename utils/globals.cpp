#include "globals.h"

namespace devils_engine {
  std::string global::root_directory() { return m_root_directory; }
  void global::set_root_directory(const std::string &path) { m_root_directory = path; }
  std::string global::m_root_directory;
}
