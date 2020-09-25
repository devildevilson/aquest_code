#include "progress_container.h"

namespace devils_engine {
  namespace utils {
    progress_container::progress_container() : current_value(0), max_value(0) {}
//     void progress_container::advance_progress() { ++progress; }
//     int64_t progress_container::current_progress() const { return progress; }
//     int64_t progress_container::steps_count() const { return hints.size(); }
//     std::string_view progress_container::hint() const { return hints[progress]; }
//     bool progress_container::is_finished() const { return progress == hints.size(); }
//     void progress_container::set_progress(const int64_t &val) { progress = val; }
//     void progress_container::reset() { progress = 0; }
//     void progress_container::add(const std::string_view &str) { hints.push_back(std::string(str)); }
//     void progress_container::add(std::string &&str) { hints.push_back(std::move(str)); }
//     void progress_container::set(const std::vector<std::string> &hints) { this->hints = hints; }
//     void progress_container::set(std::vector<std::string> &&hints) { this->hints = std::move(hints); }
//     void progress_container::clear() { hints.clear(); reset(); }
    int64_t progress_container::get_value() const {
      std::unique_lock<std::mutex> lock(mutex);
      return current_value;
    }
    
    int64_t progress_container::get_max_value() const {
      std::unique_lock<std::mutex> lock(mutex);
      return max_value;
    }
    
    size_t progress_container::get_type() const {
      std::unique_lock<std::mutex> lock(mutex);
      return type;
    }
    
    std::string progress_container::get_hint1() const {
      std::unique_lock<std::mutex> lock(mutex);
      return hint1;
    }
    
    std::string progress_container::get_hint2() const {
      std::unique_lock<std::mutex> lock(mutex);
      return hint2;
    }
    
    std::string progress_container::get_hint3() const {
      std::unique_lock<std::mutex> lock(mutex);
      return hint3;
    }
    
    void progress_container::advance() {
      std::unique_lock<std::mutex> lock(mutex);
      ++current_value;
    }
    
    bool progress_container::finished() const {
      std::unique_lock<std::mutex> lock(mutex);
      return max_value != 0 && current_value >= max_value;
    }
    
    bool progress_container::reseted() const {
      std::unique_lock<std::mutex> lock(mutex);
      return max_value == 0;
    }
    
    void progress_container::set_value(const int64_t &val) {
      std::unique_lock<std::mutex> lock(mutex);
      current_value = val;
    }
    
    void progress_container::set_max_value(const int64_t &val) {
      std::unique_lock<std::mutex> lock(mutex);
      max_value = val;
    }
    
    void progress_container::set_type(const size_t &type) {
      std::unique_lock<std::mutex> lock(mutex);
      this->type = type;
    }
    
    void progress_container::set_hint1(const std::string_view &str) {
      std::unique_lock<std::mutex> lock(mutex);
      hint1 = str;
    }
    
    void progress_container::set_hint2(const std::string_view &str) {
      std::unique_lock<std::mutex> lock(mutex);
      hint2 = str;
    }
    
    void progress_container::set_hint3(const std::string_view &str) {
      std::unique_lock<std::mutex> lock(mutex);
      hint3 = str;
    }
    
    void progress_container::set_hint1(std::string &&str) {
      std::unique_lock<std::mutex> lock(mutex);
      hint1 = std::move(str);
    }
    
    void progress_container::set_hint2(std::string &&str) {
      std::unique_lock<std::mutex> lock(mutex);
      hint2 = std::move(str);
    }
    
    void progress_container::set_hint3(std::string &&str) {
      std::unique_lock<std::mutex> lock(mutex);
      hint3 = std::move(str);
    }
    
    void progress_container::reset() {
      std::unique_lock<std::mutex> lock(mutex);
      current_value = 0;
      max_value = 0;
      // строки?
      hint1.clear();
      hint2.clear();
      hint3.clear();
    }
  }
}
