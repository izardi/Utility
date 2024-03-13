#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <cassert>
#include <condition_variable>
#include <ctime>
#include <deque>
#include <mutex>

#define _LGMTX_ std::lock_guard<std::mutex> lg(_mtx);
#define _ULMTX_ std::unique_lock<std::mutex> ul(_mtx);

template <typename T>
class SafeDeque {
  std::deque<T> _deq;
  mutable std::mutex _mtx;
  size_t _cap;
  bool _isClose;
  std::condition_variable _condC;
  std::condition_variable _condP;

 public:
  explicit SafeDeque(size_t maxCapacity = 1000)
      : _cap(maxCapacity), _isClose(false) {
    assert(maxCapacity > 0);
  }

  ~SafeDeque() { Close(); }

  void Close() {
    {
      _LGMTX_
      _deq.clear();
      _isClose = true;
    }
    _condC.notify_all();
    _condP.notify_all();
  }

  void flush() { _condC.notify_one(); }

  void clear() {
    _LGMTX_
    _deq.clear();
  }

  size_t size() const {
    _LGMTX_
    return _deq.size();
  }

  size_t capacity() const {
    _LGMTX_
    return _cap;
  }

  T front() {
    _LGMTX_
    return _deq.front();
  }

  T back() {
    _LGMTX_
    return _deq.back();
  }

  void push_back(const T& item) {
    _ULMTX_
    _condP.wait(ul, _deq.size() >= _cap);

    _deq.push_back(item);
    _condC.notify_one();
  }

  bool empty() {
    _LGMTX_
    return _deq.empty();
  }

  bool full() {
    _LGMTX_
    return _deq.size() == _cap;
  }

  bool pop(T& item) {
    {
      _ULMTX_
      _condC.wait(ul, !_deq.empty() || _isClose);
      if (_isClose) return false;
      item = _deq.front();
      _deq.pop_front();
    }
    _condP.notify_one();
    return true;
  }

  bool pop(T& item, int timeout) {
    {
      _ULMTX_
      std::cv_status cvs = _condC.wait_for(ul, std::chrono::seconds(timeout),
                                           !_deq.empty() || _isClose);
      if (cvs == std::cv_status::timeout || _isClose) return false;

      item = _deq.front();
    }
    _condP.notify_one();
    return true;
  }

  void push_front(const T& item) {
    {
      _ULMTX_
      _condP.wait(ul, _deq.size() >= _cap);

      _deq.push_front(item);
    }
    _condC.notify_one();
  }
};
#endif