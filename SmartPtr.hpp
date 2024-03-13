#pragma once
#include <algorithm>
#include <cstdio>
#include <memory>

// 引用计数
class shared_count {
 public:
  shared_count() noexcept : count_(1) {}
  void add_count() noexcept { ++count_; }
  long reduce_count() noexcept { return --count_; }
  long get_count() const noexcept { return count_; }

 private:
  long count_;
};

template <typename T>
class smart_ptr {
 public:
  template <typename U>
  friend class smart_ptr;

  explicit smart_ptr(T* _ptr = nullptr) : ptr(_ptr) {
    if (_ptr) shared_count_ = new shared_count();
  };

  ~smart_ptr() {
    printf("~smart_ptr(): %p\n", this);
    if (ptr && !shared_count_->reduce_count()) {
      delete ptr;
      delete shared_count_;
    }
  }

  /*
   * 禁用copy copy assign
   *
  smart_ptr(const smart_ptr&) = delete;
  smart_ptr& operator= (const smart_ptr&) = delete;
  */

  /*
   * copy时候转移所有权
   *
  smart_ptr(smart_ptr& other) {
      ptr = other.release();
  }

  smart_ptr& operator= (smart_ptr& rhs) {
      smart_ptr(rhs).swap(*this); // 这样写的原因是考虑了安全性
  不至于失败破坏this return *this;               // auto ptr
  就是这么干的所以被删除了
  }
  */

  template <typename U>
  smart_ptr(const smart_ptr<U>& other) noexcept {
    ptr = other.ptr;

    if (ptr) {
      other.shared_count_->add_count();
      shared_count_ = other.shared_count_;
    }
  }

  template <typename U>
  smart_ptr(smart_ptr<U>&& other) noexcept {
    ptr = other.ptr;

    if (ptr) {
      shared_count_ = other.shared_count_;
      other.ptr = nullptr;
    }
    ptr = other.release();
  }

  template <typename U>  // 将U类型转换为T类型然后构造
  smart_ptr(const smart_ptr<U>& other, T* _ptr) noexcept {
    ptr = _ptr;
    if (ptr) {
      other.shared_count_->add_count();
      shared_count_ = other.shared_count_;
    }
  }

  /*
   * 直接完成复制 不用在构造对象
   */
  smart_ptr& operator=(smart_ptr rhs) noexcept {
    rhs.swap(*this);
    return *this;
  }

  T* get() const noexcept { return ptr; }

  long use_count() const noexcept {
    if (ptr) return shared_count_->get_count();
    return 0;
  }

  T& operator*() const noexcept { return *ptr; }

  T* operator->() const noexcept { return ptr; }

  operator bool() const noexcept { return ptr; }

  void swap(smart_ptr& rhs) noexcept {
    std::swap(ptr, rhs.ptr);
    std::swap(shared_count_, rhs.shared_count_);
  }

 private:
  T* ptr;
  shared_count* shared_count_;
};

template <typename T>
void swap(smart_ptr<T>& l, smart_ptr<T>& r) {
  l.swap(r);
}

// U 类型的智能指针转换为T类型的
template <typename T, typename U>
smart_ptr<T> static_pointer_cast(const smart_ptr<U>& oth) noexcept {
  T* ptr = static_cast<T*>(oth.get());
  return smart_ptr<T>(oth, ptr);
}

template <typename T, typename U>
smart_ptr<T> reinterpret_pointer_cast(const smart_ptr<U>& oth) noexcept {
  T* ptr = reinterpret_cast<T*>(oth.get());
  return smart_ptr<T>(oth, ptr);
}

template <typename T, typename U>
smart_ptr<T> const_pointer_cast(const smart_ptr<U>& oth) noexcept {
  T* ptr = const_cast<T*>(oth.get());
  return smart_ptr<T>(oth, ptr);
}

template <typename T, typename U>
smart_ptr<T> dynamic_pointer_cast(const smart_ptr<U>& oth) noexcept {
  T* ptr = dynamic_cast<T*>(oth.get());
  return smart_ptr<T>(oth, ptr);
}
