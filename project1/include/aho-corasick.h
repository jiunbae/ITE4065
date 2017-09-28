#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#include <queue>
#include <functional>
#include <iostream>

#include <unique.h>

#define CHAR_SIZE (26)
#define CHAR_START ('a')

#define _RESERVE_SIZE_ 2048
#define _THREAD_SIZE_ 10

class Table {
public:
    using _return_type = std::queue<std::string>;
    using _raw = std::vector<std::vector<int>>;
    using _u_container = unique::vector<std::string>;

    template<typename Iterable>
    Table(Iterable container) : patterns(container) {
        init();
    }

    template<typename Iterator>
    Table(Iterator begin, Iterator end) : patterns(begin, end) {
        init();
    }

    _return_type& match(const std::string& query) {
        unsigned int start;
        unsigned int pos;
        int state;

        sync();

        std::set<std::string> uniquer;
        for (start = 0; start < query.length(); start++) {
            std::string r = "";
            state = state_init;
            pos = start;
            do {
                r += query[pos];
                state = raw[state][query[pos++] - CHAR_START];
                if (state < state_init && state > -1) {
                    if (uniquer.find(r) == uniquer.end()) {
                        fin.push(r);
                        uniquer.insert(r);
                    }
                }
            } while ((state != -1) && (pos < query.length()));
        }
        return fin;
    }

    bool wrapper(_return_type& request, std::function<void(const std::string&)> task) {
        if (request.empty()) return false;

        while (!request.empty()) {
            task(request.front());
            request.pop();
        }
        return true;
    }

    void add(const std::string& pattern) {
        pre_add.emplace(pattern);
    }

    void remove(const std::string& pattern) {
        pre_rem.emplace(pattern);
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

    _raw raw;
    _return_type fin;
    _u_container patterns;
    std::queue<std::string> pre_add, pre_rem;

    void init() {
        table_size = patterns.rsize();
        state_init = patterns.size();
        state_num = state_init + 1;
        state = state_init;

        raw.reserve(_RESERVE_SIZE_);
        patterns.reserve(_RESERVE_SIZE_);
        
        raw = std::vector<std::vector<int>>(std::max(_RESERVE_SIZE_, table_size), std::vector<int>(CHAR_SIZE, -1));

        for (const auto& pattern : patterns) {
            update_table(pattern);
        }
    }

    void sync() {
        if (pre_add.empty() && pre_rem.empty())
            return;

        while (!pre_add.empty()) {
            patterns.insert(pre_add.front());
            pre_add.pop();
        }

        while (!pre_rem.empty()) {
            patterns.erase(pre_rem.front());
            pre_rem.pop();
        }

        int _table_size = patterns.rsize();

        if (_table_size > raw.capacity()) {
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
    }

    void update_table(const std::string& pattern) {
        int flag = false;
        for (const auto& ch : pattern) {
            if (raw[state][ch - CHAR_START] == -1) {
                state = raw[pre_state = state][pre_char = ch - CHAR_START] = state_num++;
                flag = true;
            } else {
                state = raw[pre_state = state][pre_char = ch - CHAR_START];
            }
        }
        raw[pre_state][pre_char] = state_final;
        swap(raw[state_final], raw[state]);
        fill(raw[state].begin(), raw[state].end(), -1);

        if (flag) --state_num;
        state = state_init;
        ++state_final;
    }
};
