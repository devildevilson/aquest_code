#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <vector>
#include <atomic>

#include <iostream>

// барьер у меня все равно кислый
// он не учитывает те задачи, которые добавляются в самих задачах
// в этом случае либо запоминать выполнение последней задачи с добавлением
// либо самому делить на большее количество задач

namespace dt {
  // нужно еще придумать что делать с чтением переменных? mutable mutex ?
  class thread_pool {
  public:
    thread_pool(const size_t &size);
    ~thread_pool();
    
    void submitbase(const std::function<void()> &f);

    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
      using return_type = typename std::result_of<F(Args...)>::type;
      
      // тут лучше не придумали
      auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

      std::future<return_type> res = task->get_future();
      {
        std::unique_lock<std::mutex> lock(mutex);
        if (stop) throw std::runtime_error("Could not submit new task");
        tasks.emplace([local_task = std::move(task)] () { (*local_task)(); });

        condition.notify_one();
      }

      return res;
    }

    // быстрое сравнение лямбды и bind показало что разница минимальна (bind даже порой быстрее), проблема если и заключается в чем то точно не в этом
    // нужно сравнить еще с оптимизациями, с оптимизациями таже история (лямбда в этот раз чутка быстрее), лямбды конечно скорее всего будут быстрее чем bind
    // потом когда я перейду на C++17 нужно будет посмотреть быстрый способ создать таск
    template<class F, class... Args>
    void submitnr(F&& f, Args&&... args) {
      // std::bind пытается создать копии всего что тут передано (хотя наверное мне копии и нужны), функцию можно переместить
      auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
      
      // вот так тоже можно
      //auto task = [f, args...]() { f(args...); };
      
      // только с C++14
//       auto task = [f = std::move(f), largs = std::make_tuple(std::forward<Args>(args)...)] () mutable {
//         return std::apply(std::move(f), std::move(largs));
//       }
      
      {
        std::unique_lock<std::mutex> lock(mutex);
        if (stop) throw std::runtime_error("Could not submit new task");
        tasks.emplace(std::move(task));

        condition.notify_one();
      }
    }

    void barrier();
    void compute();
    void compute(const size_t &count);
    void wait(); // просто ждет всех

    bool is_dependent(const std::thread::id &id) const;
    uint32_t thread_index(const std::thread::id &id) const;

    size_t size() const;
    size_t tasks_count() const;
    size_t working_count() const;
  private:
    struct BarrierData {
      size_t taskCount;
      // думаю что ничего тут дополнитульно не нужно
    };
    
    bool stop;

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::queue<BarrierData> barriers;

    mutable std::mutex mutex;
    std::condition_variable condition;
    std::condition_variable finish;
    size_t busyWorkersCount;
  };
}

#endif
