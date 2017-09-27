#include <set>
#include <list>
#include <vector>

#ifndef UNIQUE_VECTOR_H
#define UNIQUE_VECTOR_H

#define UNIQUE_VECTOR_DEFAULT_SIZE 4096

namespace unique {

    template <typename T>
    class vector {
    public:
        typedef std::set<T> _uniquely;
        typedef std::vector<T> _container;
        typedef typename _container::iterator iterator;
        typedef const typename _container::const_iterator const_iterator;

        vector() {
            container.reserve(UNIQUE_VECTOR_DEFAULT_SIZE);
        }
        vector(size_t size) {
            container.reserve(size);
        }
        template<typename Iterable>
        vector(Iterable container) {
            container.reserve(UNIQUE_VECTOR_DEFAULT_SIZE);
            for (const auto& item : container) {
                _insert(item);
            }
        }
        template<typename Iter>
        vector(Iter begin, Iter end) {
            container.reserve(UNIQUE_VECTOR_DEFAULT_SIZE);
            for (Iter it = begin; it != end; ++it) {
                _insert(*it);
            }
        }

        ~vector() {}

        template<typename... Args>
        bool insert(Args... elements) {
            return _insert((elements)...);
        }

        bool insert(const T& element) {
            return _insert(element);
        }

        bool erase(int index) {
            return _erase(container.begin() + index);
        }

        bool erase(const T& element) {
            return _erase(std::find(container.begin(), container.end(), element));
        }

        T& operator[] (int index) {
            return container[index];
        }

        T at(int index) const {
            return container[index];
        }

        int find(const T& element) const {
            return std::distance(container.begin(), find(container.begin(), container.end(), element));
        }

        size_t capacity() const {
            return container.capacity();
        }

        size_t size() const {
            return container.size();
        }

        iterator begin() { return container.begin(); }
        const_iterator begin() const { return container.begin(); }
        iterator end() { return container.end(); }
        const_iterator end() const { return container.end(); }

    private:
        _container container;
        _uniquely inspector;

        bool contain(const T& element) const {
            return inspector.find(element) != inspector.end();
        }

        template <typename... Args>
        bool _insert(const Args... elements) {
            if (contain(T((elements)...)))
                return false;

            container.emplace_back((elements)...);
            inspector.emplace((elements)...);
            return true;
        }

        bool _insert(const T& element) {
            if (contain(element))
                return false;

            container.push_back(element);
            inspector.insert(element);
            return true;
        }

        bool _erase(iterator it) {
            if (it == container.end() || !contain(*it))
                return false;

            inspector.erase(*it);
            container.erase(it);
            return true;
        }
    };

    template <typename T>
    class list {
    public:
        typedef std::set<T> _uniquely;
        typedef std::list<T> _container;
        typedef typename _container::iterator iterator;
        typedef const typename _container::const_iterator const_iterator;

        list() {}
        template<typename Iterable>
        list(Iterable container) {
            for (const auto& item : container) {
                _insert(item);
            }
        }
        template<typename Iter>
        list(Iter begin, Iter end) {
            for (Iter it = begin; it != end; ++it) {
                _insert(*it);
            }
        }

        ~list() {}

        template<typename... Args>
        bool insert(Args... elements) {
            return _insert((elements)...);
        }

        bool insert(const T& element) {
            return _insert(element);
        }

        bool erase(int index) {
            return _erase(std::next(container.begin(), index));
        }

        bool erase(const T& element) {
            return _erase(std::find(container.begin(), container.end(), element));
        }

        T& operator[] (int index) {
            return *std::next(container.begin(), index);
        }

        T at(int index) const {
            return *std::next(container.begin(), index);
        }

        int find(const T& element) const {
            return std::distance(container.begin(), find(container.begin(), container.end(), element));
        }

        size_t size() const {
            return container.size();
        }

        iterator begin() { return container.begin(); }
        const_iterator begin() const { return container.begin(); }
        iterator end() { return container.end(); }
        const_iterator end() const { return container.end(); }

    private:
        _container container;
        _uniquely inspector;

        bool contain(const T& element) const {
            return inspector.find(element) != inspector.end();
        }

        template <typename... Args>
        bool _insert(const Args... elements) {
            if (contain(T((elements)...)))
                return false;

            container.emplace_back((elements)...);
            inspector.emplace((elements)...);
            return true;
        }

        bool _insert(const T& element) {
            if (contain(element))
                return false;

            container.push_back(element);
            inspector.insert(element);
        }

        bool _erase(iterator it) {
            if (it == container.end() || !contain(*it))
                return false;

            inspector.erase(*it);
            container.erase(it);
            return true;
        }
    };
}
#endif
