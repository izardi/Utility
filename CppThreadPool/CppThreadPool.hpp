#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(size_t threads) {
    workers.resize(threads);

    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> ul(this->queue_mtx);
            this->cond.wait(
                ul, [this] { return this->stop || !this->tasks.empty(); });

            if (this->stop && this->tasks.empty()) return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
          }
          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> ul(queue_mtx);
      stop = true;
    }

    cond.notify_all();
    for (auto& worker : workers) {
      worker.join();
    }
  }

  template <typename F, typename... Args>
  auto addTask(F&& f, Args&&... args) {
    {
      std::unique_lock<std::mutex> ul(queue_mtx);
      if (!stop) {
        tasks.emplace([])
      }
    }
  }

 private:
  std::vector<std::thread> workers;

  std::queue<std::function<void()> > tasks;

  std::mutex queue_mtx;
  std::condition_variable cond;
  bool stop{false};
};

#endif
