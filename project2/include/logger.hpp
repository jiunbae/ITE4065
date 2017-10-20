#ifndef LOGGER
#define LOGGER

#include <string>
#include <fstream>
#include <ostream>
#include <chrono>

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
