#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
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
			: n(n), r(r), e(e), pool(n), counters(r, n, INIT_VALUE), random(0, 0, r - 1)
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

			size_t commit_id = 0;
            do {
                tasks.emplace_back(pool.push([&, e = this->e](size_t id) {
					// Imp: C++ 17 feature, but not on VS17
					// Structured Binding
					// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0144r0.pdf
					// auto [ i, j, k ] = random.next<3>(); 
					size_t i, j, k;
					std::tie(i, j, k) = random.next<3>();

					auto build_id = counters.transaction(id, i, j, k);

					if (!build_id) return;

					auto cid = counters.commit(*build_id, [&l = loggers, tid=id, e](size_t o, size_t i, size_t j, size_t k, int64 x, int64 y, int64 z) -> void {
						if (o <= e)
							l[tid]->safe_write(o, i, j, k, x, y, z);
					});

					if (!cid) return;
					commit_id = *cid;
                }));
            } while (commit_id <= e);

            for (auto& task : tasks) {
                task.get();
            }
        }

    private:
        const size_t n;     // thread count
        const size_t r;     // record count
        const size_t e;      // global execution order

		thread::Pool pool;
        thread::safe::Container<int64> counters;
		util::Random<size_t> random;
		std::vector<Logger *> loggers;

		// DEBUG
		Logger debuger;
    };
}

#endif
