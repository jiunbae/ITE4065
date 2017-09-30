#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>

#include <acmap.h>

#define newl ('\n')
#define sep ('|')
#define DEFAULT_RESERVE_SIZE 2048

int main(int argc, char * argv[]) {
    int n = 0;
    char cmd;
    std::string query;

    std::ios_base::sync_with_stdio(false);
    std::cin >> n;

    query.reserve(DEFAULT_RESERVE_SIZE);
    ahocorasick::Operator op;
    for (int i = 0; i < n; i++) {
        std::cin >> query;
        op.insert(query);
    }
    std::cout << "R" << newl;

    while (std::cin >> cmd) {
        std::cin.get();
        getline(std::cin, query);

        switch (cmd) {
        case 'Q': {
            bool flag = false;

            if (!op.wrapper(op.match(query), [&flag](const std::string pattern) {
                if (!flag) {
                    std::cout << pattern;
                    flag = true;
                } else
                    std::cout << sep << pattern;
            })) {
                std::cout << -1;
            }
            std::cout << newl;
        }
            break;
        case 'A':
            op.insert(query);
            break;
        case 'D':
            op.erase(query);
            break;
        }
    }
    return 0;
}
