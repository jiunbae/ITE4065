#include <iostream>
#include <string>
#include <set>
#include <map>

#include <input.h>

using namespace std;

int main(int argc, char * argv[]) {
    int n; 
    char cmd;
    string query;
    set<string> patterns;

    cin >> n;
    for (int i = 0; i < n; i++) {
        //read_string(query);
        cin >> query;
        patterns.insert(query);
    }
    cout << "R" << '\n';

    while(cin >> cmd){
        cin.get();
        //read_line(query);
        getline(cin, query);
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
                    cout << '\n';
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
