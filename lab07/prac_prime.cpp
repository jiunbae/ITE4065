#include "pool.h"
#include <iostream>
#include <future>
#include <queue>
#include <cmath>

#define THREAD_NUM	4

int main() {
	Thread::Pool pool(THREAD_NUM);
	std::queue<std::future<size_t>> works;
	int n, m;

	for (;;) {
		std::cin >> n;
		if (n == -1) break;
		std:: cin >> m;
	
		works.emplace(pool.push([](int s, int e) {
			auto isPrime = [](int n) {
				if (n < 2) return false;
				for (int i = 0; i < std::sqrt(n); ++i) {
					if (n % i == 0)
						return false;
				}
				return true;
			};
			int ret = 0;
			for (int x = s; x < e; ++x) {
				ret += isPrime(x);	
			}	
			std::cout << "number of primes in " + std::to_string(s) + " - " + std::to_string(e) + " is " + std::to_string(ret) + "\n";
		}, n, m));
	}

	while (works.size()) {
		int r = works.front().get(); works.pop();
	}

}

