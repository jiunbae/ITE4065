#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
#include <shared_mutex>
#include <future>

#include <utility.hpp>
#include <logger.hpp>

#include <pool.hpp>
#include <counter.hpp>

namespace transaction {

    typedef long long int int64;

    class Operator {
    public:
        Operator(size_t n, size_t r, int64 e) : n(n), r(r), e(e), pool(n), random(0, r - 1)  {
            util::iterate([&l=this->loggers, &c=this->counters](size_t i) {
                l.push_back(new Logger("thread" + std::to_string(i), "txt"));
                c.push_back(new thread::safe::Counter<int64>());
            }, n);
        }

        void process() {
            std::vector<std::future<void>> tasks;

            do {
                tasks.emplace_back(pool.push([&](size_t id) {
                    size_t i = random.next();
                    size_t j = random.next(i);
                    size_t k = random.next(i, j);

                    int64 v = counters[i]->get();

                    counters[j]->add(v + 1);
                    counters[k]->add(-v);

                    order.add(1);

                    size_t o = order.get();
                    if (o <= e) {
                        std::string commit_log = std::to_string(o) + " " +
                            std::to_string(i) + " " + std::to_string(j) + " " + std::to_string(k) + " " +
                            std::to_string(v) + " " + std::to_string(counters[j]->get()) + " " + std::to_string(counters[k]->get()) + '\n';

                        (*loggers[id]) << commit_log;
                    }
                }));
            } while (order.get() < e);

            for (auto& task : tasks) {
                task.get();
            }
        }
    private:
        const size_t n;     // thread count
        const size_t r;     // record count
        const int64 e;      // global execution order

        std::vector<Logger *> loggers;
        std::vector<thread::safe::Counter<int64> *> counters;
        thread::safe::Counter<int64> order;
        thread::Pool pool;

        util::Random<size_t> random;
    };
}

#endif
