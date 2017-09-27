#include <vector>
#include <string>
#include <list>
#include <pool.h>
#include <unique.h>

#define CHAR_SIZE (26)
#define CHAR_START ('a')
#define DEFAULT_THREAD_SIZE 10
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

    std::list<std::string> match(const std::string& query) {
        sync();
        std::vector<std::pair<int, std::future<std::list<int>>>> tasks;

        for (size_t start = 0, length = query.length(); start < length; start++) {
            tasks.emplace_back(start, pool->push(
                [=](const char * query, size_t length) -> std::list<int> {
                    std::list<int> result;
                    int state = state_init;
                    size_t pos = 0;
                    do {
                        state = raw[state][query[pos++] - CHAR_START];
                        if (state < state_init && state > -1)
                            result.push_back(state);
                    } while ((state != -1) && (pos < length));
                    return result;
                }, query.c_str() + start, length - start)
            );
        }

        std::vector<std::pair<int, std::list<int>>> results;
        for (auto&& task : tasks) {
            results.emplace_back(task.first, task.second.get());
        }

        std::sort(results.begin(), results.end());

        std::list<std::string> tq;
        for (const auto& result : results) {
            for (const auto& item : result.second) {
                if (!pCheck[item]) {
                    tq.push_back(patterns[item]);
                    pCheck[item] = true;
                }
            }
        }
        return tq;
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

    unique::vector<std::string> patterns;
    std::vector<bool> pCheck;
    std::vector<std::vector<int>> raw;
    std::list<std::string> pre_add, pre_rem;

    Thread::Pool * pool;

    void init() {
        raw.reserve(DEFAULT_RESERVE_SIZE);
        raw = std::vector<std::vector<int>>(DEFAULT_RESERVE_SIZE, std::vector<int>(CHAR_SIZE, -1));
        pCheck = std::vector<bool>(DEFAULT_RESERVE_SIZE, false);
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

        std::fill(pCheck.begin(), pCheck.begin() + patterns.size(), false);

        pre_add.clear();
        pre_rem.clear();
    }

    void update_table(const std::string& pattern) {
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
