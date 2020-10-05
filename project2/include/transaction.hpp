#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
#include <future>

#include <utility.hpp>
#include <logger.hpp>

#include <pool.hpp>
#include <container.hpp>

#define INIT_VALUE 100
#define TASK_DIV 16

namespace transaction {

    typedef long long int int64;

    class Operator {
    public:
        Operator(size_t n, size_t r, int64 e) noexcept
            : n(n), r(r), e(e), pool(n), counters(r, n, INIT_VALUE), random(0, 0, r - 1) {
            // make n logger in main thread
            // By design, it is correct for the main thread to manage all logs.
            // However, because of the limited of file descriptors, a single thread can only under 512(or similar), 
            // it will not work properly on more than limitation thread(512, my test)
            util::iterate([&l=this->loggers](size_t i) {
                l.push_back(new Logger("thread" + std::to_string(i + 1), "txt"));
            }, n);
        }

        ~Operator() {
            // release Loggers
            util::iterate([&l=this->loggers](size_t i) {
                delete l[i];
            }, n);
        }

        void process() {
            std::queue<std::future<void>> tasks;

            do {
                // The work is divided into chunks.
                // Use thread::Pool to handle divided tasks.
                if (tasks.size() < (e / 16) + 1) {
                    tasks.emplace(pool.push([&](size_t id) {
                        // Imp: C++ 17 feature, but not GCC
                        // Structured Binding
                        // @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0144r0.pdf
                        // auto [ i, j, k ] = random.next<3>(); 
                        size_t i, j, k;
                        std::tie(i, j, k) = random.next<3>();

                        if (counters.order() > e)  return;

                        auto build_id = counters.transaction(id, i, j, k);

                        // case build failed
                        // counter.transaction guarantees that the processed operation correctness.
                        if (!build_id) return;

                        // counter.commit execute function with arguments (commit id, i, j, k, i_val, j_val, k_val)

                        size_t oe = e;
                        auto commit_id = counters.commit(*build_id, [&l = loggers, tid = id, e = oe](size_t o, size_t i, size_t j, size_t k, int64 x, int64 y, int64 z) -> void {
                            if (o > e) return;
                            l[tid]->safe_write(o, i, j, k, x, y, z);
                        });

                        // case commit failed
                        if (!commit_id) return;

                    }));
                } else {
                    // with std::future, Jobs can be pending and processed in parallel.
                    while (!tasks.empty()) {
                        tasks.front().get();
                        tasks.pop();
                    }
                }
            } while (counters.order() < e);

            while (!tasks.empty()) {
                tasks.front().get(); 
                tasks.pop();
            }
        }

    private:
        const size_t n;     // thread count
        const size_t r;     // record count
        const size_t e;     // global execution order

        thread::Pool pool;
        thread::safe::Container<int64> counters;
        util::Random<size_t> random;
        std::vector<Logger *> loggers;
    };
}

#endif
