#include <vector>

#include <pool.hpp>
#include <container.hpp>

namespace atomic {
    template <typename T>
    class Snapshot {
    public:
        using typedef _collection = std::vector<T>;
        Snapshot(size_t n)
            : pool(n), records(n) {
        }

        ~Snapshot() {
        }

        T update(T v) {
            size_t index = getThreadIndex();
            LValue old_value = records[i].get();
            LValue new_value = new LValue(old_value + 1, v);
            records[i].set(new_value);
        }

        _collection scan() {

        }
    private:
        thread::Pool pool;
        thread::safe::Record<T> records;
    };
}