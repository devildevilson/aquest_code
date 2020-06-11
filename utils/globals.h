#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

namespace devils_engine {
  class global {
  public:
    template <typename T>
    static T* get(T* ptr = nullptr) {
      static T* cont = nullptr;
      if (ptr != nullptr) cont = ptr;
      return cont;
    }

    static std::string root_directory();
    void set_root_directory(const std::string &path);
  private:
    static std::string m_root_directory;
  };
}

#endif
