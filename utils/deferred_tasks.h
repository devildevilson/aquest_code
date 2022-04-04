#ifndef DEFFERED_TASKS_H
#define DEFFERED_TASKS_H

#include <vector>
#include <queue>
#include <functional>
#include <mutex>

namespace devils_engine {
  namespace utils {
    class deffered_tasks {
    public:
      using task_t = std::function<bool(const size_t&)>;
      
      deffered_tasks();
      
      void add(const task_t &t);
      void add(task_t &&t);
      void update(const size_t &time);
    private:
      std::mutex mutex;
      //std::vector<task_t> tasks;
      std::queue<task_t> tasks;
    };
  }
}

#endif
