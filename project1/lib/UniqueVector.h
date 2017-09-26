#include <vector>
#include <unordered_set>
#include <set>

#ifndef UNIQUE_VECTOR_H
#define UNIQUE_VECTOR_H

#define UNIQUE_VECTOR_DEFAULT_SIZE 256

template <typename T>
class UniqueVector {
public:    
    typedef typename std::vector<T>::iterator iterator;
    typedef const typename std::vector<T>::const_iterator const_iterator;

    UniqueVector() {
        container.reserve(UNIQUE_VECTOR_DEFAULT_SIZE);
    }
    UniqueVector(size_t size) {
        container.reserve(size);
    }
    ~UniqueVector() {}

    bool insert(const T element) {
        return _insert(element);
    }

    bool remove(int index) {
        return _remove(container.begin() + index);
    }

    bool remove(const T& element) {
        return _remove(container.find(element));
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

    size_t size() const {
        return container.size();
    }

    iterator begin() { return container.begin(); }
    const_iterator begin() const { return container.begin(); }
    iterator end() { return container.end(); }
    const_iterator end() const { return container.end(); }

private:
    std::vector<T> container;
    std::set<T> unique;

    bool contain(const T& element) const {
        return unique.find(element) != unique.end();
    }

    bool _insert(const T element) {
        if (contain(element))
            return false;

        container.emplace_back(element);
        unique.insert(element);
        return true;
    }

    template <typename IT>
    bool _remove(IT it) {
        if (!contain(*it))
            return false;

        container.erase(it);
        unique.erase(*it);
        return true;
    }
};

#endif
