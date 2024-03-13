#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "http.h"
#include "parse.h"
#include "log.h"

int main(int argc, const char *argv[]){

	if (argc != 2) return 1;

    set_debug(true);

	const char *url = argv[1];
	char *scheme = parse_scheme(url);
    char *domain = parse_domain(url, scheme);  
    char *host = parse_host(domain);
    unsigned short port = parse_port(domain, host, scheme);
    char *ipv4 = parse_ip(host);
    char *uri = parse_uri(url, scheme, domain);

    http_url *wget_url = (http_url *)malloc(sizeof(http_url));
    memset(wget_url, '\0', sizeof(http_url));

    wget_url->scheme = scheme;
    wget_url->domain = domain;
    wget_url->host = host;
    wget_url->port = port;
    wget_url->ip = ipv4;
    wget_url->uri = uri;
    
    print_http_url(wget_url);
    
    int sockfd = http_connect(wget_url);
    printf("%d", sockfd);
    http_download(sockfd);

    free(uri);
    free(scheme);
    free(domain);
    free(host);
    free(ipv4);
    free(wget_url);
	return 0;
}
