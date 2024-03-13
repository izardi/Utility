#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <variant>

using std::condition_variable;
using std::lock_guard;
using std::mutex;
using std::queue;
using std::shared_ptr;

template <typename T>
class ThreadSafeQueue {
  mutable mutex m_mtx;
  queue<T> m_queue;
  std::condition_variable m_cond;

 public:
  void Push(T val) {
    std::lock_guard<std::mutex> lk(m_mtx);
    m_queue.push(std::move(val));
    m_cond.notify_one();
  }

  void WaitAndPop(T &val) {
    std::unique_lock<std::mutex> lk(m_mtx);
    m_cond.wait(lk, [this] { return !m_queue.empty(); });
    val = std::move(m_queue.front());
    m_queue.pop();
  }

  std::shared_ptr<T> WaitAndPop() {
    std::unique_lock<std::mutex> lk(m_mtx);
    m_cond.wait(lk, [this] { return !m_queue.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
    m_queue.pop();
    return res;
  }

  bool TryPop(T &val) {
    std::lock_guard<std::mutex> lk(m_mtx);
    if (m_queue.empty()) return false;

    val = std::move(m_queue.front());
    m_queue.pop();
    return true;
  }

  std::shared_ptr<T> TryPop() {
    std::lock_guard<std::mutex> lk(m_mtx);
    if (m_queue.empty()) return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
    m_queue.pop();
    return res;
  }

  bool Empty() const {
    std::lock_guard<std::mutex> lk(m_mtx);
    return m_queue.empty();
  }
};