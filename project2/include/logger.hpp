#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <ostream>

namespace transaction {
    class Logger {
    public:
        Logger(const std::string&& filename, const std::string& format = "") {
            stream.open(name = filename + (format.size() ? "." + format : ""));
        }

        template <typename T>
        void write(const T& message) {
            stream << message;
            stream << '\n';
        }

        template <typename T, typename... Args>
        void write(const T& message, const Args&... messages) {
            stream << message;
            stream << ' ';
            write(messages...);
        }

        template <typename T>
        Logger& operator<<(T&& message) {
            stream << message;
            return *this;
        }

    private:
        std::string name;
        std::ofstream stream;
    };
}

#endif
