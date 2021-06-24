#include "deferred_tasks.h"

namespace devils_engine {
  namespace utils {
    deffered_tasks::deffered_tasks() {}
    void deffered_tasks::add(const task_t &t) { tasks.push_back(t); }
    void deffered_tasks::update(const size_t &time) {
      for (size_t i = 0; i < tasks.size(); ++i) {
        const bool ret = tasks[i](time);
        
        if (ret) { // удаляем?
          tasks[i] = std::move(tasks.back());
          tasks.pop_back();
          --i;
        }
      }
    }
  }
}
