#include <iostream>
#include <string>
#include <set>
#include <map>

#include <input.h>

using namespace std;

int main(int argc, char * argv[]) {
    int n = read_int();
    char cmd;
    string query;
    set<string> patterns;

    for (int i = 0; i < n; i++) {
        patterns.insert(read_string());
    }
    cout << "R" << std::endl;

    while(cin >> cmd){
        cin.get();
        query = read_string();
        switch(cmd){
            case 'Q':
                {
                    multimap<size_t, string> result;
                    for (set<string>::iterator it = patterns.begin();
                            it != patterns.end(); it++){
                        size_t pos = query.find(*it);
                        if (pos != string::npos){
                            result.insert(make_pair(pos, *it));
                        }
                    }
                    multimap<size_t, string>::iterator it = result.begin();
                    for (int cnt = result.size(); cnt != 0; cnt--, it++){
                        cout << it->second;
                        if (cnt != 1){
                            cout << "|";
                        }
                    }
                    cout << std::endl;
                }
                break;
            case 'A':
                patterns.insert(query);
                break;
            case 'D':
                patterns.erase(query);
                break;
        }
    }
    return 0;
}
