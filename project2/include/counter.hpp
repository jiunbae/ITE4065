#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <mutex>
#include <shared_mutex>

namespace thread {
    namespace safe {
        class Lock {
        public:
            Lock() = default;

            void lock() {
                mutex.lock();
            }

            void release() {
                mutex.unlock();
            }
        private:
            std::mutex mutex;
        };

        template <typename T>
        class Counter {
        public:
            Counter(Lock& global) : global(global) { };

            T get(bool g=true) const {
                if (g) global.lock();
                std::shared_lock<std::shared_timed_mutex> lock(mutex);
                if (g) global.release();
                return value;
            }

            T add(T v, bool g=true) {
                if (g) global.lock();
                std::unique_lock<std::shared_timed_mutex> lock(mutex);
                if (g) global.release();
                value += v;
                return value;
            }

            void reset(bool g=true) {
                if (g) global.lock();
                std::unique_lock<std::shared_timed_mutex> lock(mutex);
                if (g) global.release();
                value = 0;
            }

        private:
            mutable std::shared_timed_mutex mutex;
            Lock& global;
            T value = 0;
        };
    }
}

#endif
