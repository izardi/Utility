#ifndef BUFFER_H_
#define BUFFER_H_

#include <sys/uio.h> //readv
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

class Buffer {
  std::vector<char> mBuf;
  std::atomic<std::size_t> mRpos;
  std::atomic<std::size_t> mWpos;

  char *beginPtr_();
  const char *beginPtr_() const;
  void _makeSpace(size_t len);

public:
  Buffer(int initBufSize = 1024);
  ~Buffer() = default;

  size_t writable() const;
  size_t readable() const;
  size_t prePendable() const;

  const char *peek() const;
  void ensureWritable(size_t len);
  void hasWriteten(size_t len);

  void retrieve(size_t len);
  void retrieveUntil(const char *end);

  void retrieveAll();
  std::string retrieveAllToStr();

  const char *beginWrite() const;
  char *beginWrite();

  void append(const std::string &str);
  void append(const char *str, size_t len);
  void append(const void *str, size_t len);
  void append(const Buffer &_buf);

  ssize_t readFd(int fd, int *Errno);
  ssize_t writeFd(int fd, int *Errno);
};

#endif
