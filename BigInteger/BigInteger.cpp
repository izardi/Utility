#include "BigInteger.h"

BigInteger::BigInteger(const char *n) : length(strlen(n)), isNegative(false) {
  int i = 0;
  if (n[0] == '-') {
    isNegative = true;
    ++i;
  }

  while (i < length && n[i] == '0') ++i;

  capacity = length * 10;
  nums = new char[capacity];
  std::copy(i + n, n + length, nums);
  length = length - i;
}

BigInteger::BigInteger(const BigInteger &a) {
  capacity = a.capacity;
  length = a.length;
  isNegative = a.isNegative;
  nums = new char[capacity];
  std::copy(a.nums, a.nums + a.length, nums);
}

BigInteger::BigInteger(BigInteger &&a) {  // 移动构造
  *this = a;
}

void BigInteger::Swap(BigInteger &a, BigInteger &b) {
  std::swap(a.length, b.length);
  std::swap(a.capacity, b.capacity);
  std::swap(a.isNegative, b.isNegative);
  std::swap(a.nums, b.nums);
}
/*
 * @breif 返回 a > b
 */
bool BigInteger::compareabs(const BigInteger &a, const BigInteger &b) {
  if (a.length != b.length) {
    return a.length > b.length;
  }

  int i = 0;
  while (i < a.length && a.nums[i] == b.nums[i]) ++i;
  if (i == a.length) return false;

  return a.nums[i] > b.nums[i];
}

bool BigInteger::isEqual(const BigInteger &a, const BigInteger &b) {
  if (a.isNegative != b.isNegative || a.length != b.length) return false;

  int i = 0;
  while (i < a.length && a.nums[i] == b.nums[i]) ++i;
  return i == a.length && a.isNegative == b.isNegative;
}

BigInteger BigInteger::add(BigInteger &a, BigInteger &b) {
  printf("add()\n");
  BigInteger res;
  int al = a.length, bl = b.length;

  int base = 0;
  while (al > 0 && bl > 0) {
    base += a[--al] - '0' + b[--bl] - '0';
    res.push_back(base % 10 + '0');
    base /= 10;
  }
  while (al > 0) {
    base += a[--al] - '0';
    res.push_back(base % 10 + '0');
    base /= 10;
  }
  while (bl > 0) {
    base += b[--bl] - '0';
    res.push_back(base % 10 + '0');
    base /= 10;
  }
  res.reverse();
  return res;
}

BigInteger BigInteger::minus(BigInteger &a, BigInteger &b) {
  BigInteger res;
  int al = a.length, bl = b.length;

  int base = 0;
  while (al > 0 && bl > 0) {
    base = base + a[--al] - b[--bl];
    if (base < 0) {
      res.push_back(base + 10 + '0');
      base = -1;
    } else {
      res.push_back(base + '0');
      base = 0;
    }
  }

  while (al > 0) {
    base = base + a[--al] - '0';
    if (base < 0) {
      res.push_back(base + 10 + '0');
      base = -1;
    } else {
      res.push_back(base + '0');
      base = 0;
    }
  }

  while (bl > 0) {
    base = base + b[--bl] - '0';
    if (base < 0) {
      res.push_back(base + 10 + '0');
      base = -1;
    } else {
      res.push_back(base + '0');
      base = 0;
    }
  }

  res.reverse();
  return res;
}

BigInteger &BigInteger::operator=(const BigInteger &a) {
  if (length != 0) delete[] nums;
  capacity = a.capacity;
  length = a.length;
  isNegative = a.isNegative;
  nums = new char[capacity];
  std::copy(a.nums, a.nums + length, nums);
  return *this;
}

BigInteger BigInteger::operator+(BigInteger &a) {
  BigInteger res;
  if (a.isNegative && isNegative) {
    res = add(*this, a);
    res.isNegative = true;
  } else if (a.isNegative && !isNegative) {
    if (compareabs(a, *this)) {
      res = minus(a, *this);
      res.isNegative = true;
    } else {
      res = minus(*this, a);
      res.isNegative = false;
    }
  } else if (!a.isNegative && isNegative) {
    if (compareabs(*this, a)) {
      res = minus(*this, a);
      res.isNegative = true;
    } else {
      res = minus(a, *this);
      res.isNegative = false;
    }
  } else {
    res = add(*this, a);
    res.isNegative = false;
  }

  return res;
}

BigInteger BigInteger::operator-(BigInteger &a) {
  BigInteger res;
  if (!isNegative && a.isNegative) {  // z - f
    res = add(*this, a);
    res.isNegative = false;
  } else if (!isNegative && !a.isNegative) {  // z - z
    if (compareabs(a, *this)) {               // x - d
      res = minus(a, *this);
      res.isNegative = true;
    } else {  // d - x
      res = minus(*this, a);
      res.isNegative = false;
    }
  } else if (isNegative && a.isNegative) {  // f - f
    if (compareabs(*this, a)) {
      res = minus(*this, a);
      res.isNegative = true;
    } else {
      res = minus(a, *this);
      res.isNegative = false;
    }
  } else {  // f - z
    res = add(*this, a);
    res.isNegative = false;
  }

  return res;
}

bool BigInteger::operator<(const BigInteger &a) const {
  if (isNegative && !isNegative)
    return true;
  else if (!isNegative && isNegative)
    return false;
  else if (isNegative) {
    return compareabs(*this, a);
  }
  return compareabs(a, *this);
}

void BigInteger::reverse() {
  int l = 0, r = length - 1;
  while (l < r) {
    std::swap(nums[l++], nums[r--]);
  }
}

void BigInteger::push_back(char x) {
  if (length >= capacity) {
    capacity *= 2;
    char *t = nums;
    nums = new char[capacity];
    std::copy(t, t + length, nums);
    delete[] t;
  }
  nums[length++] = x;
}

void BigInteger::print() {
  if (isNegative) printf("-");
  nums[length] = '\0';
  int i = 0;
  while (nums[i] == '0') ++i;
  printf("%s\n", nums + i);
}

void BigInteger::read() {
  printf("please input number:\n");
  char c = getchar();
  if (c == '-') {
    isNegative = true;
  } else {
    while (c == '0') c = getchar();
    while (c != '\n') {
      push_back(c);
      c = getchar();
    }
  }
}
