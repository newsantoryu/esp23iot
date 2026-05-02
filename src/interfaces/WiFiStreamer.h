#ifndef WIFI_STREAMER_H
#define WIFI_STREAMER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

struct WiFiDataEntry {
    String device_id;
    unsigned long timestamp;
    float frequency;
    String note;
    int octave;
    float cents;
    float confidence;
    bool stable;
};

class WiFiStreamer {
private:
    static const int BUFFER_SIZE = 10;
    WiFiDataEntry buffer[BUFFER_SIZE];
    int bufferIndex = 0;
    unsigned long lastSend = 0;
    static const unsigned long SEND_RATE = 1000; // 1Hz sending
    
    HTTPClient http;
    String serverURL;
    bool isConnected;
    unsigned long lastConnectionAttempt = 0;
    static const unsigned long RECONNECT_INTERVAL = 30000; // 30 seconds
    
    bool connectToWiFi();
    bool sendToBackend(const String& jsonData);
    String formatJSON(const WiFiDataEntry& entry);

public:
    WiFiStreamer();
    
    void begin(const String& ssid, const String& password, const String& serverIP);
    void loop();
    
    // Add pitch data to buffer
    void addPitchData(float frequency, const String& note, int octave,
                     float cents, float confidence, bool stable);
    
    // Add health metrics to buffer
    void addHealthMetrics(unsigned long uptime, int memoryUsage,
                         float cpuUsage, int errorCount);
    
    // Send buffered data if needed
    void sendIfNeeded();
    
    // Get current buffer size
    int getBufferSize() const { return bufferIndex; }
    
    // Clear buffer
    void clearBuffer() { bufferIndex = 0; }
    
    // Check connection status
    bool isWiFiConnected() const { return isConnected; }
};

#endif // WIFI_STREAMER_H
