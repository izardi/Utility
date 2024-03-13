#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>

class XTcp {
public:
    XTcp() {}
    virtual ~XTcp() {}

    int CreatSock();
    bool Bind(uint16_t port);
    XTcp Accept();
    void Close();
    ssize_t Recv(char* buf, size_t len);
    ssize_t Send(const char* buf, size_t len);
    bool Connect(const char* ip, unsigned short port);


private:
    int sock;
    unsigned short port;
    char ip[16]; 
};
