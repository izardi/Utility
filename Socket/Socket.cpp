#include "Socket.h"

#include <sys/socket.h>

#include <algorithm>

Socket::Socket(int domain, int type, int protocol) {
  sock = ::socket(domain, type, protocol);
  if (sock < 0) perror("Socket()");
}

Socket::Socket(int fd) : sock(fd) {}

Socket::~Socket() { this->Close(); }

void Socket::Close() {
  if (sock != -1) {
    ::close(sock);
    sock = -1;
  }
}

void Socket::Bind(const struct sockaddr *addr, socklen_t len) {
  if (::bind(sock, (const struct sockaddr *)addr, len) < 0) perror("Bind()");
}

void Socket::TListen(int backlog) {
  if (::listen(sock, backlog) < 0) perror("TListen()");
}

int Socket::TAccept(struct sockaddr *__restrict addr,
                    socklen_t *__restrict addr_len) {
  int clientfd = ::accept(sock, addr, addr_len);
  if (clientfd < 0) perror("TAccept()");
  return clientfd;
}

void Socket::TConnect(const struct sockaddr *addr, socklen_t len) {
  if (::connect(sock, addr, len) < 0) perror("TConnect()");
}

ssize_t Socket::TSend(const void *buf, size_t n, int flags) {
  return ::send(sock, buf, n, flags);
}

ssize_t Socket::TRecv(void *buf, size_t n, int flags) {
  return ::recv(sock, buf, n, flags);
}
ssize_t Socket::USendto(const void *buf, size_t n, int flags,
                        const struct sockaddr *addr, socklen_t addr_len) {
  return sendto(sock, buf, n, flags, addr, addr_len);
}

ssize_t Socket::URecvfrom(void *__restrict buf, size_t n, int flags,
                          struct sockaddr *__restrict addr,
                          socklen_t *__restrict addr_len) {
  return recvfrom(sock, buf, n, flags, addr, addr_len);
}