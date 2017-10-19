#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

//#define USE_STRAND

#ifndef USE_STRAND
void Print(const boost::system::error_code& e,
           boost::asio::deadline_timer* t,
           int* count) {
    if (*count < 10) {
        std::cout << *count << std::endl;
        ++(*count);

        t->expires_at(t->expires_at() + boost::posix_time::seconds(1));

        t->async_wait(boost::bind(Print,
                    boost::asio::placeholders::error, t, count));
    }
}

int main(void) {
    boost::asio::io_service io;

    int count = 0;

    boost::asio::deadline_timer t1(io, boost::posix_time::seconds(1));
    t1.async_wait(boost::bind(Print,
                boost::asio::placeholders::error, &t1, &count));

    boost::asio::deadline_timer t2(io, boost::posix_time::seconds(1));
    t2.async_wait(boost::bind(Print,
                boost::asio::placeholders::error, &t2, &count));

    boost::thread thd(boost::bind(&boost::asio::io_service::run, &io));
    io.run();
    thd.join();

    std::cout << "Final count is " << count << std::endl;

    return 0;
}

#else
void Print(const boost::system::error_code& e,
           boost::asio::deadline_timer* t,
           int* count,
           boost::asio::io_service::strand* strand) {
    if (*count < 10) {
        std::cout << *count << std::endl;
        ++(*count);

        t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
        
        t->async_wait(strand->wrap(boost::bind(Print,
                    boost::asio::placeholders::error, t, count, strand)));
    }
}

int main(void) {
    boost::asio::io_service io;

    int count = 0;

    boost::asio::io_service::strand strand(io);

    boost::asio::deadline_timer t1(io, boost::posix_time::seconds(1));
    t1.async_wait(strand.wrap(boost::bind(Print,
                boost::asio::placeholders::error, &t1, &count, &strand)));

    boost::asio::deadline_timer t2(io, boost::posix_time::seconds(1));
    t2.async_wait(strand.wrap(boost::bind(Print,
                boost::asio::placeholders::error, &t2, &count, &strand)));

    boost::thread thd(boost::bind(&boost::asio::io_service::run, &io));
    io.run();
    thd.join();

    std::cout << "Final count is " << count << std::endl;

    return 0;
}
#endif 
