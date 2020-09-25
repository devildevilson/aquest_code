#ifndef PROGRESS_CONTAINER_H
#define PROGRESS_CONTAINER_H

#include <vector>
#include <string>
#include <cstdint>
#include <atomic>
#include <mutex>

namespace devils_engine {
  namespace utils {
    class progress_container {
    public:
      enum type {
        creating_map,
        
        loading_map, // загрузка карты после создания?
        loading_created_map,
        loading_map_save,
        loading_battle,
        loading_encounter,
        
        back_to_menu,
      };
      
      progress_container();
      int64_t get_value() const;
      int64_t get_max_value() const;
      size_t get_type() const;
      std::string get_hint1() const;
      std::string get_hint2() const;
      std::string get_hint3() const;
      void advance();
      bool finished() const;
      bool reseted() const;
      
      void set_value(const int64_t &val);
      void set_max_value(const int64_t &val);
      void set_type(const size_t &type);
      void set_hint1(const std::string_view &str);
      void set_hint2(const std::string_view &str);
      void set_hint3(const std::string_view &str);
      void set_hint1(std::string &&str);
      void set_hint2(std::string &&str);
      void set_hint3(std::string &&str);
      void reset();
    private:
      mutable std::mutex mutex;
      int64_t current_value;
      int64_t max_value;
      size_t type;
      std::string hint1;
      std::string hint2;
      std::string hint3;
    };
  }
}

#endif
