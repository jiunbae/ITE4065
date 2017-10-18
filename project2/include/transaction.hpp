#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
#include <shared_mutex>
#include <future>

#include <utility.hpp>
#include <logger.hpp>

#include <pool.hpp>
#include <threadsafe.hpp>

namespace transaction {

    typedef long long int int64;

    class Operator {
    public:
        Operator(size_t n, size_t r, int64 e)
        : n(n), r(r), e(e), counters(r, g), order(g), pool(n), random(0, r - 1) {
            util::iterate([&l=this->loggers, &g=this->g](size_t i) {
                l.push_back(new Logger("thread" + std::to_string(i), "txt"));
            }, n);
        }

        ~Operator() {
            util::iterate([&l=this->loggers, &c=this->counters](size_t i) {
                delete l[i];
            }, n);
        }

        void process() {
            std::vector<std::future<void>> tasks;

            do {
                tasks.emplace_back(pool.push([&](size_t id) {
                    size_t i = random.next();
                    size_t j = random.next(i);
                    size_t k = random.next(i, j);

                    int64 x = counters.read(i);
                    int64 y = counters.write(j, x + 1);
                    int64 z = counters.write(k, -x);

                    {   // commit stage
                        g.lock();
                        order.add(1, false);

                        int64 o = order.get(false);

                        if (o <= e) {
                            loggers[id]->safe_write(o, i, j, k, x, y, z);
                        }

                        g.unlock();
                    }

                }));
            } while (order.get(false) < e);

            for (auto& task : tasks) {
                task.get();
            }
        }
    private:
        const size_t n;     // thread count
        const size_t r;     // record count
        const int64 e;      // global execution order

        std::vector<Logger *> loggers;
        std::mutex g;
        thread::safe::Container<int64> counters;
        thread::safe::Counter<int64> order;
        thread::Pool pool;

        util::Random<size_t> random;
    };
}

#endif
