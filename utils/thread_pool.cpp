#include "thread_pool.h"

//#include "Logging.h"

namespace dt {
  thread_pool::thread_pool(const size_t &size) {
    busyWorkersCount = size;
    stop = false;

    for (size_t i = 0; i < size; ++i) {
      workers.emplace_back([this] () {
        while (true) {
          std::function<void()> task;

          {
            std::unique_lock<std::mutex> lock(this->mutex);
            --busyWorkersCount;
            if (tasks.empty()) finish.notify_one();

//             if (!barriers.empty() && barriers.front().taskCount == 0 && busyWorkersCount == 0) {
//               barriers.pop();
//               condition.notify_all();
//             }

            // condition_variable освобождает мьютекс пока ждет
            condition.wait(lock, [this] () {
//               const bool barrierExist = !barriers.empty();
              return stop || !tasks.empty(); //&& ((barrierExist && barriers.front().taskCount != 0) || !barrierExist))
            });

            if (stop && tasks.empty()) return; // это гарантирует что мы выйдем когда закончим выполнение всех задач

            task = std::move(tasks.front());
            tasks.pop();

//             if (!barriers.empty()) --barriers.front().taskCount;

            ++busyWorkersCount;
          }

          task();
        }
      });
    }
  }

  thread_pool::~thread_pool() {
    {
      std::unique_lock<std::mutex> lock(this->mutex);
      stop = true;
      condition.notify_all();
    }

    for(std::thread &worker : workers) {
      if (worker.joinable()) worker.join();
    }
  }

  void thread_pool::submitbase(const std::function<void()> &f) {
    std::unique_lock<std::mutex> lock(mutex);
    if (stop) throw std::runtime_error("Could not submit new task");
    tasks.emplace(f);
    condition.notify_one();
  }

  void thread_pool::barrier() {
    std::unique_lock<std::mutex> lock(this->mutex);
    barriers.push({tasks.size()});
  }

  void thread_pool::compute() {
    // RegionLog rl("thread_pool::compute()", true);

    while (true) {
      std::function<void()> task;

      {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->tasks.empty()) return; // это гарантирует что мы выйдем когда закончим выполнение всех задач

        task = std::move(this->tasks.front());
        this->tasks.pop();
      }

      task();
    }
  }

  void thread_pool::compute(const size_t &count) {
    for (size_t i = 0; i < count; ++i) {
      std::function<void()> task;

      {
        std::unique_lock<std::mutex> lock(mutex);
        if (tasks.empty()) return; // это гарантирует что мы выйдем когда закончим выполнение всех задач

        task = std::move(tasks.front());
        tasks.pop();
      }

      task();
    }
  }
  void thread_pool::wait() {
    // RegionLog rl("thread_pool::wait()", true);
    // не уверен на сколько быстро это все работает
    // но по идее оверхед от атомарных переменных был бы больше
    if (is_dependent(std::this_thread::get_id())) return;

    std::unique_lock<std::mutex> lock(mutex);
    //std::cout << "wait " << this->tasks.empty() << " " << busyWorkersCount << '\n';
    //if (this->tasks.empty() && busyWorkersCount == 0) return;

    finish.wait(lock, [this] () {
      return tasks.empty() && (busyWorkersCount == 0);
    });
  }

  bool thread_pool::is_dependent(const std::thread::id &id) const {
    for (size_t i = 0; i < workers.size(); ++i) {
      if (workers[i].get_id() == id) return true;
    }

    return false;
  }

  uint32_t thread_pool::thread_index(const std::thread::id &id) const {
    for (size_t i = 0; i < workers.size(); ++i) {
      if (workers[i].get_id() == id) return i+1;
    }

    return 0;
  }

  size_t thread_pool::size() const {
    return workers.size();
  }

  size_t thread_pool::tasks_count() const {
    // по идее тут должен быть мьютекс, но я не могу его сюда поставить потому что конст
    // будет ли переменная работать правильно? по стандарту (если я правильно понял) не должна
    std::unique_lock<std::mutex> lock(mutex);
    return tasks.size();
  }

  size_t thread_pool::working_count() const {
    std::unique_lock<std::mutex> lock(mutex);
    return busyWorkersCount;
  }
}
