#include "pool.hpp"
#include "list.cc"
#include "random.hpp"
#include <queue>
#include <future>
#include <iostream>

int main() {
    thread::Pool pool(4);
    util::Random<int> random;
    List<int> list;

    std::queue<std::future<void>> q;
    for (int i = 0; i < 10000; ++i){
        q.push(pool.push([&list, v = random.next()](){
            list.insert(v);
        }));   
    }

    while (!q.empty()) {
        q.front().get();
        q.pop();
    }

    std::cout << list.size() << std::endl;

    return 0;
}
