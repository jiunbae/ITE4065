#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <iostream>
#include <stdio.h>

#define newl ('\n')
#define gc getchar
#if defined(__unix__) && !defined(__CYGWIN__)
#define gc getchar_unlocked
#endif

static bool _input_init = false;

inline int read_int() {
    // register int c = gc();
    // int x = 0;
    // bool neg = false;
    // for (; ((c < 48 || c > 57) && c != '-'); c = gc());
    // if (c == '-') {
    //     neg = true;
    //     c = gc();
    // }
    // for (; c > 47 && c < 58; c = gc()) {
    //     x = (x << 1) + (x << 3) + c - 48;
    // }
    // return (neg ? -1 : 1) * x;
    int n;
    // scanf("%d", &n);
    // return n;
    cin >> n;
    return n;  
}

inline char read_char() {
    return std::cin.get();
}

inline void read_string(std::string& string) {
    std::cin >> string;
}

inline void read_line(std::string& line) {
    getline(std::cin, line);
}

inline void put_int(const int& c) {
    std::cout << c;
}

inline void put_char(const char& ch) {
    std::cout << ch;
}

inline void put_char_newline(const char& ch) {
    std::cout << ch << newl;
}

inline void put_string(const std::string& string) {
    std::cout << string;
}

inline void put_line(const std::string& line) {
    std::cout << line << newl;
}

inline void put_newline() {
    std::cout << newl;
}


#endif
