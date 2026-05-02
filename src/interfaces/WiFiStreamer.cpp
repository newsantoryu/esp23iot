#include "WiFiStreamer.h"

// WiFi credentials - configure these for your network
const String WIFI_SSID = "Victor - 2.4G-EXT";
const String WIFI_PASSWORD = "07110589";
const String BACKEND_IP = "localhost"; // Change to your backend IP
const int BACKEND_PORT = 8001;

WiFiStreamer::WiFiStreamer() : bufferIndex(0), lastSend(0), isConnected(false) {
    serverURL = "http://" + BACKEND_IP + ":" + String(BACKEND_PORT) + "/api/esp32-data";
}

void WiFiStreamer::begin(const String& ssid, const String& password, const String& serverIP) {
    // Update server URL if provided
    if (serverIP != "") {
        serverURL = "http://" + serverIP + ":" + String(BACKEND_PORT) + "/api/esp32-data";
    }
    
    Serial.println("🔗 WiFiStreamer: Starting WiFi connection...");
    
    // Configure WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false); // Keep WiFi active
    
    // Try to connect
    if (connectToWiFi()) {
        Serial.println("✅ WiFiStreamer: Connected to WiFi");
        Serial.print("📡 IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("🌐 Server URL: ");
        Serial.println(serverURL);
    } else {
        Serial.println("❌ WiFiStreamer: Failed to connect to WiFi");
    }
}

bool WiFiStreamer::connectToWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ Already connected to WiFi");
        return true;
    }
    
    Serial.println("🔗 Connecting to WiFi...");
    Serial.println("SSID: " + String(WIFI_SSID));
    Serial.println("Password length: " + String(String(WIFI_PASSWORD).length()));
    
    // Reset WiFi before connecting
    WiFi.disconnect(true, true);
    delay(1000);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for connection with detailed logging
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        
        // Log WiFi status every 5 attempts
        if (attempts % 5 == 0) {
            Serial.println();
            Serial.println("Attempt " + String(attempts + 1) + "/20 - Status: " + String(WiFi.status()));
        }
        attempts++;
    }
    
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi connected successfully!");
        Serial.println("IP Address: " + WiFi.localIP().toString());
        Serial.println("SSID: " + WiFi.SSID());
        Serial.println("RSSI: " + String(WiFi.RSSI()) + "dBm");
        isConnected = true;
        return true;
    } else {
        Serial.println("❌ Failed to connect to WiFi");
        Serial.println("Final status: " + String(WiFi.status()));
        Serial.println("Final attempts: " + String(attempts));
        isConnected = false;
        return false;
    }
}

void WiFiStreamer::loop() {
    // Check WiFi connection and try to reconnect if needed
    if (WiFi.status() != WL_CONNECTED) {
        if (isConnected) {
            Serial.println("🔌 WiFi disconnected");
            isConnected = false;
        }
        
        // Try to reconnect periodically
        if (millis() - lastConnectionAttempt > RECONNECT_INTERVAL) {
            connectToWiFi();
            lastConnectionAttempt = millis();
        }
        return;
    }
    
    if (!isConnected) {
        Serial.println("🔗 WiFi reconnected");
        isConnected = true;
    }
    
    // Send buffered data if needed
    sendIfNeeded();
}

void WiFiStreamer::addPitchData(float frequency, const String& note, int octave,
                                float cents, float confidence, bool stable) {
    if (bufferIndex < BUFFER_SIZE) {
        WiFiDataEntry entry = {
            .device_id = "esp32_audio_001",
            .timestamp = 1777743000000UL + ((unsigned long)millis() * 1000),  // Current Unix timestamp + millis*1000 (2026)
            .frequency = frequency,
            .note = note,
            .octave = octave,
            .cents = cents,
            .confidence = confidence,
            .stable = stable
        };

        buffer[bufferIndex] = entry;
        bufferIndex++;
    }
}

void WiFiStreamer::addHealthMetrics(unsigned long uptime, int memoryUsage,
                                     float cpuUsage, int errorCount) {
    if (bufferIndex < BUFFER_SIZE) {
        WiFiDataEntry entry = {
            .device_id = "esp32_audio_001",
            .timestamp = 1777743000000UL + ((unsigned long)millis() * 1000),  // Current Unix timestamp + millis*1000 (2026)
            .frequency = 0.0,  // Health data has no frequency
            .note = "HEALTH",
            .octave = 0,
            .cents = 0.0,
            .confidence = cpuUsage / 100.0,  // Use confidence field for CPU usage
            .stable = errorCount == 0
        };

        buffer[bufferIndex] = entry;
        bufferIndex++;
    }
}

void WiFiStreamer::sendIfNeeded() {
    unsigned long currentTime = millis();
    
    // Send data at configured rate
    if (currentTime - lastSend >= SEND_RATE && bufferIndex > 0) {
        // Send all buffered entries
        for (int i = 0; i < bufferIndex; i++) {
            String json = formatJSON(buffer[i]);
            
            if (sendToBackend(json)) {
                Serial.println("📤 WiFiStreamer: Data sent successfully");
                Serial.print("📊 Data: ");
                Serial.println(json);
            } else {
                Serial.println("❌ WiFiStreamer: Failed to send data");
            }
            
            delay(100); // Small delay between sends
        }

        // Clear buffer
        bufferIndex = 0;
        lastSend = currentTime;
    }
}

bool WiFiStreamer::sendToBackend(const String& jsonData) {
    if (!isConnected || WiFi.status() != WL_CONNECTED) {
        return false;
    }
    
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000); // 5 second timeout
    
    int httpResponseCode = http.POST(jsonData);
    
    bool success = false;
    if (httpResponseCode > 0) {
        if (httpResponseCode == 200) {
            success = true;
        } else {
            Serial.print("⚠️ HTTP Response code: ");
            Serial.println(httpResponseCode);
        }
    } else {
        Serial.print("❌ HTTP Error: ");
        Serial.println(httpResponseCode);
    }
    
    http.end();
    return success;
}

String WiFiStreamer::formatJSON(const WiFiDataEntry& entry) {
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
