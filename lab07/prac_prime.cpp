#include "pool.h"
#include <iostream>
#include <future>
#include <queue>

#define THREAD_NUM	4

int main() {
	Thread::Pool pool(THREAD_NUM);
	Thread::Pool ost(1);
	std::queue<std::future<void>> tasks;
	std::queue<std::future<void>> ost_task;
	int n, m;

	for (;;) {
		std::cin >> n;
		if (n == -1) break;
		std:: cin >> m;
	
		tasks.emplace(pool.push(
	        [&o = ost, &ot = ost_task](int s, int e) {
				int ret = 0;
				for (int x = s; x < e; ++x) {
					ret += [](int n) -> int {
						if (n < 2) return 0;
						for (int i = 2; i < (int)(n / 2); ++i) {
							if (n % i == 0)
								return 0;
						}
						return 1;
					}(x);
				}
				ot.emplace(o.push([](int s, int e, int ret){
					std::cout << "number of primes in " + std::to_string(s) + " - " + std::to_string(e) + " is " + std::to_string(ret) + "\n";
				}, s, e, ret));
    	}, n, m));

        while (!tasks.empty()) {
            tasks.front().get();
            tasks.pop();
        }

        while (!ost_task.empty()) {
        	ost_task.front().get();
        	ost_task.pop();
        }
	}
}
