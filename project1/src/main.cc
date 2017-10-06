#include <string>
#include <iostream>

#include <ahocorasick.h>

#define newl ('\n')
#define sep ('|')
#define DEFAULT_RESERVE_SIZE 256

int main(int argc, char * argv[]) {
    int n = 0;
    char cmd;
    std::string query;

    // off sync with stdio, iostream much faster
    std::ios_base::sync_with_stdio(false);
    // reserve query, allocate memory beffore assign (more efficient)
    query.reserve(DEFAULT_RESERVE_SIZE);
    // problem solv operator, @see include/ahocorasick.Operator
    ahocorasick::Operator op;
    std::cin >> n;
    for (int i = 0; i < n; i++) {
        std::cin >> query;
        op.insert(query);
    }
    std::cout << "R" << newl;

    // process
    while (std::cin >> cmd) {
        std::cin.get();
        getline(std::cin, query);
        switch (cmd) {
        case 'Q': {
            bool flag = false;
            // @see also Operator::wrapper
            // process: do lambda function with matched pattern (unique and sequentially)
            // wrapper return false if there are no matches.
            if (!op.wrapper(op.match(query), [&flag](const std::string& pattern) {
                if (!flag) {
                    std::cout << pattern;
                    flag = true;
                } else
                    std::cout << sep << pattern;
            })) std::cout << -1;                // if there are no matches.
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