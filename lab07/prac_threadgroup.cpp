#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define NUM_THREAD_IN_POOL	4

void print() {
	std::cout << "Hi, I'm thread " << boost::this_thread::get_id() << std::endl;
}

int main(void) {
	boost::asio::io_service io;
	boost::thread_group threadpool;
	boost::asio::io_service::work* work = new boost::asio::io_service::work(io);

	for (int i = 0; i < NUM_THREAD_IN_POOL; i++) {
		threadpool.create_thread(boost::bind(
				&boost::asio::io_service::run, &io));
	}

	while (1) {
		io.post(print);
		sleep(1);
	}

	delete work;
	io.stop();

	return 0;
}
