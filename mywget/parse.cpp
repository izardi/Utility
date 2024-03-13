#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include "parse.h"
#include "log.h"

void print_http_url(http_url* hurl) {
    info_log("scheme:\t%s\n", hurl->scheme);
    info_log("domain:\t%s\n", hurl->domain);
    info_log("host:\t%s\n", hurl->host);
    info_log("ip:\t%s:%d\n", hurl->ip, hurl->port);
    info_log("uri:\t%s\n", hurl->uri);
} 

// <scheme>://<user>:<password>@<host>:<port>/<url-path>?<query>#<fragment>
char* parse_scheme(const char *url){
    const char* p = strstr(url, "://");

    if (p == nullptr) {
        printf("url illegal\t%s\n", url);
        exit(1);
    }

    long len = p - url + 1;
    char *scheme = (char *)malloc(len);

    memset(scheme, 0, len);
    strncpy(scheme, url, len-1);

    debug_log("scheme:\t%s\n", scheme);

    return scheme;
}

char* parse_domain(const char *url, const char *scheme) {
// 没有 URL	https://a.com	host 后面为空
// 后面有 URI	https://a.com/bbbb	host 后面会有 /
// 有 Query 参数	https://a.com?b=/c	host 后面为 ?，且参数里面不排除有 / 的可能性
// 有锚点	https://a.com#dddd	host 后面可能直接跟锚点标识#

    url = url + strlen(scheme) + 3;

    size_t i;
    for(i = 0; i < strlen(url); ++i) {
        if (url[i] == '/' || url[i] == '?' || url[i] == '#')
            break;
    }
    
    char* domain = strndup(url, i);

    debug_log("domain:\t%s\n", domain);

    return domain;
}

char* parse_host(const char* domain) {
    
    const char* s = strchr(domain, ':');
    char* host = nullptr;

    if (s != nullptr) {
        host = strndup(domain, s - domain);
    } else {
        host = strndup(domain, strlen(domain));
    }

    char tmp;
    for (int i = 0; i < (int)strlen(host); ++i) {
        tmp = host[i];
        if ((tmp >= 'a' && tmp <= 'z') || 
                (tmp >= '0' && tmp <= '9') ||
                (tmp == '.' || tmp == '-')) {}
        else {
            error_log("illegal host:%s\n", host);
            exit(1);
        }
    }

    debug_log("host:\t%s\n", host);

    return host;
}

unsigned short parse_port(const char* domain, const char* host, const char* scheme) {
    unsigned short port = 0;

    if(strlen(domain) != strlen(host)) {
        port = (unsigned short)(atoi(domain + strlen(host) +1));
    } else {
        if (strcmp(scheme, "http") == 0) port = 80;
        else if (strcmp(scheme, "https") == 0) port = 443;
        else {
            error_log("unknow port");
            exit(1);
        }
    }

    debug_log("port:\t%u\n", port);

    return port;
}


// gethostbyname
// struct hostent* gethostbyname(const char* name);

/*
 * struct hostent {
 *      char* h_name; 
 *      char** h_aliases;
 *      int h_addrtype;
 *      int h_length;
 *      char** h_addr_list;
 * }
*/
 // inet_ntoa
 // 
 // 将网络地址转换为带.分割的字符串格式
 // char* inet_nota(struct in_addr in);
 //
 // #include <sys/socket.h>
 // #include <netinet/in.h>
 // #include <arpa/inet.h>
 
char* parse_ip(const char* host_name) {
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host_name, nullptr, &hints, &res);
    if (status != 0) {
        error_log("getaddrinfo error %s\n", gai_strerror(status));
        exit(1);
    }

    // 不在遍历 ai_next 只得到一个ip即可

    char *ipv4 = (char*)malloc(INET_ADDRSTRLEN);
    struct sockaddr_in *ip = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(ip->sin_addr), ipv4, INET_ADDRSTRLEN);

    freeaddrinfo(res);

    debug_log("ip:\t%s\n", ipv4);
    return ipv4;
}


char* parse_uri(const char *url, const char *scheme, const char *domain) {
    url += strlen(scheme) + 3 + strlen(domain);

    char *uri = strndup(url, strlen(url));

    debug_log("uri:\t%s\n", uri);
    return uri;
}
