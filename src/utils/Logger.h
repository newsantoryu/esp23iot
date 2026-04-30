#pragma once
#include <Arduino.h>

enum LogLevel {
    LOG_NONE = 0,
    LOG_ERROR = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3
};

class Logger {
public:
    static void setLevel(LogLevel level);
    static void error(const String& message);
    static void info(const String& message);
    static void debug(const String& message);
    static void audio(const String& message);
    static void display(const String& message);
    static void pitch(const String& message);
    
private:
    static LogLevel currentLevel;
    static unsigned long lastLogTime;
    static const unsigned long LOG_RATE_LIMIT = 200; // 200ms = 5 logs/segundo global
    static unsigned long logCount;
    static unsigned long lastSecondTime;
    
    static void log(LogLevel level, const String& prefix, const String& message);
    static bool shouldLog();
    static void updateLogStats();
};
