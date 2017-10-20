#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
#include <shared_mutex>
#include <future>

#include <utility.hpp>
#include <logger.hpp>

#include <pool.hpp>
#include <threadsafe.hpp>

#define INIT_VALUE 100

namespace transaction {

    typedef long long int int64;

    class Operator {
    public:
        Operator(size_t n, size_t r, int64 e) noexcept
			: n(n), r(r), e(e), pool(n), counters(r, n, INIT_VALUE, g), order(), random(0, 0, r - 1)
			// DEBUG
			, debuger("test", "txt")
		{
            util::iterate([&l=this->loggers](size_t i) {
                l.push_back(new Logger("thread" + std::to_string(i + 1), "txt"));
            }, n);
        }

        ~Operator() {
            util::iterate([&l=this->loggers](size_t i) {
                delete l[i];
            }, n);
        }

        void process() {
            std::vector<std::future<void>> tasks;

            do {
                tasks.emplace_back(pool.push([&](size_t id) {
					// Imp: C++ 17 feature, but not on VS17
					// Structured Binding
					// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0144r0.pdf
					// auto [ i, j, k ] = random.next<3>(); 
					size_t i, j, k;
					std::tie(i, j, k) = random.next<3>();

					auto build_id = counters.build(id, i, j, k);

					if (build_id) {

					}
					

					/*debuger << std::to_string(id) + " is requested\n" +
						std::to_string(i) + ", " + std::to_string(j) + ", " + std::to_string(k) + " selected!\n";

					int64 x = counters.read(i);
                    int64 y = counters.write(j, x + 1);
                    int64 z = counters.write(k, -x);

					// DEBUG
					debuger << "value from: " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(y) + "\nvalue to  : " +
						std::to_string(x) + ", " + std::to_string(x + y + 1) + ", " + std::to_string(z - x) + "\n";

                    {   // commit stage
                        g.lock();

						counters.release(i);
						counters.release(j);
						counters.release(k);

                        order.add(1);

                        int64 o = order.get();

                        if (o <= e) {
                            loggers[id]->safe_write(o, i, j, k, x, y + x + 1, z - x);
						} else {

						}

                        g.unlock();
                    }*/
					
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

        std::mutex g;

		thread::Pool pool;

        thread::safe::Container<int64> counters;
        thread::safe::Record<int64> order;
		
		util::Random<size_t> random;

		std::vector<Logger *> loggers;

		// DEBUG
		Logger debuger;
    };
}

#endif
