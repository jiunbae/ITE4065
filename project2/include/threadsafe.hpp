#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <vector>
#include <list>
#include <queue>

#include <mutex>
#include <shared_mutex>

#include <logger.hpp>
#include <mutex.hpp>

#if defined(__GNUC__) && (__GNUC__ < 7)
	#include <experimental/optional>
	template <typename T>
	using optional = std::experimental::optional<T>;
#else
	#include <optional>
	template <typename T>
	using optional = std::optional<T>;
#endif

namespace thread {
    namespace safe {
		template <typename T>
		class Record {
		public:
			Record(T value = T(0)) noexcept
				: value(value) {
			}

			T get() {
				mutex.lock_shared();

				return value;
			}

			T add(T v) {
				mutex.lock();

				T origin = value;
				value += v;
				return origin;
			}

			T reset(T v = 0) {
				mutex.lock();

				T origin = value;
				value = v;
				return origin;
			}

			void release() {
			}

		private:
			mutable thread::safe::Mutex mutex;
			T value = 0;
		};

        template <typename T>
        class Container {
        public:
			Container(size_t record_count, size_t thread_count, T init, std::mutex& global)
				: global(global), q(record_count), visit(thread_count, false), global_count(0) {
				while (record_count--)
					records.push_back(new Record<T>(init));
			};

            ~Container() {
                for (auto& e : records)
                    delete e;
            }

			std::experimental::optional<size_t> build(size_t tid, size_t i, size_t j, size_t k) {
				if (assert_deadlock(tid, i, j, k)) {
					return 0;
				} else {
					return {};
				}
				
			}

			std::string&& commit(size_t build_id) {

			}

            T read(size_t index, bool g=true) {
                if (!assert_index(index)) throw std::out_of_range("index out of range");
                T value = records[index]->get();
                return value;
            }

            T write(size_t index, T v, bool g=true) {
                if (!assert_index(index)) throw std::out_of_range("index out of range");
                T value = records[index]->add(v);
                return value;
            }

			void release(size_t index) {
				if (!assert_index(index)) throw std::out_of_range("index out of range");

				records[index]->release();
			}

		protected:
			class Operation {
			public:
				enum Operator { GET, ADD };

				Operation(size_t tid, size_t index, Operator op, Operation* depend =nullptr) noexcept
					: index(index), tid(tid), op(op), requested(), depend(depend) {
				}

				void set_dependency(Operation* d) {
					depend = d;
				}

				Operation* dependency() {
					return depend;
				}

			protected:
				Operation* depend;
				Timestamp requested, executed;
				size_t index;
				size_t tid;
				Operator op;
			};

        private:
			size_t global_count;
            std::mutex& global;
            std::vector<Record<T> *> records;
			std::vector<std::vector<Operation>> q;
			std::vector<bool> visit;
				
            bool assert_index(size_t index) {
                return 0 <= index && index < records.size();
            }

			bool assert_deadlock(size_t tid, size_t i, size_t j, size_t k) {	
				global.lock();

				/*std::fill(visit.begin(), visit.end(), false);

				q[i].emplace_back(tid, i, Operation::Operator::GET);
				q[j].emplace_back(tid, j, Operation::Operator::ADD, &q[i].back());
				q[k].emplace_back(tid, k, Operation::Operator::ADD, &q[i].back());

				bool deadlock = [&t = this->q](size_t i) -> bool {
					std::pair<size_t, size_t> index = { i, t[i].size() - 1 };
					index = { 2, 1 };
					auto selector = [&t](const std::pair<size_t, size_t>& i) -> Operation* {
						return &t[i.first][i.second];
					};
					Operation* origin = selector(index);
					Operation* operation = selector(index);

					do {
						if (index.second == 0)
							return false;
						while (operation->dependency() != nullptr)
							operation = operation->dependency();
						index.second -= 1;
						operation = selector(index);
						if (origin == operation)
							return true;
					} while (true);
				}(i);

				global.unlock();*/

				return true;
			}
		};
    }
}

#endif
