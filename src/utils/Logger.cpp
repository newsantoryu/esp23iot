#include "Logger.h"

LogLevel Logger::currentLevel = LOG_INFO;
unsigned long Logger::lastLogTime = 0;

void Logger::setLevel(LogLevel level) {
    currentLevel = level;
}

bool Logger::shouldLog() {
    unsigned long now = millis();
    if (now - lastLogTime < LOG_RATE_LIMIT) {
        return false;
    }
    lastLogTime = now;
    return true;
}

void Logger::log(LogLevel level, const String& prefix, const String& message) {
    if (level > currentLevel || !shouldLog()) return;
    
    unsigned long timestamp = millis();
    Serial.print("[");
    Serial.print(timestamp);
    Serial.print("] ");
    Serial.print(prefix);
    Serial.print(": ");
    Serial.println(message);
}

void Logger::error(const String& message) {
    log(LOG_ERROR, "ERROR", message);
}

void Logger::info(const String& message) {
    log(LOG_INFO, "INFO", message);
}

void Logger::debug(const String& message) {
    log(LOG_DEBUG, "DEBUG", message);
}

void Logger::audio(const String& message) {
    log(LOG_DEBUG, "AUDIO", message);
}

void Logger::display(const String& message) {
    log(LOG_DEBUG, "DISPLAY", message);
}

void Logger::pitch(const String& message) {
    log(LOG_DEBUG, "PITCH", message);
}
