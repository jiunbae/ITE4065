#include <iostream>
#include <future>
#include <queue>
#include <chrono>

#include <pool.hpp>
#include <random.hpp>

#include <list.h>

int main(int argc, char * argv[]) {
	std::ios::sync_with_stdio(false);

	lockfree::List<int, lockfree::Node<int>*> list;
	thread::Pool pool(1);
	util::Random<int> random;
	size_t test_size = 400000;

	std::queue<std::future<void>> tasks;

	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < test_size; ++i) {
		tasks.push(pool.push([&list, q=i%3, v=random.next()](int tid) -> void {
			if		(q == 0)	list.insert(&v);
			else if (q == 1)	list.remove(&v);
			else				list.contains(&v);
		}));
	}

	while (!tasks.empty()) {
		tasks.front().get();
		tasks.pop();
	}

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

	std::cout << duration << std::endl;
	return 0;
}