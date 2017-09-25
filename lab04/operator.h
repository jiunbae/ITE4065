#include <utility>

#include <iostream>
#include "pool.h"

#define DEFAULT_THREAD_SIZE 10

using task_return_type = int;
using task_type = std::function<task_return_type(int)>;

class Operator {
public:
    Operator() {
        threadpool = new Thread::Pool(DEFAULT_THREAD_SIZE);
    }
    Operator(size_t size) {
        threadpool = new Thread::Pool(size);
    }

    void set(task_type t) {
        task = t;
    }

    int run(int s, int e) {
        int token = (e - s) / (threadpool->size() * 2);
        if (token <= 0) token = 1;

        std::vector< std::future<int> > results;
        for (int i = s; i <= e; i += token) {
            results.emplace_back(threadpool->push([this](int s, int e) -> int {
                int sum = 0;
                if (!(s % 2)) s += 1;
                for (int i = s; i < e; i += 2) {
                    sum += this->task(i);
                }
                return sum;
            }, i, std::min(i + token - 1, e)));
        }

        int total = 0;
        for (auto&& result : results) {
            total += result.get();
        }
        return total;
    }

private:
    Thread::Pool * threadpool;
    std::function<int(int)> task;
};