#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
  struct Pool {
    std::mutex mtx_;
    std::condition_variable cond_;
    std::queue<std::function<void()>> tasks_;
    bool isClose_;
  };
  std::shared_ptr<Pool> pool_;

 public:
  explicit ThreadPool(size_t thread_count = 8)
      : pool_(std::make_shared<Pool>()) {
    assert(thread_count > 0);

    for (size_t i = 0; i < thread_count; ++i) {
      std::thread([pool = pool_] {
        std::unique_lock<std::mutex> ul(pool->mtx_);
        while (true) {
          if (!pool->tasks_.empty()) {
            auto task = std::move(pool->tasks_.front());
            pool->tasks_.pop();
            ul.unlock();
            task();
            ul.lock();
          } else if (pool->isClose_)
            break;
          else
            pool->cond_.wait(ul);
        }
      }).detach();
    }
  }
  ThreadPool(ThreadPool&&) = default;

  ~ThreadPool() {
    if (static_cast<bool>(pool_)) {
      {
        std::lock_guard<std::mutex> lg(pool_->mtx_);
        pool_->isClose_ = true;
      }
      pool_->cond_.notify_all();
    }
  }
  template <typename F>
  void addTask(F&& task) {
    {
      std::lock_guard<std::mutex> lg(pool_->mtx_);
      pool_->tasks_.emplace(std::forward<F>(task));
    }
    pool_->cond_.notify_one();
  }
};

#endif