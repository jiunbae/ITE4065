#ifndef LOGGER
#define LOGGER

#include <string>
#include <fstream>
#include <ostream>
#include <chrono>

#if defined(__GNUC__) && (__GNUC__ < 7)
//Imp: C++17 feature! but not on gcc < 7
//gcc5 and earlier provides an experimental C++ 17 standard from "experimental/"
	// A non-owning reference to a string
	// @see also: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3921.html
	#include <experimental/string_view>
	using string_view = std::experimental::string_view;
#elif (defined(_MSC_VER ) && (_MSC_VER  >= 1910)) || defined(__GNUC__) && (__GNUC__ >= 7)
// VS2017.3 supports a broader range of C++ 17 standards with `/std:c++latest` tag
	#include <string_view>
	using string_view = std::string_view;
#else
#endif

/*
	Timestamp class using std::chrono
	Supports up to nano time units.

	You can use with Logger or others.
*/
class Timestamp {
	using clock = std::chrono::system_clock;
	using timepoint = std::chrono::system_clock::time_point;
	using duration = std::chrono::milliseconds;

public:
	Timestamp()
		: generated(clock::now()), v(clock::to_time_t(generated)) {
	}

	long long value() {
		return v;
	}

	std::ostream& operator<<(std::ostream& os) {
		os << v;
		return os;
	}

private:
	timepoint generated;
	long long v;
};

/*
	Logger class for logging
	
	write function writes a string to the stream over time,
	It may not be output sequentially by other threads.
	So it supports safe_write function.
	safe_write function puts another stream buffer and writes it to the file when the output is done.

	Also, support ostream operator<< override.
*/
class Logger {
public:
    Logger(const std::string&& filename, const std::string& format = "")
		: stream(name = filename + (format.size() ? "." + format : ""), std::ofstream::out) {
	}

    ~Logger() {
        stream.close();
    }

    template <typename T>
    void write(const T& message) {
        stream << message << '\n';
    }

    template <typename T, typename... Args>
    void write(const T& message, const Args&... messages) {
        stream << message << ' ';
        write(messages...);
    }

    template <typename T>
    void safe_write(const T& message) {
        safe_stream << message << '\n';
        stream << safe_stream.rdbuf();
        safe_stream.clear();
    }

    template <typename T, typename... Args>
    void safe_write(const T& message, const Args&... messages) {
        safe_stream << message << ' ';
        safe_write(messages...);
    }

	Logger& operator<<(const string_view&& message) {
		stream << message;
		return *this;
	}

    template <typename T>
    Logger& operator<<(T&& message) {
        stream << message;
        return *this;
    }

private:
	std::stringstream safe_stream;
	std::string name;
    std::ofstream stream;
};

#endif
