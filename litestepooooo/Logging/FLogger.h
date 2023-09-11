#pragma once
#include <stdio.h>
#include <ctime>

#define LOG_FILE_NAME "Litebox_log.txt"


class FLogger{
    static FILE* logFile;
public:

    FLogger();
    ~FLogger();


    template<typename... Args>
    static void log(const char* message_str, const char* message, Args... args)
    {
        if (logFile != 0) {
            std::time_t current_time = std::time(0);
            std::tm timestamp;
            errno_t  err = localtime_s(&timestamp, &current_time);

            char buffer[80];
            strftime(buffer, 80, "%T  %d-%m-%Y ", &timestamp);
            fprintf(logFile, "%s    ", buffer);
            fprintf(logFile, message_str);
            fprintf(logFile, message, args...);
            fprintf(logFile, "\n");
            fflush(logFile); //writes the unwritten data thats in buffer to the file.
        }
    }

     template<typename... Args>
     static void debug(const char* message, Args... args) {
         FLogger::log<Args...>("[DEBUG]", message, args...);
     }


   

    template<typename... Args>
    static void info(const char* message, Args... args ) {
        FLogger::log<Args...>("[INFO]", message, args...);
    }

    template<typename... Args>
    static void error(const char* message, Args... args ) {
        FLogger::log<Args ...>("[error]", message, args...);
    }




};
