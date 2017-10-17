#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

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
            util::iterate([&loggers = this->loggers, &counters = this->counters](size_t i) {
                loggers.emplace_back(Logger("thread" + std::to_string(i), "txt"));
                counters.push_back(new thread::safe::Counter<int64>());
            }, n);
        }

        void process() {
            std::vector<std::future<void>> tasks;

            do {
                tasks.emplace_back(pool.push([&o = this->order, &c = this->counters, &l = this->loggers, &r = this->r, &rand = this->random]() {
                    size_t i = rand.next();
                    size_t j = rand.next(i);
                    size_t k = rand.next(i, j);

                    c[j]->add(c[i]->get() + 1);
                    c[k]->add(-c[i]->get());

                    o.add(1);

                    //l[id].write(o.get(), i, j, k, c[i].get(), c[j].get(), c[k].get());
                }));
            } while (order.get() < e);

            for (auto& task : tasks) {
                task.get();
            }
        }
    private:
        size_t n;       // thread count
        size_t r;       // record count
        int64 e;        // global execution order

        std::vector<Logger> loggers;
        std::vector<thread::safe::Counter<int64> *> counters;
        thread::safe::Counter<int64> order;
        thread::Pool pool;

        util::Random<size_t> random;
    };
}

#endif
