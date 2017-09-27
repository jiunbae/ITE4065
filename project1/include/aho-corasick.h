#include <vector>
#include <string>
#include <list>
#include <queue>

#include <pool.h>
#include <unique.h>

#define CHAR_SIZE (26)
#define CHAR_START ('a')
#define DEFAULT_THREAD_SIZE 24
#define DEFAULT_RESERVE_SIZE 4096

class Table {
public:
    Table() {
        init();
    }

    template<typename Iterable>
    Table(Iterable container) {
        init();
        for (const auto& item : container) {
            pre_add.emplace_back(item);
        }
        sync();
    }

    template<typename Iter>
    Table(Iter begin, Iter end) {
        init();
        for (; begin != end; ++begin) {
            pre_add.emplace_back(*begin);
        }
        sync();
    }

    std::queue<std::string>& match(const std::string& query) {
        sync();

        std::fill(results.begin(), results.begin() + query.length(), 0);
        std::fill(uniques.begin(), uniques.begin() + patterns.size(), false);

        for (size_t start = 0, length = query.length(); start < length; start++) {
            pool->push(
                [=](const char * query, size_t length) -> void {
                    size_t pos = 0;
                    for (int state = state_init; state != -1 && pos < length; ++pos) {
                        state = raw[state][query[pos] - CHAR_START];
                        if (-1 < state && state < state_init) {
                            results[length - pos] = state + 1;
                        }
                    }
                }, query.c_str() + start, length - start
            );
        }

        for (auto it = results.rbegin() + (results.capacity() - query.length()); it != results.rend(); ++it) {
            if (*it && !uniques[*it - 1]) {
                fin.push(patterns[*it - 1]);
                uniques[*it - 1] = true;
            }
        }

        return fin;
    }

    bool wrapper(std::queue<std::string>& request, const std::function<void(const std::string&)>& task) {
        if (request.empty())
            return false;

        while (!request.empty()) {
            task(request.front());
            request.pop();
        }
        return true;
    }

    void resize(size_t size) {
        pool = new Thread::Pool(size);
    }

    void add(const std::string& pattern) {
        pre_add.push_back(pattern);
    }

    void remove(const std::string& pattern) {
        pre_rem.push_back(pattern);
    }

    int init_state() const {
        return state_init;
    }

private:
    size_t table_size = 0;
    int state_final = 0;
    int state_init = 0;
    int state_num = 0;
    int state = 0;
    int pre_state = 0;
    char pre_char = 0;

    std::vector<std::vector<int>> raw;
    std::list<std::string> pre_add, pre_rem;
    unique::vector<std::string> patterns;
    std::vector<bool> uniques;
    std::vector<int> results;

    std::queue<std::string> fin;

    Thread::Pool * pool;

    void init() {
        raw.reserve(DEFAULT_RESERVE_SIZE);
        results.reserve(DEFAULT_RESERVE_SIZE);
        uniques.reserve(DEFAULT_RESERVE_SIZE);

        raw = std::vector<std::vector<int>>(DEFAULT_RESERVE_SIZE, std::vector<int>(CHAR_SIZE, -1));
        results = std::vector<int>(DEFAULT_RESERVE_SIZE, 0);
        uniques = std::vector<bool>(DEFAULT_RESERVE_SIZE, false);

        pool = new Thread::Pool(DEFAULT_THREAD_SIZE);
    }

    void sync() {
        size_t _table_size = table_size;

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

        if (_table_size > raw.capacity()) {
            raw.resize(_table_size, std::vector<int>(CHAR_SIZE, -1));
            results.resize(_table_size, 0);
            uniques.resize(_table_size, false);
        }

        table_size = _table_size;

        for (int i = 0; i < state_num + 1; i++) {
            fill(raw[i].begin(), raw[i].end(), -1);
        }

        state_final = 0;
        state = state_init = patterns.size();
        state_num = state_init + 1;

        for (const auto& pattern : patterns) {
            update_table(pattern);
        }

        pre_add.clear();
        pre_rem.clear();
    }

    void update_table(const std::string& pattern) {
        bool check = false;

        for (const auto& ch : pattern) {
            if (raw[state][ch - CHAR_START] == -1) {
                state = raw[pre_state = state][pre_char = ch - CHAR_START] = state_num++;
                check = true;
            }
            else {
                state = raw[pre_state = state][pre_char = ch - CHAR_START];
            }
        }
        raw[pre_state][pre_char] = state_final;
        std::swap(raw[state_final], raw[state]);
        std::fill(raw[state].begin(), raw[state].end(), -1);

        if (check)
            --state_num;
        state = state_init;
        ++state_final;
    }
};
