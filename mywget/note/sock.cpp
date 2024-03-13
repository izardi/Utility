#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int main() {
    int sockfd = 0;
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) exit(1);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);
    
    struct hostent* ht = gethostbyname("www.baidu.com");
    char* ip = inet_ntoa(*(struct in_addr*)ht->h_addr_list[0]);
    printf("ip is %s\n", ip);
    
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0) exit(1);
    
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        exit(1);
    
    const char* http_request_header = 
        "GET /index.html HTTP/1.1\r\nHost: baidu.com\r\nConnection: Close\r\n\r\n";
    
    if (write(sockfd, http_request_header, strlen(http_request_header)) < 0)
        exit(1);
    
    char response_buf[1024] {0};
    
    int len = 0;
    
    while ((len = read(sockfd, response_buf, 1024)) > 0) {
        printf("%s", response_buf);
    }
    
    close(sockfd);
    return 0;
}
