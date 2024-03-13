#include "XTcp.hpp"

ssize_t XTcp::Recv(char* buf, size_t len){
    return ::recv(sock, buf, len, 0);
}

ssize_t XTcp::Send(const char* buf, size_t len) {
    ssize_t cnt = 0;
    while (cnt != len) {
        cnt += ::send(sock, buf + cnt , len - cnt, 0);
    }
    return cnt;
}

void XTcp::Close() {
    if (sock > 0) ::close(sock);
}

bool XTcp::Bind(uint16_t port) {
    sockaddr_in saddr{AF_INET, htons(port), {htonl(INADDR_ANY)}, {0}};
    if (::bind(sock, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr)) != 0) {
        perror("::bind error");
        return false;
    }
    listen(sock, 128);
    return true;
}

XTcp XTcp::Accept() {
    sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int clisock = ::accept(sock, reinterpret_cast<sockaddr*>(&caddr), &len);
    if (clisock < 0) {
        perror("::accept error");
        return {};
    }
    printf("accept client %d\n", clisock);
    XTcp tcp;
    tcp.sock = clisock;
    tcp.port = ntohs(caddr.sin_port);
    inet_ntop(AF_INET, &caddr.sin_addr, ip, 16);
    printf("client ip is %s, port is %d\n", tcp.ip, tcp.port);

    return tcp;
}

int XTcp::CreatSock() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("creatSock error");
    }
    return sock;
}

bool XTcp::Connect(const char *ip, unsigned short port) {
    if (sock <= 0 ) CreatSock();
    sockaddr_in saddr = 
        {.sin_family = AF_INET, .sin_port = port, .sin_addr = {inet_addr(ip)}};

    if (0 != ::connect(sock, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr))) {
        perror("::connect error");
        return false;
    }
    return true;
}


