#ifndef _STRVEC_H__
#define _STRVEC_H__

#include <memory>
#include <string>
namespace Utility {

class StrVec {
 private:
  std::string* elements;
  std::string* first_free;
  std::string* cap;

 public:
  StrVec() : elements(nullptr), first_free(nullptr), cap(nullptr) {}

  // 移动构造函数移动赋值必须啊标记为noexcept
  // 移动操作不应该抛出异常
  // 因为移动操作参数状态会发生改变所以无法回退所以不能有异常发生
  StrVec(StrVec&& s) noexcept
      : elements(s.elements), first_free(s.first_free), cap(s.cap) {
    s.elements = s.first_free = s.cap = nullptr;
  }

  StrVec& operator=(StrVec&& rhs) noexcept {
    if (this != &rhs) {
      free();
      elements = rhs.elements;
      first_free = rhs.first_free, cap = rhs.cap;
      rhs.elements = rhs.cap = rhs.first_free = nullptr;
    }
    return *this;
  }

  StrVec(const StrVec& s) {
    auto data = alloc_n_copy(s.begin(), s.end());
    elements = data.first;
    first_free = cap = data.second;
  }

  StrVec& operator=(const StrVec& rhs) {
    auto data = alloc_n_copy(rhs.begin(), rhs.end());
    free();
    elements = data.first;
    first_free = cap = data.second;
    return *this;
  }

  std::string& operator[](std::size_t n) { return elements[n]; }
  const std::string& operator[](std::size_t n) const { return elements[n]; }

  ~StrVec() { free(); }

  // 移动
  void push_back(std::string&& s) {
    chk_n_alloc();
    alloc.construct(first_free++, std::move(s));
  }

  // 复制
  void push_back(const std::string& s) {
    chk_n_alloc();
    alloc.construct(first_free++, s);
  }

  size_t size() const { return first_free - elements; }

  size_t capacity() const { return cap - elements; }

  std::string* begin() const { return elements; }

  std::string* end() const { return first_free; }

 private:
  static std::allocator<std::string> alloc;

  void chk_n_alloc() {
    if (size() == capacity()) reallocate();
  }

  std::pair<std::string*, std::string*> alloc_n_copy(const std::string* b,
                                                     const std::string* e) {
    auto data = alloc.allocate(e - b);
    // uninitialized_copy
    // 复制_1 到 _2 到data, 返回复制的最后一个元素的下一个的迭代器
    return {data, uninitialized_copy(b, e, data)};
  }

  void free() {
    if (elements) {
      for (auto p = first_free; p != elements;) alloc.destroy(--p);
      alloc.deallocate(elements, cap - elements);
    }
  }

  void reallocate() {
    auto newcapacity = size() ? 2 * size() : 1;
    auto newdata = alloc.allocate(newcapacity);
    auto dest = newdata;
    auto elem = elements;
    for (size_t i = 0; i != size(); ++i)
      alloc.construct(dest++, std::move(*elem++));
    free();
    elements = newdata;
    first_free = dest;
    cap = elements + newcapacity;
  }
};

}  // namespace Utility

#endif
