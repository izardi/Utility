#pragma once

#include "buffer.h"

#include <unistd.h>
Buffer::Buffer(int _size) : mBuf(_size), mRpos(0), mWpos(0) {}

size_t Buffer::writable() const { return mBuf.size() - mWpos; };

size_t Buffer::readable() const { return mWpos - mRpos; }

size_t Buffer::prePendable() const { return mRpos; }

const char *Buffer::peek() const { return beginPtr_() + mRpos; }

void Buffer::retrieve(size_t len) {
  assert(len <= readable());
  mRpos += len;
}

void Buffer::retrieveUntil(const char *end) {
  assert(peek() <= end);
  retrieve(end - peek());
}

void Buffer::retrieveAll() {
  bzero(&mBuf[0], mBuf.size());
  mRpos = mWpos = 0;
}

std::string Buffer::retrieveAllToStr() {
  std::string str(peek(), readable());
  retrieveAll();
  return str;
}

const char *Buffer::beginWrite() const { return beginPtr_() + mWpos; }

char *Buffer::beginWrite() { return beginPtr_() + mWpos; }

void Buffer::hasWriteten(size_t len) { mWpos += len; }

void Buffer::append(const std::string &str) {
  append(str.data(), str.length());
}

void Buffer::append(const char *str, size_t len) {
  assert(str);
  ensureWritable(len);
  std::copy(str, str + len, beginWrite());
}

void append(const void *str, size_t len) {
  assert(str);
  append(static_cast<const char *>(str), len);
}

void Buffer::append(const Buffer &_buf) {
  append(_buf.peek(), _buf.readable());
}

void Buffer::ensureWritable(size_t len) {
  if (writable() < len)
    _makeSpace(len);
  assert(writable() >= len);
}

ssize_t Buffer::readFd(int fd, int *Errno) {
  char buf_[BUFSIZ];
  const size_t vwritable = writable();

  struct iovec iov[2] = {{beginPtr_() + mWpos, vwritable}, {buf_, BUFSIZ}};
  const ssize_t len = readv(fd, iov, 2);
  if (len < 0)
    *Errno = errno;
  else if (static_cast<size_t>(len) <= writable()) {
    mWpos += len;
  } else {
    mWpos = mBuf.size();
    append(buf_, len - vwritable);
  }
  return len;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
  size_t readlen = readable();
  ssize_t len = write(fd, peek(), readlen);
  if (len < 0) {
    *saveErrno = errno;
    return len;
  }
  mRpos += len;
  return len;
}

char *Buffer::beginPtr_() { return &*mBuf.begin(); }

const char *Buffer::beginPtr_() const { return &*mBuf.begin(); }

void Buffer::_makeSpace(size_t len) {
  if (writable() + prePendable() < len)
    mBuf.resize(mWpos + len + 1);
  else {
    size_t len = readable();
    std::copy(beginPtr_() + mRpos, beginPtr_() + mWpos, beginPtr_());
    mRpos = 0;
    mWpos = mRpos + len;
    assert(len == readable());
  }
}
