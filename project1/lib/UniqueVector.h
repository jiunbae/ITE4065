#include <vector>
#include <unordered_set>

#ifndef UNIQUE_VECTOR_H
#define UNIQUE_VECTOR_H

#define UNIQUE_VECTOR_DEFAULT_SIZE 256

template <typename T>
class UniqueVector {
public:
    UniqueVector() {
        container.resize(UNIQUE_VECTOR_DEFAULT_SIZE);
        unique.resize(UNIQUE_VECTOR_DEFAULT_SIZE);
    }
    UniqueVector(size_t size) {
        container.resize(size);
        unique.resize(size);
    }
    ~UniqueVector() {}

    bool insert(const T element) {
        return _insert(element);
    }

    bool remove(const int index) {

    }

    bool remove(const T element) {

    }

    T& operator[] (int index) {
        return container[index];
    }

    T at(int index) const {
        return container[index];
    }
private:
    std::vector<T> container;
    std::set<T> unique;

    bool contain(const T& element) const {
        return unique.find(element) != unique.end();
    }

    void _insert(const T element) {
        if (contain(element))
            return false;

        container.emplace_back(element);
        unique.insert(element);
        return true;
    }

    void _remove(int index) {
        if (!contain(at(index)))
            return false;
    }
}

#endif
