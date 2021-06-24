#ifndef DEFFERED_TASKS_H
#define DEFFERED_TASKS_H

#include <vector>
#include <functional>

namespace devils_engine {
  namespace utils {
    class deffered_tasks {
    public:
      using task_t = std::function<bool(const size_t&)>;
      
      deffered_tasks();
      
      void add(const task_t &t);
      void update(const size_t &time);
    private:
      std::vector<task_t> tasks;
    };
  }
}

#endif
