#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <vector>
#include <mutex>
#include <shared_mutex>

namespace thread {
    namespace safe {
        template <typename T>
        class Counter {
        public:
            Counter(std::mutex& global) : global(global) { };

            T get(bool g=true) const {
                if (g) global.lock();
                std::shared_lock<std::shared_timed_mutex> lock(mutex);
                if (g) global.unlock();
                return value;
            }

            T add(T v, bool g=true) {
                if (g) global.lock();
                std::unique_lock<std::shared_timed_mutex> lock(mutex);
                if (g) global.unlock();
                value += v;
                return value;
            }

            void reset(bool g=true) {
                if (g) global.lock();
                std::unique_lock<std::shared_timed_mutex> lock(mutex);
                if (g) global.unlock();
                value = 0;
            }

        private:
            mutable std::shared_timed_mutex mutex;
            std::mutex& global;
            T value = 0;
        };

        template <typename T>
        class Container {
            static_assert(Counter<T>::value, "T must be able to be templated by the thread::safe::Counter.");
        public:
            Container(size_t size, std::mutex& global) : elements(size), global(global) {

            };

            T read(size_t index, bool g=true) const {

            }

            T write(size_t index, T v, bool g=true) {

            }

        private:
            std::mutex& global;
            std::vector<Counter<T>> elements;
        };
    }
}

#endif
