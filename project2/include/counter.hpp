#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <mutex>
#include <shared_mutex>

namespace thread {
    namespace safe {
        template <typename T>
        class Counter {
        public:
            Counter() = default;

            T get() const {
                std::shared_lock<std::shared_timed_mutex> lock(mutex);
                return value;
            }

            void add(T v) {
                std::unique_lock<std::shared_timed_mutex> lock(mutex);
                value += v;
            }

            void reset() {
                std::unique_lock<std::shared_timed_mutex> lock(mutex);
                value = 0;
            }

        private:
            mutable std::shared_timed_mutex mutex;
            T value = 0;
        };
    }
}

#endif