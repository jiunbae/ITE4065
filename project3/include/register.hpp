#ifndef ATOMIC_REGISTER_HPP
#define ATOMIC_REGISTER_HPP

#include <mutex>
#include <shared_mutex>

namespace atomic {
	/*
		A Multiple Readers Single Writer register

		support operation
		-	get
		-	add
		-	reset
	*/
	template <typename T, typename M = std::shared_timed_mutex>
	class Register {
	public:
		Register(T value = T(0)) noexcept
			: value(value) {
		}

		T get() const {
			std::shared_lock<M> lock(mutex);
			return value;
		}

		T add(T v) {
			std::unique_lock<M> lock(mutex);
			T origin = value;
			value += v;
			return origin;
		}

		T reset(T v = 0) {
			std::unique_lock<M> lock(mutex);
			T origin = value;
			value = v;
			return origin;
		}

	private:
		mutable M mutex;
		T value;
	};
}
#endif