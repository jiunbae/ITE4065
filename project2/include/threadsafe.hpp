#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <vector>
#include <list>

#include <mutex>
#include <shared_mutex>

#include <mutex.hpp>

namespace thread {
    namespace safe {
		template <typename T>
		class Record {
		public:
			Record(std::mutex& global) noexcept
				: global(global) { };

			T get(bool g = true) const {
				if (g) global.lock();
				std::shared_lock<Mutex> lock(mutex);
				if (g) global.unlock();
				return value;
			}

			T add(T v, bool g = true) {
				if (g) global.lock();
				std::unique_lock<Mutex> lock(mutex);
				if (g) global.unlock();
				value += v;
				return value;
			}

			T reset(T v = 0, bool g = true) {
				if (g) global.lock();
				std::unique_lock<Mutex> lock(mutex);
				if (g) global.unlock();
				T origin = value;
				value = v;
				return origin;
			}

		private:
			//mutable std::shared_timed_mutex mutex;
			mutable Mutex mutex;
			std::mutex& global;
			T value = 0;
		};

        template <typename T>
        class Container {
        public:
            Container(size_t size, std::mutex& global)
				: global(global), read_wait(size), write_wait(size) {
                while (size--)
                    records.push_back(new Record<T>(global));
            };

            ~Container() {
                for (auto& e : records)
                    delete e;
            }

			/*
				record refer for capsulize object
				Record<T>& and bool as argument (bool need cuz, global lock checker) return value T
			*/
			T at(size_t index, const std::function<T(Record<T>&, bool)>& f, bool g=true) {
				if (!assert_index(index)) throw std::out_of_range("index out of range");
				return f(*records[index], g);
			}

            T read(size_t index, bool g=true) {
                if (!assert_index(index)) throw std::out_of_range("index out of range");
				append_wait(index, read_wait, read_mutex);
                T value = records[index]->get(g);
				release_wait(index, read_wait, read_mutex);
                return value;
            }

            T write(size_t index, T v, bool g=true) {
                if (!assert_index(index)) throw std::out_of_range("index out of range");
				append_wait(index, write_wait, write_mutex);
                T value = records[index]->add(v, g);
				release_wait(index, write_wait, write_mutex);
                return value;
            }

        private:
            std::mutex& global;
            std::vector<Record<T> *> records;

			std::mutex read_mutex;
			std::mutex write_mutex;
            std::list<size_t> read_wait;
            std::list<size_t> write_wait;

            bool assert_index(size_t index) {
                return 0 <= index && index < records.size();
            }

			void append_wait(size_t index, std::list<size_t>& waiter, std::mutex& mutex) {
				//std::unique_lock<std::mutex> lock(mutex);
				//waiter.push_back(index);
			}

			void release_wait(size_t index, std::list<size_t>& waiter, std::mutex& mutex) {
				//std::unique_lock<std::mutex> lock(mutex);
				//waiter.remove(index);
			}
        };
    }
}

#endif
