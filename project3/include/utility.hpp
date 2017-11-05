#ifndef UTILITY_HPP
#define UTILITY_HPP

namespace util {
	template <typename F>
	static void queue_apply(std::queue<F>& container, const std::function<void(F)>& f, size_t size = size_t(0)) {
		while (container.size() > size) {
			auto front = container.front(); container.pop();
			f(front);
		}
	}
}

#endif