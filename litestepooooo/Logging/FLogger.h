#pragma once
#include <fstream>
#define LOG_FILE_NAME L"Litebox_log.txt"

class FLogger {

public:
    enum class LogPriority
    {
        Debug, Info, Error
    };

    FLogger();
    ~FLogger();


    template<typename... Args>
    void debug(const char* message, Args... args) {
        log<Args...>("[DEBUG]", message, args...);
    }

    template<typename... Args>
    void info(const char* message, Args... args) {
        log<Args...>("[INFO]", message, args...);
    }

    template<typename... Args>
    void error(const char* message, Args... args) {
        log<Args...>("[error]", message, args...);
    }

private:

    template<typename... Args>
    void log(const char* message_str, const char* message, Args... args)
    {
        if (file != 0) {
            std::time_t current_time = std::time(0);
            std::tm timestamp;
            errno_t  err = localtime_s(&timestamp, &current_time);

            char buffer[80];
            std::strftime(buffer, 80, "%T  %d-%m-%Y ", &timestamp);
            std::fprintf(file, "%s    ", buffer);
            std::fprintf(file, message_str);
            std::fprintf(file, message, args...);
            std::fprintf(file, "\n");

        }
    }

    FILE* file;

};