#ifndef THREAD_SAFE_CONTAINER_HPP
#define THREAD_SAFE_CONTAINER_HPP

#include <deque>
#include <vector>
#include <list>
#include <queue>
#include <functional>
#include <algorithm>
#include <limits>

#include <logger.hpp>
#include <counter.hpp>


#if defined(__GNUC__) && (__GNUC__ < 7)
//Imp: C++17 feature! but not on gcc < 7
//gcc5 and earlier provides an experimental C++ 17 standard from "experimental/"
	// A proposal to add a utility class to represent optional objects
	// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3672.html
	#include <experimental/optional>
	template <typename T>
	using optional = std::experimental::optional<T>;

	// A non-owning reference to a string
	// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3921.html
	#include <experimental/string_view>
	using string_view = std::experimental::string_view;
#elif (defined(_MSC_VER ) && (_MSC_VER  >= 1910)) || defined(__GNUC__) && (__GNUC__ >= 7)
// VS2017.3 supports a broader range of C++ 17 standards with `/std:c++latest` tag
	#include <optional>
	template <typename T>
	using optional = std::optional<T>;

	#include <string_view>
	using string_view = std::string_view;
#else
#endif

namespace thread {
    namespace safe {

		/*
			Container is collection of thread::safe::Record

			support build transaction, commit
			- transaction	build transaction if failed undo all operation
			- commit		commit and merge
		*/
        template <typename T>
        class Container {
			using record = Record<T, thread::safe::Mutex>;
        public:
			/*
				Implement of a single Operation on Record

				support
				- execute	do execution by a predetermined operator
				- undo		undo execution before, throw std::bad_function_call undo before execution
			*/
			class Operation {
			public:
				Operation(size_t tid, size_t rid, Operator op) noexcept
					: operand(nullptr), rid(rid), tid(tid), op(op){
				}

				T execute(record* operand, T value = 0) {
					this->operand = operand;
					switch (op) {
						case Operator::READ:
							return evaluated = origin = operand->get();
						case Operator::WRITE:
							origin = operand->add(value);
							evaluated = origin + value;
							if (assert_overflow(origin, value)) 
								throw std::overflow_error("");
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

				void release() {
					if (operand == nullptr) return;
					operand->release(op);
				}

				record* get_operand(const std::vector<record*>& records) const {
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

			protected:
				record* operand;
				size_t rid;
				size_t tid;
				Operator op;
				T origin;
				T evaluated;

			private:
				static bool assert_overflow(T o, T v) {
					if ((o > 0 && v < 0) || (o < 0 && v > 0)) return false;
					T u_eval = std::numeric_limits<T>::max() - (o > 0 ? o : -o);
					if (u_eval < (v > 0 ? v : -v)) 
						return true;
					return false;
				}
			};

			Container(size_t record_count, size_t thread_count, T init)
				: waiting(record_count) {
				while (record_count--)
					records.push_back(new record(init));
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
							delete get;
							delete add;
							delete sub;
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
							delete get;
							delete add;
							delete sub;
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
				try {
					add->execute(add->get_operand(records), val + 1);
				} catch (std::overflow_error& oe) {
					get->undo();
					get->get_operand(records)->release(get->oper());
					add->undo();
					add->get_operand(records)->release(add->oper());
					delete get;
					delete add;
					delete sub;
					return {};
				}

				{
					std::lock_guard<std::mutex> lock(global);
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
							delete get;
							delete add;
							delete sub;
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
				try {
					sub->execute(sub->get_operand(records), -val);
				}
				catch (std::overflow_error& oe) {
					get->undo();
					get->get_operand(records)->release(get->oper());
					add->undo();
					add->get_operand(records)->release(add->oper());
					sub->undo();
					sub->get_operand(records)->release(sub->oper());
					delete get;
					delete add;
					delete sub;
					return {};
				}

				{
					std::lock_guard<std::mutex> lock(global);
					history.emplace_back(get, add, sub);
					return history.size() - 1;
				}
			}

			optional<size_t> commit(size_t build_id, const std::function<void(size_t, size_t, size_t, size_t, T, T, T)>& f) {
				{
					std::lock_guard<std::mutex> lock(global);
					if (!assert_history(build_id)) throw std::out_of_range("wrong build number");
					// Imp: C++ 17 feature, but not on GCC
					// Structured Binding
					// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0144r0.pdf
					// auto[get, add, sub] = history[build_id];
					Operation *get, *add, *sub;
					std::tie(get, add, sub) = history[build_id];
					

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
            std::vector<record*> records;
			std::deque<std::deque<Operation*>> waiting;
			std::deque<std::tuple<Operation*, Operation*, Operation*>> history;

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
					//while (point->dependency() != nullptr) point = point->dependency();
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
