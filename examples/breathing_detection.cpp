/*
 * ESP32 Audio Pitch Engine - OroborusUnity Breathing Detection Example
 * 
 * This example demonstrates the breathing pattern detection capabilities
 * of the OroborusUnity integration using the I2S microphone.
 * 
 * Features:
 * - Real-time breathing pattern analysis
 * - Breathing rate calculation (BPM)
 * - Pattern classification (normal, deep, shallow, irregular)
 * - Visual feedback on OLED display
 * - WiFi streaming of breathing data
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "utils/Logger.h"
#include "utils/NoiseFilter.h"
#include "interfaces/WiFiStreamer.h"

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2S pins (same as pitch detection)
#define I2S_WS  25
#define I2S_SD  34
#define I2S_SCK 26
#define I2S_PORT I2S_NUM_0

// Audio buffer
#define BUFFER_LEN 1024
int32_t audioBuffer[BUFFER_LEN];

// Breathing detection parameters
#define BREATHING_SAMPLE_RATE 16000
#define BREATHING_MAX_FREQ 4.0      // Max breathing frequency (Hz)
#define BREATHING_MIN_FREQ 0.1      // Min breathing frequency (Hz)
#define BREATHING_THRESHOLD 0.3     // Detection sensitivity

// Breathing patterns enum
enum BreathingPattern {
    PATTERN_NORMAL,
    PATTERN_DEEP,
    PATTERN_SHALLOW,
    PATTERN_IRREGULAR,
    PATTERN_APNEA
};

// Breathing detection class
class BreathingDetector {
private:
    float filteredSignal[BUFFER_LEN];
    float breathingRate;
    float breathingDepth;
    BreathingPattern currentPattern;
    float confidence;
    bool isCalibrated;
    float noiseFloor;
    
    // Peak detection
    static const int PEAK_HISTORY_SIZE = 10;
    float peakTimes[PEAK_HISTORY_SIZE];
    int peakIndex;
    unsigned long lastPeakTime;
    
    // Pattern analysis
    float inhaleDuration;
    float exhaleDuration;
    float regularityScore;
    
public:
    BreathingDetector() {
        breathingRate = 0;
        breathingDepth = 0;
        currentPattern = PATTERN_NORMAL;
        confidence = 0;
        isCalibrated = false;
        noiseFloor = 0;
        peakIndex = 0;
        lastPeakTime = 0;
        inhaleDuration = 0;
        exhaleDuration = 0;
        regularityScore = 0;
    }
    
    bool begin() {
        Serial.println("Initializing Breathing Detector...");
        
        // Initialize noise floor
        noiseFloor = 0.01;
        
        // Calibrate with silence
        calibrateSilence();
        
        Serial.println("Breathing Detector ready!");
        return true;
    }
    
    bool detectBreathingPattern(const int32_t* buffer, int size) {
        // Filter audio signal for breathing frequencies
        filterBreathingSignal(buffer, size, filteredSignal);
        
        // Calculate signal envelope
        float envelope = calculateEnvelope(filteredSignal, size);
        
        // Check if there's breathing activity
        if (envelope < (noiseFloor + BREATHING_THRESHOLD)) {
            confidence = 0;
            return false;
        }
        
        // Detect peaks (breathing cycles)
        detectPeaks(filteredSignal, size);
        
        // Calculate breathing rate
        if (peakIndex >= 2) {
            calculateBreathingRate();
            analyzePattern();
            confidence = min(0.95, regularityScore);
            return true;
        }
        
        return false;
    }
    
    float getBreathingRate() const { return breathingRate; }
    float getBreathingDepth() const { return breathingDepth; }
    BreathingPattern getPattern() const { return currentPattern; }
    float getConfidence() const { return confidence; }
    
    String getPatternString() const {
        switch (currentPattern) {
            case PATTERN_NORMAL: return "NORMAL";
            case PATTERN_DEEP: return "DEEP";
            case PATTERN_SHALLOW: return "SHALLOW";
            case PATTERN_IRREGULAR: return "IRREGULAR";
            case PATTERN_APNEA: return "APNEA";
            default: return "UNKNOWN";
        }
    }
    
    void setSensitivity(float sensitivity) {
        // Adjust threshold based on sensitivity (0.1-1.0)
        // This would modify BREATHING_THRESHOLD
    }
    
private:
    void filterBreathingSignal(const int32_t* input, int size, float* output) {
        // Apply bandpass filter for breathing frequencies (0.1-4 Hz)
        float alpha = 0.99; // High-pass filter coefficient
        float beta = 0.98;  // Low-pass filter coefficient
        
        static float highPass = 0;
        static float lowPass = 0;
        
        for (int i = 0; i < size; i++) {
            // Convert to float and normalize
            float sample = (float)input[i] / 2147483648.0f;
            
            // High-pass filter (remove DC offset)
            highPass = alpha * (highPass + sample - (i > 0 ? (float)input[i-1] / 2147483648.0f : 0));
            
            // Low-pass filter (smooth the signal)
            lowPass = beta * lowPass + (1 - beta) * highPass;
            
            output[i] = lowPass;
        }
    }
    
    float calculateEnvelope(const float* signal, int size) {
        float sum = 0;
        for (int i = 0; i < size; i++) {
            sum += abs(signal[i]);
        }
        return sum / size;
    }
    
    void detectPeaks(const float* signal, int size) {
        unsigned long currentTime = millis();
        
        for (int i = 1; i < size - 1; i++) {
            // Simple peak detection
            if (signal[i] > signal[i-1] && signal[i] > signal[i+1] && signal[i] > 0.1) {
                if (currentTime - lastPeakTime > 1000) { // Minimum 1 second between peaks
                    peakTimes[peakIndex % PEAK_HISTORY_SIZE] = currentTime;
                    lastPeakTime = currentTime;
                    peakIndex++;
                    
                    // Calculate depth
                    breathingDepth = min(1.0, signal[i] * 2);
                }
            }
        }
    }
    
    void calculateBreathingRate() {
        if (peakIndex < 2) return;
        
        // Calculate average interval between peaks
        float totalInterval = 0;
        int count = min(peakIndex, PEAK_HISTORY_SIZE) - 1;
        
        for (int i = 0; i < count; i++) {
            int idx1 = (peakIndex - count + i) % PEAK_HISTORY_SIZE;
            int idx2 = (peakIndex - count + i + 1) % PEAK_HISTORY_SIZE;
            
            totalInterval += peakTimes[idx2] - peakTimes[idx1];
        }
        
        if (count > 0) {
            float avgInterval = totalInterval / count / 1000.0; // Convert to seconds
            breathingRate = 60.0 / avgInterval; // Convert to BPM
        }
    }
    
    void analyzePattern() {
        // Classify breathing pattern based on rate and depth
        if (breathingRate < 6 || breathingRate > 30) {
            currentPattern = PATTERN_IRREGULAR;
        } else if (breathingRate < 12 && breathingDepth > 0.7) {
            currentPattern = PATTERN_DEEP;
        } else if (breathingRate > 20 && breathingDepth < 0.3) {
            currentPattern = PATTERN_SHALLOW;
        } else if (breathingRate < 3) {
            currentPattern = PATTERN_APNEA;
        } else {
            currentPattern = PATTERN_NORMAL;
        }
        
        // Calculate regularity score
        calculateRegularity();
    }
    
    void calculateRegularity() {
        if (peakIndex < 3) {
            regularityScore = 0.5;
            return;
        }
        
        // Calculate variance in peak intervals
        float intervals[PEAK_HISTORY_SIZE - 1];
        float sum = 0;
        int count = min(peakIndex - 1, PEAK_HISTORY_SIZE - 1);
        
        for (int i = 0; i < count; i++) {
            int idx1 = (peakIndex - count + i) % PEAK_HISTORY_SIZE;
            int idx2 = (peakIndex - count + i + 1) % PEAK_HISTORY_SIZE;
            intervals[i] = (peakTimes[idx2] - peakTimes[idx1]) / 1000.0;
            sum += intervals[i];
        }
        
        if (count > 0) {
            float mean = sum / count;
            float variance = 0;
            
            for (int i = 0; i < count; i++) {
                variance += pow(intervals[i] - mean, 2);
            }
            
            variance /= count;
            
            // Convert variance to regularity score (0-1)
            regularityScore = max(0, 1 - variance / 4.0); // 4 seconds variance = 0 score
        }
    }
    
    void calibrateSilence() {
        Serial.println("Calibrating silence level...");
        
        // Collect samples for noise floor estimation
        float noiseSum = 0;
        int samples = 100;
        
        for (int i = 0; i < samples; i++) {
            size_t bytes_read = 0;
            i2s_read(I2S_PORT, audioBuffer, sizeof(audioBuffer), &bytes_read, portMAX_DELAY);
            
            if (bytes_read > 0) {
                int samples_read = bytes_read / sizeof(int32_t);
                for (int j = 0; j < samples_read; j++) {
                    float sample = (float)audioBuffer[j] / 2147483648.0f;
                    noiseSum += abs(sample);
                }
            }
        }
        
        noiseFloor = noiseSum / (samples * BUFFER_LEN);
        Serial.printf("Noise floor calibrated: %.4f\n", noiseFloor);
        
        isCalibrated = true;
    }
};

// Global variables
BreathingDetector breathingDetector;
WiFiStreamer streamer;
bool wifiConnected = false;

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Audio Pitch Engine - OroborusUnity Breathing Detection");
    
    // Initialize display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Display initialization failed");
        return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Breathing Detector");
    display.println("Initializing...");
    display.display();
    
    delay(1000);
    
    // Initialize I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = BREATHING_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 100,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };
    
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_set_clk(I2S_PORT, BREATHING_SAMPLE_RATE, 32, I2S_CHANNEL_MONO);
    
    // Initialize breathing detector
    if (!breathingDetector.begin()) {
        Serial.println("Failed to initialize breathing detector");
        return;
    }
    
    // Initialize WiFi (optional)
    // setupWiFi();
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Ready!");
    display.println("Breathe normally");
    display.println("near microphone");
    display.display();
    
    Serial.println("System ready. Place microphone 10-20cm from mouth/nose.");
}

void loop() {
    // Read audio data
    size_t bytes_read = 0;
    i2s_read(I2S_PORT, audioBuffer, sizeof(audioBuffer), &bytes_read, portMAX_DELAY);
    
    if (bytes_read > 0) {
        int samples_read = bytes_read / sizeof(int32_t);
        
        // Detect breathing pattern
        bool detected = breathingDetector.detectBreathingPattern(audioBuffer, samples_read);
        
        if (detected) {
            // Get breathing metrics
            float rate = breathingDetector.getBreathingRate();
            float depth = breathingDetector.getBreathingDepth();
            BreathingPattern pattern = breathingDetector.getPattern();
            float confidence = breathingDetector.getConfidence();
            
            // Update display
            updateBreathingDisplay(rate, depth, pattern, confidence);
            
            // Serial output
            Serial.printf("Breathing: %.1f BPM, Pattern: %s, Depth: %.2f, Confidence: %.2f\n", 
                         rate, breathingDetector.getPatternString().c_str(), depth, confidence);
            
            // Stream data if WiFi is connected
            if (wifiConnected) {
                streamBreathingData(rate, depth, pattern, confidence);
            }
        } else {
            // Show no detection screen
            if (millis() % 5000 < 100) { // Update every 5 seconds
                updateNoDetectionDisplay();
            }
        }
    }
    
    delay(100); // 10Hz update rate
}

void updateBreathingDisplay(float rate, float depth, BreathingPattern pattern, float confidence) {
    display.clearDisplay();
    
    // Title
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("BREATHING DETECTOR");
    
    // Rate
    display.setCursor(0, 12);
    display.printf("Rate: %.1f BPM", rate);
    
    // Pattern
    display.setCursor(0, 22);
    display.printf("Pattern: %s", breathingDetector.getPatternString().c_str());
    
    // Depth bar
    display.setCursor(0, 32);
    display.println("Depth:");
    int barLength = depth * 50;
    display.drawRect(50, 32, 50, 8, SSD1306_WHITE);
    display.fillRect(50, 32, barLength, 8, SSD1306_WHITE);
    
    // Confidence
    display.setCursor(0, 44);
    display.printf("Confidence: %.0f%%", confidence * 100);
    
    // Status indicator
    display.setCursor(0, 54);
    display.println("● DETECTING");
    
    display.display();
}

void updateNoDetectionDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("No breathing detected");
    display.println("Check microphone");
    display.println("placement");
    display.display();
}

void streamBreathingData(float rate, float depth, BreathingPattern pattern, float confidence) {
    // Create JSON data
    String jsonData = "{";
    jsonData += "\"type\":\"breathing\",";
    jsonData += "\"timestamp\":" + String(millis()) + ",";
    jsonData += "\"rate\":" + String(rate, 1) + ",";
    jsonData += "\"depth\":" + String(depth, 2) + ",";
    jsonData += "\"pattern\":\"" + breathingDetector.getPatternString() + "\",";
    jsonData += "\"confidence\":" + String(confidence, 2);
    jsonData += "}";
    
    // Send via WiFi (would need WiFiStreamer implementation)
    // streamer.sendData(jsonData);
    
    // Also output to serial for debugging
    Serial.println("Streaming: " + jsonData);
}

void setupWiFi() {
    // WiFi setup code would go here
    // const char* ssid = "YOUR_WIFI_SSID";
    // const char* password = "YOUR_WIFI_PASSWORD";
    
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    
    // Serial.println("WiFi connected");
    // wifiConnected = true;
}
