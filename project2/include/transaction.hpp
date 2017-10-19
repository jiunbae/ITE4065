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
			: n(n), r(r), e(e), pool(n), counters(r, g), order(g), random(0, r - 1) {
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
					size_t i, j, k;
					std::tie(i, j, k) = random.next<3>();
					// Imp: C++ 17 feature, but not on VS17
					// Structured Binding
					// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0144r0.pdf
					// auto [ i, j, k ] = random.next<3>(); 

                    int64 x = counters.read(i);
                    int64 y = counters.write(j, x + 1);
                    int64 z = counters.write(k, -x);

                    {   // commit stage
                        g.lock();
                        order.add(1, false);

                        int64 o = order.get(false);

                        if (o <= e) {
                            loggers[id]->safe_write(o, i, j, k, x, y, z);
						} else {

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

        std::mutex g;

		thread::Pool pool;

        thread::safe::Container<int64> counters;
        thread::safe::Counter<int64> order;
		
		util::Random<size_t> random;

		std::vector<Logger *> loggers;
    };
}

#endif
