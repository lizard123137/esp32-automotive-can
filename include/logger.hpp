#pragma once

#include <Arduino.h>
#include <string>

enum logger_level {
    ERROR,
    INFO,
    DEBUG,
};

class Logger {
public:
    static const logger_level current_level = INFO;

    static void print(
        logger_level level,
        const std::string& module_name,
        const char* fmt,
        ...
    ) {
        if (level > current_level) return;

        const char* prefix = "UNKNOWN";
        switch(level) {
        case ERROR: prefix = "ERROR  "; break;
        case INFO: prefix = "INFO   "; break;
        case DEBUG: prefix = "DEBUG  "; break;
        }

        char buffer[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        Serial.printf("[%s] %s - %s\n", prefix, module_name.c_str(), buffer);
    }

private:
    Logger() = delete;
};