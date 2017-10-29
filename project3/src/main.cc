#include <iostream>
#include <future>
#include <queue>
#include <chrono>
#include <thread>

#include <argparser.hpp>
#include <snapshot.hpp>
#include <register.hpp>
#include <pool.hpp>

int main(int argc, char * argv[]) {
	std::ios::sync_with_stdio(false);
    arg::Parser parser;
    
    parser.argument("N", "thread count");

    parser.parse(argc, argv);

	size_t n = parser.get<size_t>("N");

	{
		atomic::Register<size_t> count(0);
		atomic::Snapshot<int> snapshot(n);
		thread::Pool pool(n);
		std::queue<std::future<void>> tasks;
		
		std::thread time_guard([&pool]() {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			pool.terminate();
		});

		while (!pool.is_stop()) {
			tasks.emplace(pool.push([&snapshot, &count](size_t tid) {
				count.add(1);
			}));
		}

		while (tasks.size()) {
			tasks.front().get();
			tasks.pop();
		}

		time_guard.join();
		std::cout << "update : " << count.get() << '\n';
	}
	return 0;
}