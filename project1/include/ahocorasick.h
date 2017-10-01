#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#include <queue>

using TABLE = std::vector<std::vector<int>>;

using namespace std;

#define CHAR_SIZE (26)
#define CHAR_START ('a')

class Table {
public:
    std::set<std::string> patterns;

    Table(const std::set<std::string>& patterns) : patterns(patterns) {
        for (const auto& pattern : patterns) {
            table_size += pattern.length() + 1;
        }

        state_init = patterns.size();
        state_num = state_init + 1;
        state = state_init;

        raw = std::vector<std::vector<int>>(table_size, std::vector<int>(CHAR_SIZE, -1));

        for (const auto& pattern : patterns) {
            update_table(pattern);
        }
    }

    list<string> match(const string& query) {
        unsigned int start;
        unsigned int pos;
        int state;

        sync();

        list<string> result;
        for (start = 0; start < query.length(); start++) {
            string r = "";
            state = state_init;
            pos = start;
            do {
                r += query[pos];
                state = raw[state][query[pos++] - CHAR_START];
                if (state < state_init && state > -1) {
                    auto iter = find(result.begin(), result.end(), r);
                    if (iter == result.end())
                        result.push_back(r);
                }
            } while ((state != -1) && (pos < query.length()));
        }
        return result;
    }

    void add(const string& pattern) {
        pre_add.insert(pattern);
    }

    void remove(const string& pattern) {
        pre_rem.insert(pattern);
    }

    int init_state() const {
        return state_init;
    }

private:
    int table_size = 0;

    int state_final = 0;
    int state_init = 0;
    int state_num = 0;
    int state = 0;
    int pre_state = 0;
    char pre_char = 0;

    std::vector<std::vector<int>> raw;
    std::set<std::string> pre_add, pre_rem;

    void sync() {
        int _table_size = table_size;

        if (pre_add.empty() && pre_rem.empty())
            return;

        for (const auto& pattern : pre_add) {
            patterns.insert(pattern);
            _table_size += pattern.length() + 1;
        }
        
        for (const auto& pattern : pre_rem) {
            patterns.erase(pattern);
            _table_size -= pattern.length() + 1;
        }

        if (_table_size > table_size) {
            raw.resize(_table_size, std::vector<int>(CHAR_SIZE, -1));
        }

        table_size = _table_size;

        for (int i = 0; i < state_num + 1; i++) {
            fill(raw[i].begin(), raw[i].end(), -1);
        }

        state_final = 0;
        state_init = patterns.size();
        state_num = state_init + 1;
        state = state_init;

        for (const auto& pattern : patterns) {
            update_table(pattern);
        }

        pre_add.clear();
        pre_rem.clear();
    }

    void update_table(const string& pattern) {
        for (const auto& ch : pattern) {
            if (raw[state][ch - CHAR_START] == -1) {
                raw[state][ch - CHAR_START] = state_num;
                pre_state = state;
                pre_char = ch - CHAR_START;
                state = state_num++;
            }
            else {
                state = raw[pre_state = state][pre_char = ch - CHAR_START];
            }
        }
        raw[pre_state][pre_char] = state_final;

        swap(raw[state_final], raw[state]);
        fill(raw[state].begin(), raw[state].end(), -1);

        --state_num;
        state = state_init;
        ++state_final;
    }
};