#include <iostream>
#include <string>
#include <vector>

#define newl ('\n')
#include <aho-corasick.h>

using namespace std;

int main(int argc, char * argv[]) {
    int n; 
    char cmd;
    string query;
    TABLE table;
    vector<string> patterns;
    int init_state = 0;

    std::ios::sync_with_stdio(false);
    cin >> n;
    for (int i = 0; i < n; i++) {
        //read_string(query);
        cin >> query;
        patterns.push_back(query);
    }
    table = create_table(patterns, init_state);
    cout << "R" << newl;

    while(cin >> cmd){
        cin.get();
        //read_line(query);
        getline(cin, query);
        switch(cmd){
            case 'Q': {
                MATCH matches = find_match(table, query, init_state);

                auto begin = matches.begin();
                if (begin != matches.end())
                    cout << patterns[*begin++];
                while (begin != matches.end())
                    cout << '|' << patterns[*begin++];
                cout << newl;
            }
                break;
            case 'A':
                patterns.push_back(query);
                table = create_table(patterns, init_state);
                break;
            case 'D':
                for (auto it = patterns.begin(); it != patterns.end(); it++) {
                    if ((*it) == query) {
                        patterns.erase(it);
                    }
                }
                table = create_table(patterns, init_state);
                break;
        }
    }
    return 0;
}
