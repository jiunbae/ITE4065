#ifndef UTILITY_HPP
#define UTILITY_HPP

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
        Random(T min, T max) : 
            gen(std::random_device()()), dis(min, max) {}

        T next() {
            return dis(gen);
        }

        template <typename... Args>
        T next(Args... args) {
            T v = next();
            while (contain(v, args...)) v = next();
            return v;
        }

    private:
        std::mt19937 gen;
        std::uniform_int_distribution<T> dis;
    };
}

#endif
