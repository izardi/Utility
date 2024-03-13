#pragma once

typedef struct {
    char *scheme;
    char *user;
    char *pass;
    char *domain;
    char *host;
    char *ip;
    char *uri;
    unsigned short port;
} http_url;


void print_http_url(http_url* hurl);
char* parse_scheme(const char *url);
char* parse_domain(const char *url, const char *scheme);
char* parse_host(const char* domain);
unsigned short parse_port(const char* domain, const char* host, const char* scheme);
char* parse_ip(const char* host_name);
char* parse_uri(const char *url, const char *scheme, const char *domain);
