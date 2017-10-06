#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>

// define newl, cuz std::endl is too much slow for buffer flush
#define newl ('\n')
#define sep ('|')
#include <ahocorasick.h>

using namespace std;

int main(int argc, char * argv[]) {
    int n = 0;
    char cmd;
    string query;
    set<string> patterns;

    // make stream io faster, (but do not use mixed `iostream` with `stdio.h`)
    std::ios_base::sync_with_stdio(false);
    std::cin >> n;
    for (int i = 0; i < n; i++) {
        std::cin >> query;
        patterns.insert(query);
    }
    Table* table = new Table(patterns);
    std::cout << "R" << newl;

    while (std::cin >> cmd) {
        std::cin.get();
        getline(std::cin, query);

        switch (cmd) {
        case 'Q': {
            // Table::match return list of matched patterns
            list<string> matches = table->match(query);

            // so, just print return of Table::match
            auto begin = matches.begin();
            if (begin != matches.end()) {
                std::cout << *(begin++);
                while (begin != matches.end()) {
                    std::cout << sep;
                    std::cout << *(begin++);
                }
            } else {
                std::cout << -1;
            }
            std::cout << newl;
        }
                  break;
        case 'A':
            // Table::add do not process immediately, it will processed if necessary(Table::match called)
            table->add(query);
            break;
        case 'D':
            // Table::remove do not process immediately, it will processed if necessary(Table::match called)
            table->remove(query);
            break;
        }
    }
    return 0;
}