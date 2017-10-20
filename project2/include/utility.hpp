#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <unordered_set>

#include <functional>
#include <random>
#include <limits>

namespace util {
    void repeat(std::function<void(void)>&& f, size_t n) {
        while (n--) f();
    }

    void iterate(std::function<void(size_t)>&& f, size_t n) {
        for (size_t i = 0; i < n; ++i) f(i);
    }

    template <typename T, typename U>
    bool contain(T t, U u) {
        return t == u;
    }

    template <typename T, typename U, typename... Args>
    bool contain(T t, U u, Args... args) {
        return t == u + contain(t, args...);
    }

    template <typename T>
    class Random {
    public:
		Random(std::mt19937::result_type seed, T min, T max) noexcept
			: gen(seed), dis(min, max) {
		}

        Random(T min, T max) noexcept
			: gen(std::random_device()()), dis(min, max) {
		}

        T next() { return dis(gen); }

		template <size_t N>
		auto next() {
			std::unordered_set<T> ret;
			while (ret.size() < N) {
                T v = next();
                if (ret.find(v) == ret.end())
                    ret.insert(v);

                // Imp: this is C++17 feature! but not on gcc5.4
                // If statement with initializer
                // @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0305r0.html
                // if (T v = next(); ret.find(v) == ret.end())
                //     ret.insert(v)
            }

            std::vector<T> v(ret.begin(), ret.end());
			return tuple_from_vector(v, std::make_index_sequence<N>());
		}

    private:
        std::mt19937 gen;
        std::uniform_int_distribution<T> dis;

        // Imp: C++14 feature!
        // Generic lambda-capture initializers
        // @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3610.html
        // template <size_t... Indices>
        // auto tuple_from_set(const std::unordered_set<T>& s, std::index_sequence<Indices...>) {
        //     return std::make_tuple([&b = c.begin()](size_t v) -> T {
        //         std::advance(b, v);
        //         return *(b);
        //     } (Indices)...);
        // }
        // According to the standard, It must work but not on gcc, cuz const reference capture problem
        // It can reduce create temporary vector for make tuple from elements in runtime.

		template <size_t... Indices>
		auto tuple_from_vector(const std::vector<T>& v, std::index_sequence<Indices...>) {
            return std::make_tuple(v[Indices]...);
		}
    };
}

#endif
