#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <cstring>
#include <cstdio>

constexpr int group_id = 1337;

void strtolower(char *str) {
    for (; *str; ++str)
        *str = (char)tolower(*str);
}

void fatal_error(const char *syscall) {
    perror(syscall);
    exit(1);
}

#define log(...) do { printf(__VA_ARGS__); putchar('\n'); fflush(stdout); } while(0)

#endif
