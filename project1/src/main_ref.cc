#include <iostream>
#include <string>
#include <set>
#include <map>

using namespace std;

int main() {
    int N;
    set<string> word_list;
    char cmd;
    string buf;

    std::ios::sync_with_stdio(false);

    cin >> N;
    for (int i = 0; i < N; i++){
        cin >> buf;
        word_list.insert(buf);
    }
    cout << "R" << std::endl;

    while(cin >> cmd){
        cin.get();
        getline(cin, buf);
        switch(cmd){
            case 'Q':
                {
                    multimap<size_t, string> result;
                    for (set<string>::iterator it = word_list.begin();
                            it != word_list.end(); it++){
                        size_t pos = buf.find(*it);
                        if (pos != string::npos){
                            result.insert(make_pair(pos, *it));
                        }
                    }
                    multimap<size_t, string>::iterator it = result.begin();
                    int cnt = result.size();
                    if (cnt) {
                        for (; cnt != 0; cnt--, it++){
                            cout << it->second;
                            if (cnt != 1){
                                cout << "|";
                            }
                        }
                    } else {
                        cout << -1;
                    }
                    cout << std::endl;
                }
                break;
            case 'A':
                word_list.insert(buf);
                break;
            case 'D':
                word_list.erase(buf);
                break;
        }
    }
    return 0;
}
