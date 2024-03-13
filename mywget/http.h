#pragma once

#include "parse.h"

char* http_create_request(http_url* url);

int http_connect(http_url* url);

int http_download(int sockfd);
