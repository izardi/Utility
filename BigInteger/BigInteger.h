#pragma once

#include <algorithm>
#include <cstring>
#include <iostream>

class BigInteger {
  bool isNegative;
  char *nums;
  int length;
  int capacity;

 public:
  BigInteger() : length(0), capacity(1), isNegative(false) {
    nums = new char[capacity];
  }

  BigInteger(const char *n);

  BigInteger(const BigInteger &a);

  BigInteger(BigInteger &&a);

  ~BigInteger() { delete[] nums; }

  static void Swap(BigInteger &a, BigInteger &b);

  static bool compareabs(const BigInteger &a, const BigInteger &b);

  static bool isEqual(const BigInteger &a, const BigInteger &b);

  static BigInteger add(BigInteger &a, BigInteger &b);

  static BigInteger minus(BigInteger &a, BigInteger &b);

  char &operator[](int i) { return nums[i]; }

  BigInteger &operator=(BigInteger &&a) {  // swap & copy
    Swap(*this, a);
    return *this;
  }

  BigInteger &operator=(const BigInteger &a);

  bool operator<(const BigInteger &a) const;

  BigInteger operator+(BigInteger &a);

  BigInteger operator-(BigInteger &a);

  int getCap() const { return capacity; }

  int getLen() const { return length; }

  bool isEmpty() const { return length == 0; }

  void reverse();

  void push_back(char x);

  void print();

  void read();
};
