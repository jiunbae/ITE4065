#ifndef COUNTER_HPP
#define COUNTER_HPP

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
				: waiting(record_count), visit(thread_count, false) {
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

				bool deadlock = false;
				{
					std::unique_lock<std::mutex> lock(global);
					if (deadlock |= assert_deadlock(tid, i)) {
						return {};
					}
					waiting[get->record_id()].emplace_back(get);
					get->set_dependency(add);
					records[i]->acquire(get->oper());
				}
				T read = get->execute(records[i]);

				{
					std::unique_lock<std::mutex> lock(global);
					if (deadlock |= assert_deadlock(tid, j)) {
						waiting[i].pop_back();
						return {};
					}
					waiting[add->record_id()].emplace_back(add);
					add->set_dependency(sub);
					records[j]->acquire(add->oper());
				}
				add->execute(records[j], read + 1);

				{
					std::unique_lock<std::mutex> lock(global);
					if (deadlock |= assert_deadlock(tid, k)) {
						waiting[i].pop_back();
						waiting[j].pop_back();
						return {};
					}
					waiting[sub->record_id()].emplace_back(sub);
					records[k]->acquire(sub->oper());
				}
				sub->execute(records[k], -read);

				{
					std::unique_lock<std::mutex> lock(global);
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

					waiting[get->record_id()].erase(waiting[get->record_id()].begin());
					waiting[add->record_id()].erase(waiting[add->record_id()].begin());
					waiting[sub->record_id()].erase(waiting[sub->record_id()].begin());
					get->release();
					add->release();
					sub->release();

					count += 1;
					f(count, get->record_id(), add->record_id(), sub->record_id(),
						get->eval(), add->eval(), sub->eval());

					return count;
				}
			}

			T order() {
				std::unique_lock<std::mutex> lock(global);
				return count;
			}

			void release(size_t index) {
				if (!assert_index(index)) throw std::out_of_range("index out of range");

				records[index]->release();
			}

        private:
            std::mutex global;

			T count;
            std::vector<Record<T> *> records;
			std::vector<std::vector<Operation*>> waiting;
			std::vector<std::tuple<Operation*, Operation*, Operation*>> history;
			std::vector<bool> visit;

            bool assert_index(size_t index) {
                return 0 <= index && index < records.size();
            }

			bool assert_history(size_t index) {
				return 0 <= index && index < history.size();
			}

			bool assert_deadlock(size_t thread_id, size_t record_id) {
				if (waiting[record_id].empty()) return false;

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
					if (!assert_arrange) return false;
					point = select(index = *assert_arrange);
					if (point->thread_id() == origin->thread_id()) return true;
				}
				return false;
			}
		};
    }
}

#endif
