#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time>

void print(const boost::system::error_code& e) {
	std::cout << "Hello, Err!" << std::endl;
}

int main(void) {
	boost::asio::io_service io;
	
	boost::asio::deadline_timer t(io, boost::posix_time::seconds(2));
	t.async_wait(&print);
	printf("after async_wait\n");
	io.run();
	printf("after io.run()\n");
	return 0;
}
