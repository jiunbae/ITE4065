#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <functional>

#include <mutex>
#include <shared_mutex>

#include <logger.hpp>
#include <mutex.hpp>

/*
	Imp: C++17 feature! but not on gcc < 7
	gcc5 and earlier provides an experimental C++ 17 standard from "experimental/"
*/
#if defined(__GNUC__) && (__GNUC__ < 7)
	// A proposal to add a utility class to represent optional objects
	// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3672.html
	#include <experimental/optional>
	template <typename T>
	using optional = std::experimental::optional<T>;

	// A non-owning reference to a string
	// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3921.html
	#include <experimental/string_view>
	using string_view = std::experimental::string_view;
#else // VS2017.3 supports a broader range of C++ 17 standards with `/std:c++latest` tag
	#include <optional>
	template <typename T>
	using optional = std::optional<T>;

	#include <string_view>
	using string_view = std::string_view;
#endif

namespace thread {
    namespace safe {
		enum Operator { READ, WRITE };

		template <typename T>
		class Record {
		public:
			Record(T value = T(0)) noexcept
				: value(value) {
			}

			void acquire(Operator op) {
				switch (op) {
					case Operator::READ:
						mutex.lock_shared();
						break;
					case Operator::WRITE:
						mutex.lock();
						break;
				}
			}

			void release(Operator op) {
				switch (op) {
					case Operator::READ:
						mutex.unlock_shared();
						break;
					case Operator::WRITE:
						mutex.unlock();
						break;
				}
			}

			T get() {
				return value;
			}

			T add(T v) {
				T origin = value;
				value += v;
				return origin;
			}

			T reset(T v = 0) {
				T origin = value;
				value = v;
				return origin;
			}

		private:
			mutable thread::safe::Mutex mutex;
			T value = 0;
		};

        template <typename T>
        class Container {
        public:
			class Operation {
			public:
				Operation(size_t tid, size_t rid, Operator op) noexcept
					: operand(nullptr), rid(rid), tid(tid), op(op){
				}

				T execute(Record<T>* operand, T value = 0) {
					executed = Timestamp();
					this->operand = operand;
					switch (op) {
						case Operator::READ:
							return evaluated = origin = operand->get();
						case Operator::WRITE:
							origin = operand->add(value);
							evaluated = origin + value;
							return origin;
					}
					return T(0);
				}

				void undo() {
					if (operand == nullptr) throw std::bad_function_call();
					switch (op) {
						case Operator::READ: return;
						case Operator::WRITE:
							operand->reset(origin);
					}
				}

				void set_dependency(Operation* d) {
					depend = d;
				}

				void release() {
					if (operand == nullptr) return;
					operand->release(op);
				}

				T eval() const {
					return evaluated;
				}

				Operator oper() const {
					return op;
				}

				size_t record_id() const {
					return rid;
				}

				Operation* dependency() const {
					return depend;
				}

			protected:
				Operation* depend;
				Timestamp requested, executed;
				Record<T>* operand;
				size_t rid;
				size_t tid;
				Operator op;
				T origin;
				T evaluated;
			};

			Container(size_t record_count, size_t thread_count, T init)
				: q(record_count), visit(thread_count, false) {
				while (record_count--)
					records.push_back(new Record<T>(init));
			};

            ~Container() {
                for (auto& e : records)
                    delete e;
            }

			/*optional<size_t> build(std::queue<Operation*> tasks) {
				std::stack<Operation*> done;

				while (!tasks.empty()) {
					Operation* operation = tasks.front(); tasks.pop();

					{
						std::unique_lock<std::mutex> lock(global);
						bool deadlock = assert_deadlock(operation);
						if (deadlock) return {};
						records[operation->record_id()]->acquire(operation->oper());
					}


					done.push(operation);
				}
			}*/

			optional<size_t> transaction(size_t tid, size_t i, size_t j, size_t k) {
				Operation* get = new Operation(tid, i, Operator::READ);
				Operation* add = new Operation(tid, j, Operator::WRITE);
				Operation* sub = new Operation(tid, k, Operator::WRITE);

				add->set_dependency(sub);
				get->set_dependency(add);

				{
					std::unique_lock<std::mutex> lock(global);

					bool deadlock = false;

					if (deadlock |= assert_deadlock(get)) return {};
					T read = get->execute(records[i]);

					if (deadlock |= assert_deadlock(add)) {
						get->undo();
						return {};
					}
					add->execute(records[j], read + 1);

					if (deadlock |= assert_deadlock(sub)) {
						get->undo();
						add->undo();
						return {};
					}
					sub->execute(records[k], -read);

					history.emplace_back(get, add, sub);
					return history.size() - 1;
				}
			}

			optional<size_t> commit(size_t build_id, const std::function<void(size_t, size_t, size_t, size_t, T, T, T)>& f) {
				if (!assert_history(build_id)) throw std::out_of_range("build out of range");
				{
					std::unique_lock<std::mutex> lock(global);

					Operation* get, *add, *sub;
					std::tie(get, add, sub) = history[build_id];

					get->release();
					add->release();
					sub->release();

					T o = order.add(1) + 1;
					f(o, get->record_id(), add->record_id(), sub->record_id(),
						get->eval(), add->eval(), sub->eval());

					return o;
				}
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

        private:
            std::mutex global;

			Record<T> order;
            std::vector<Record<T> *> records;
			std::vector<std::vector<Operation*>> q;
			std::vector<std::tuple<Operation*, Operation*, Operation*>> history;
			std::vector<bool> visit;

            bool assert_index(size_t index) {
                return 0 <= index && index < records.size();
            }

			bool assert_history(size_t index) {
				return 0 <= index && index < history.size();
			}

			bool assert_deadlock(Operation* operation) {
				std::fill(visit.begin(), visit.end(), false);

				size_t rid = operation->record_id();
				q[rid].emplace_back(operation);

				return [&t = this->q](size_t i) -> bool {
					auto selector = [&t](const std::pair<size_t, size_t>& i) -> Operation* {
						return t[i.first][i.second];
					};
					std::pair<size_t, size_t> index = { i, t[i].size() - 1 };
					Operation* origin = selector(index);
					Operation* operation = selector(index);

					while (index.second) {
						while (operation->dependency() != nullptr)
							operation = operation->dependency();
						index.second -= 1;
						operation = selector(index);
						if (origin == operation)
							return true;
					};
					return false;
				}(rid);
			}

			optional<size_t> assert_deadlock(size_t tid, size_t i, size_t j, size_t k) {	
				global.lock();

				std::fill(visit.begin(), visit.end(), false);

				q[i].emplace_back(tid, i, Operation::Operator::GET);
				q[j].emplace_back(tid, j, Operation::Operator::ADD, &q[i].back());
				q[k].emplace_back(tid, k, Operation::Operator::ADD, &q[i].back());

				bool deadlock = [&t = this->q](size_t i) -> bool {
					auto selector = [&t](const std::pair<size_t, size_t>& i) -> Operation* {
						return &t[i.first][i.second];
					};
					std::pair<size_t, size_t> index = { i, t[i].size() - 1 };
					Operation* origin = selector(index);
					Operation* operation = selector(index);

					while (index.second) {
						while (operation->dependency() != nullptr)
							operation = operation->dependency();
						index.second -= 1;
						operation = selector(index);
						if (origin == operation)
							return true;
					};
					return false;
				}(i);

				if (deadlock) return {};

				size_t h = history.size();
				history.emplace_back(std::make_tuple<Operation*, Operation*, Operation*>
					(&q[i].back(), &q[j].back(), &q[k].back())
				);

				global.unlock();
				return h;
			}
		};
    }
}

#endif
