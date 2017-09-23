#include <vector>
#include <string>

using namespace std;

#define CHAR_SIZE (26)
#define CHAR_START ('a')

using TABLE = vector<vector<char>>;
using MATCH = vector<int>;

TABLE create_table(const vector<string>& patterns, int& init_state) {
    int table_size = 0;
    for (const auto& pattern : patterns)
        table_size += pattern.length() + 1;

    int pattern_num = 0;
    int state_init = patterns.size();
    int state = state_init;
    int state_num = state_init + 1;
    int state_final = 0;
    int pre_state = 0;
    char pre_char = 0;

    TABLE table = vector<vector<char>>(table_size, vector<char>(CHAR_SIZE, -1));

    for (const auto& pattern : patterns) {
        for (const auto& ch : pattern) {
            if (table[state][ch - CHAR_START] == -1) {
                table[state][ch - CHAR_START] = state_num;
                pre_state = state;
                pre_char = ch - CHAR_START;
                state = state_num;
                state_num = state_num + 1;
            }
            else {
                pre_state = state;
                pre_char = ch - CHAR_START;
                state = table[state][ch - CHAR_START];

            }
        }
        table[pre_state][pre_char] = state_final;
        pattern_num = pattern_num + 1;

        for (int i = 0; i < CHAR_SIZE; i++) {
            table[state_final][i] = table[state][i];
            table[state][i] = -1;
        }

        state_num--;
        state = state_init;
        state_final++;
    }
    init_state = state_init;
    return table;
}

MATCH find_match(const TABLE& table, const string& query, int init_state) {
    unsigned int start;
    unsigned int pos;
    int state;

    MATCH result;

    for (start = 0; start < query.length(); start++) {
        state = init_state;
        pos = start;

        do {
            state = table[state][query[pos] - CHAR_START];
            pos += 1;
            if (state < init_state && state > -1) {
                result.push_back(state);
            }
        } while ((state != -1) && (pos < (size_t)query.length()));
    }

    return result;
}
