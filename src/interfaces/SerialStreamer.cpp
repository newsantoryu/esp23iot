#include "SerialStreamer.h"

const String SerialStreamer::DEVICE_ID = "esp32_audio_001";

SerialStreamer::SerialStreamer() {
    bufferIndex = 0;
    lastStream = 0;
}

void SerialStreamer::addPitchData(float frequency, const String& note, int octave, 
                                  float cents, float confidence, bool stable) {
    if (bufferIndex < STREAM_BUFFER_SIZE) {
        LogEntry entry = {
            .device_id = DEVICE_ID,
            .timestamp = millis(),
            .frequency = frequency,
            .note = note,
            .octave = octave,
            .cents = cents,
            .confidence = confidence,
            .stable = stable
        };
        
        streamBuffer[bufferIndex] = entry;
        bufferIndex++;
    }
}

void SerialStreamer::addHealthMetrics(unsigned long uptime, int memoryUsage, 
                                       float cpuUsage, int errorCount) {
    if (bufferIndex < STREAM_BUFFER_SIZE) {
        LogEntry entry = {
            .device_id = DEVICE_ID,
            .timestamp = millis(),
            .frequency = 0.0,  // Health data has no frequency
            .note = "HEALTH",
            .octave = 0,
            .cents = 0.0,
            .confidence = cpuUsage / 100.0,  // Use confidence field for CPU usage
            .stable = errorCount == 0
        };
        
        streamBuffer[bufferIndex] = entry;
        bufferIndex++;
    }
}

void SerialStreamer::streamIfNeeded() {
    unsigned long currentTime = millis();
    
    // Stream at configured rate
    if (currentTime - lastStream >= STREAM_RATE && bufferIndex > 0) {
        // Stream all buffered entries
        for (int i = 0; i < bufferIndex; i++) {
            String json = formatJSON(streamBuffer[i]);
            Serial.println(json);
            
            // Small delay to prevent overwhelming the serial port
            delay(1);
        }
        
        // Clear buffer
        bufferIndex = 0;
        lastStream = currentTime;
    }
}

String SerialStreamer::formatJSON(const LogEntry& entry) {
    // Create JSON document
    StaticJsonDocument<256> doc;
    
    // Add basic fields
    doc["device_id"] = entry.device_id;
    doc["timestamp"] = entry.timestamp;
    
    if (entry.note == "HEALTH") {
        // Health metrics format
        doc["type"] = "health_metrics";
        doc["uptime"] = millis();  // Current uptime
        doc["memory_usage"] = ESP.getFreeHeap();
        doc["cpu_usage"] = entry.confidence * 100;  // Convert back to percentage
        doc["error_count"] = entry.stable ? 0 : 1;
        doc["health_score"] = entry.stable ? 100 : 75;
    } else {
        // Pitch data format
        doc["type"] = "pitch_data";
        doc["frequency"] = entry.frequency;
        doc["note"] = entry.note;
        doc["octave"] = entry.octave;
        doc["cents"] = entry.cents;
        doc["confidence"] = entry.confidence;
        doc["stable"] = entry.stable;
    }
    
    // Convert to string
    String jsonString;
    serializeJson(doc, jsonString);
    
    return jsonString;
}
