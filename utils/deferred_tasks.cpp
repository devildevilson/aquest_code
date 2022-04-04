#include "deferred_tasks.h"

namespace devils_engine {
  namespace utils {
    // очередь?
    deffered_tasks::deffered_tasks() {}
    void deffered_tasks::add(const task_t &t) { 
      std::unique_lock<std::mutex> lock(mutex);
      //tasks.push_back(t);
      tasks.push(t);
    }
    
    void deffered_tasks::add(task_t &&t) { 
      std::unique_lock<std::mutex> lock(mutex);
      //tasks.emplace_back(std::move(t));
      tasks.emplace(std::move(t));
    }
    
    void deffered_tasks::update(const size_t &time) {
//       task_t tmp;
//       
//       std::vector<task_t> new_tasks;
//       new_tasks.reserve(tasks.size());
//       for (size_t i = 0; i < tasks.size(); ++i) {
//         {
//           std::unique_lock<std::mutex> lock(mutex);
//           tmp = std::move(tasks[i]);
//         }
//         
//         const bool ret = tmp(time); // после исполнения этого количество tasks может возрасти
//         if (ret) continue; // удаляем
//         new_tasks.emplace_back(std::move(tmp));
//       }
//       
//       std::unique_lock<std::mutex> lock(mutex);
//       tasks = std::move(new_tasks);
      
      // в таком дизайне должно произойти все как задумано
      std::queue<task_t> new_tasks;
      
      while (true) {
        task_t tmp;
        
        {
          std::unique_lock<std::mutex> lock(mutex);
          if (tasks.empty()) { tasks = std::move(new_tasks); break; }
          
          tmp = std::move(tasks.front());
          tasks.pop();
        }
        
        const bool ret = tmp(time);
        if (ret) continue; // удаляем
        new_tasks.emplace(std::move(tmp));
      }
    }
  }
}
