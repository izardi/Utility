#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

class Socket {
  int sock;

 public:
  static void getHostByname(const char *url, char *ipinfo) {
    struct addrinfo hints, *res = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (::getaddrinfo(url, nullptr, &hints, &res) != 0) {
      perror("getaddrinfo()");
      _exit(-1);
    }
    char ip[INET_ADDRSTRLEN];
    for (auto p = res; p != nullptr; p = p->ai_next) {
      if (p->ai_family == AF_INET) {
        inet_ntop(p->ai_family, &((struct sockaddr_in *)p->ai_addr)->sin_addr,
                  ip, INET_ADDRSTRLEN);
        printf("ip of %s is %s\n", url, ip);
      }
    }
    if (ipinfo) {
      strcpy(ipinfo, ip);
    }
  }

  Socket(int domain, int type, int protocol = 0);
  explicit Socket(int fd);
  ~Socket();

  void Close();

  void Bind(const struct sockaddr *addr, socklen_t len);

  void TListen(int backlog = SOMAXCONN);

  int TAccept(struct sockaddr *__restrict addr = nullptr,
              socklen_t *__restrict addr_len = nullptr);

  void TConnect(const struct sockaddr *addr, socklen_t len);

  ssize_t TSend(const void *buf, size_t n, int flags = 0);

  ssize_t TRecv(void *buf, size_t n, int flags = 0);

  ssize_t USendto(const void *buf, size_t n, int flags,
                  const struct sockaddr *addr, socklen_t addr_len);

  ssize_t URecvfrom(void *__restrict buf, size_t n, int flags,
                    struct sockaddr *__restrict addr,
                    socklen_t *__restrict addr_len);
};
