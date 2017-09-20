#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <iostream>
#include <stdio.h>

#define newl ('\n')
#define gc getchar
#if defined(__unix__) && !defined(__CYGWIN__ )
    #define gc getchar_unlocked
#endif

static bool init = false;

using namespace std;

inline int read_int() {
    register int c = gc();
    int x = 0;
    bool neg = false;
    for (; ((c < 48 || c > 57) && c != '-'); c = gc());
    if (c == '-') {
        neg = true;
        c = gc();
    }
    for(; c > 47 && c < 58; c = gc()) {
        x = (x << 1) + (x << 3) + c - 48;
    }
    return (neg ? -1 : 1) * x;
}

inline std::string read_string() {
    if (!init) {
        std::ios_base::sync_with_stdio(false);
        std::cin.tie(NULL);
        init = true;
    }
    std::string line;
    getline(std::cin, line);
    return line;
}

#endif
