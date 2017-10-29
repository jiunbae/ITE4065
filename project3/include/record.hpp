#ifndef THREAD_SAFE_COUNTER_HPP
#define THREAD_SAFE_COUNTER_HPP

#include <mutex.hpp>

namespace thread {
	namespace safe {
		enum Operator { READ, WRITE };

		/*
			A Record support mutex

			acquire to get lock(with Operator)
			try_acquire to try lock(with Operator)
			release to unlock

			support operation
			-	get
			-	add
			-	reset
		*/
		template <typename T, typename M>
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
				return false;
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
			mutable M mutex;
			T value = 0;
		};
	}
}

#endif