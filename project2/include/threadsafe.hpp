#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <iostream>

#include <deque>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <functional>
#include <algorithm>

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

			bool try_acquire(Operator op, size_t tid) {
				switch (op) {
				case Operator::READ:
					return mutex.try_lock_shared(tid);
				case Operator::WRITE:
					return mutex.try_lock(tid);
				}
			}

			void acquire(Operator op, size_t tid) {
				switch (op) {
					case Operator::READ:
						mutex.lock_shared(tid);
						break;
					case Operator::WRITE:
						mutex.lock(tid);
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
			//mutable std::shared_timed_mutex mutex;
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

				Record<T>* get_operand(const std::vector<Record<T>*>& records) const {
					return records[rid];
				}

				size_t thread_id() const {
					return tid;
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
				Record<T>* operand;
				size_t rid;
				size_t tid;
				Operator op;
				T origin;
				T evaluated;
			};

			Container(size_t record_count, size_t thread_count, T init)
				: waiting(record_count) {
				while (record_count--)
					records.push_back(new Record<T>(init));
			};

            ~Container() {
				for (auto& story : history) {
					Operation * a, *b, *c;
					std::tie(a, b, c) = story;
					delete a;
					delete b;
					delete c;
				}

                for (auto& e : records)
                    delete e;
            }


			optional<size_t> transaction(size_t tid, size_t i, size_t j, size_t k) {
				Operation* get = new Operation(tid, i, Operator::READ);
				Operation* add = new Operation(tid, j, Operator::WRITE);
				Operation* sub = new Operation(tid, k, Operator::WRITE);

				bool try_failed = false;
				{
					std::lock_guard<std::mutex> lock(global);
					if (!get->get_operand(records)->try_acquire(get->oper(), tid)) {
						if (assert_deadlock(tid, i)) {
							return {};
						}
						try_failed = true;
					}
					waiting[get->record_id()].emplace_back(get);
				}
				if (try_failed) {
					get->get_operand(records)->acquire(get->oper(), tid);
					try_failed = false;
				}
				T val = get->execute(get->get_operand(records));

				{
					std::lock_guard<std::mutex> lock(global);
					get->set_dependency(add);
					if (!add->get_operand(records)->try_acquire(add->oper(), tid)) {
						if (assert_deadlock(tid, j)) {
							for (auto it = waiting[get->record_id()].begin(); it != waiting[get->record_id()].end(); ++it) {
								if (*it == get) {
									waiting[get->record_id()].erase(it);
									break;
								}
							}
							get->undo();
							get->get_operand(records)->release(get->oper());
							return {};
						}
						try_failed = true;
					}
					waiting[add->record_id()].emplace_back(add);
				}
				if (try_failed) {
					add->get_operand(records)->acquire(add->oper(), tid);
					try_failed = false;
				}
				add->execute(add->get_operand(records), val + 1);

				{
					std::lock_guard<std::mutex> lock(global);
					add->set_dependency(sub);
					if (!sub->get_operand(records)->try_acquire(sub->oper(), tid)) {
						if (assert_deadlock(tid, k)) {
							for (auto it = waiting[get->record_id()].begin(); it != waiting[get->record_id()].end(); ++it) {
								if (*it == get) {
									waiting[get->record_id()].erase(it);
									break;
								}
							}
							for (auto it = waiting[add->record_id()].begin(); it != waiting[add->record_id()].end(); ++it) {
								if (*it == add) {
									waiting[add->record_id()].erase(it);
									break;
								}
							}
							get->undo();
							get->get_operand(records)->release(get->oper());
							add->undo();
							add->get_operand(records)->release(add->oper());
							return {};
						}
						try_failed = true;
					}
					waiting[sub->record_id()].emplace_back(sub);
				}
				if (try_failed) {
					sub->get_operand(records)->acquire(sub->oper(), tid);
					try_failed = false;
				}
				sub->execute(sub->get_operand(records), -val);

				{
					std::lock_guard<std::mutex> lock(global);
					history.emplace_back(get, add, sub);
					return history.size() - 1;
				}
			}

			optional<size_t> commit(size_t build_id, const std::function<void(size_t, size_t, size_t, size_t, T, T, T)>& f) {
				{
					std::lock_guard<std::mutex> lock(global);
					Operation *get, *add, *sub;
					std::tie(get, add, sub) = history[build_id];
					last = std::make_tuple(get, add, sub);

					get->get_operand(records)->release(get->oper());
					for (auto it = waiting[get->record_id()].begin(); it != waiting[get->record_id()].end(); ++it) {
						if (*it == get) {
							waiting[get->record_id()].erase(it);
							break;
						}
					}
					add->get_operand(records)->release(add->oper());
					for (auto it = waiting[add->record_id()].begin(); it != waiting[add->record_id()].end(); ++it) {
						if (*it == add) {
							waiting[add->record_id()].erase(it);
							break;
						}
					}
					sub->get_operand(records)->release(sub->oper());
					for (auto it = waiting[sub->record_id()].begin(); it != waiting[sub->record_id()].end(); ++it) {
						if (*it == sub) {
							waiting[sub->record_id()].erase(it);
							break;
						}
					}

					count += 1;

					f(count, get->record_id(), add->record_id(), sub->record_id(),
						get->eval(), add->eval(), sub->eval());

					return count;
				}
			}

			size_t order() {
				size_t v = count;
				return v;
			}

			void release(size_t index) {
				if (!assert_index(index)) throw std::out_of_range("index out of range");

				records[index]->release();
			}

        private:
            std::mutex global;

			T count;
            std::vector<Record<T> *> records;
			std::deque<std::deque<Operation*>> waiting;
			std::deque<std::tuple<Operation*, Operation*, Operation*>> history;
			std::tuple<Operation*, Operation*, Operation*> last;

            bool assert_index(size_t index) {
                return 0 <= index && index < records.size();
            }

			bool assert_history(size_t index) {
				return 0 <= index && index < history.size();
			}

			bool assert_deadlock(size_t thread_id, size_t record_id) {
				if (waiting[record_id].empty()) 
					return false;
				return true;
				auto arrange = [&t = this->waiting](Operation* operation) -> optional<std::pair<size_t, size_t>> {
					size_t record_id = operation->record_id();
					
					auto tar = std::find(t[record_id].begin(), t[record_id].end(), operation);
					if (tar == t[record_id].end()) 
						return {};

					size_t index = std::distance(t[record_id].begin(), tar);
					if (index == 0) 
						return {};
					return std::make_pair(record_id, index);
				};

				auto select = [&t = this->waiting](std::pair<size_t, size_t>& index)->Operation* {
					return t[index.first][index.second -= 1];
				};

				std::pair<size_t, size_t> index = { record_id, waiting[record_id].size() - 1 };
				Operation* origin = waiting[record_id].back();
				Operation* point = waiting[record_id].back();
				
				while (index.second) {
					while (point->dependency() != nullptr) point = point->dependency();
					auto assert_arrange = arrange(point);
					if (!assert_arrange) 
						return true;
					point = select(index = *assert_arrange);
					if (point->thread_id() == origin->thread_id()) 
						return true;
				}
				return true;
			}
		};
    }
}

#endif
