#include <cstdio>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <netdb.h>

int main(int argc, char* argv[]) {
    struct addrinfo hints, *res;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: %s hostname \n", argv[0]);
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], nullptr, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 0;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    for(struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
        void *addr;
        char ipver[6];

        if(p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
            strncpy(ipver, "ipv4", 4);
            ipver[4] = '\0';
        } else {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            strncpy(ipver, "ipv6", 4);
            ipver[4] = '\0';
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf(" %s : %s\n", ipver, ipstr);

    }
    freeaddrinfo(res);
    return 0;
}
