#ifndef SERIAL_STREAMER_H
#define SERIAL_STREAMER_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct LogEntry {
    String device_id;
    unsigned long timestamp;
    float frequency;
    String note;
    int octave;
    float cents;
    float confidence;
    bool stable;
};

class SerialStreamer {
private:
    static const int STREAM_BUFFER_SIZE = 50;
    LogEntry streamBuffer[STREAM_BUFFER_SIZE];
    int bufferIndex = 0;
    unsigned long lastStream = 0;
    static const unsigned long STREAM_RATE = 100; // 10Hz streaming
    static const String DEVICE_ID;
    
public:
    SerialStreamer();
    
    // Add pitch data to stream buffer
    void addPitchData(float frequency, const String& note, int octave, 
                      float cents, float confidence, bool stable);
    
    // Add health metrics to stream buffer
    void addHealthMetrics(unsigned long uptime, int memoryUsage, 
                         float cpuUsage, int errorCount);
    
    // Stream buffered data if needed
    void streamIfNeeded();
    
    // Format LogEntry as JSON string
    String formatJSON(const LogEntry& entry);
    
    // Get current buffer size
    int getBufferSize() const { return bufferIndex; }
    
    // Clear buffer
    void clearBuffer() { bufferIndex = 0; }
};

#endif // SERIAL_STREAMER_H
