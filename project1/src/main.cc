#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>

#define newl ('\n')
#define sep ('|')
#include "aho-corasick.h"

using namespace std;

int main(int argc, char * argv[]) {
    int n = 0;
    char cmd;
    string query;
    vector<string> patterns;

    std::ios_base::sync_with_stdio(false);
    query.reserve(1024);
    std::cin >> n;
    for (int i = 0; i < n; i++) {
        std::cin >> query;
        patterns.push_back(query);
    }
    Table* table = new Table(patterns);
    std::cout << "R" << newl;


    while (std::cin >> cmd) {
        std::cin.get();
        getline(std::cin, query);

        switch (cmd) {
        case 'Q': {
            bool flag = false;
            if (!table->wrapper(table->match(query), [&flag](const std::string& pattern) {
                if (!flag) {
                    std::cout << pattern;
                    flag = true;
                } else {
                    std::cout << sep << pattern;
                }
            })) std::cout << -1;

            std::cout << newl;
        }
                  break;
        case 'A':
            table->add(query);
            break;
        case 'D':
            table->remove(query);
            break;
        }
    }
    return 0;
}
