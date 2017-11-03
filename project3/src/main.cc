#include <iostream>
#include <future>
#include <queue>
#include <chrono>
#include <thread>

#include <argparser.hpp>
#include <snapshot.hpp>
#include <register.hpp>
#include <pool.hpp>
#include <random.hpp>

int main(int argc, char * argv[]) {
	std::ios::sync_with_stdio(false);
    arg::Parser parser;

	parser.argument("N", "thread count");
	parser.argument("T", "set time");

    parser.parse(argc, argv);

	size_t n = parser.get<size_t>("N");
	size_t t = parser.get<size_t>("T", 10);

	// main thread scope
	{
		atomic::Snapshot<int> snapshot(n);		// snapshot instance
		thread::Pool pool(n);					// thread::Pool for multi thread
		util::Random<int> random;				// util::Random generator
		size_t count = size_t(0);
		std::queue<std::future<void>> tasks;	// thread::Pool tasks
		
		// time guard to run only the set time
		std::thread time_guard([&pool, &t]() {
			std::this_thread::sleep_for(std::chrono::seconds(t));
			pool.terminate();
		});

		// push tasks to thread::Pool until set time
		while (!pool.is_stop()) {
			tasks.emplace(pool.push([&pool, &snapshot, &count, &random](size_t tid) {
				snapshot.update(tid, random.next());
			}));
			count += 1;
		}
		
		std::cout << "update : " << count << '\n';
		time_guard.join();

		while (tasks.size()) {
			tasks.front().get();
			tasks.pop();
		}
	}
	return 0;
}