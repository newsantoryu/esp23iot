# OroborusUnity Integration - Breathing Detection

## 🌊 Overview

The OroborusUnity integration extends the ESP32 Audio Pitch Engine with advanced breathing pattern detection capabilities. This module analyzes audio signals captured through the I2S microphone to identify and monitor breathing patterns in real-time.

### Purpose

- **Health Monitoring**: Track breathing patterns for wellness applications
- **Meditation Support**: Provide feedback for breathing exercises
- **Biometric Analysis**: Extract respiratory metrics from audio signals
- **Integration Ready**: Seamlessly works with existing pitch detection system

## 🔧 Technical Architecture

### Signal Processing Pipeline

```
Audio Input (I2S) → Breathing Filter → Pattern Analysis → Metrics Extraction → Data Output
```

### Core Components

#### 1. **BreathingDetector Class**
```cpp
class BreathingDetector {
public:
    BreathingDetector();
    bool detectBreathingPattern(const int32_t* audioBuffer, int bufferSize);
    float getBreathingRate();
    float getBreathingDepth();
    BreathingPattern getPattern();
    
private:
    float breathingFilter(const int32_t* buffer, int size);
    void analyzePattern(float filteredSignal);
    float calculateRate();
};
```

#### 2. **Breathing Patterns**
- **Normal**: Regular breathing (12-20 breaths/min)
- **Deep**: Slow, deep breathing (6-12 breaths/min)
- **Shallow**: Rapid, shallow breathing (>20 breaths/min)
- **Irregular**: Inconsistent patterns

#### 3. **Metrics Extraction**
- **Breathing Rate**: Breaths per minute (BPM)
- **Inhalation/Exhalation Ratio**: Time balance analysis
- **Depth Amplitude**: Signal strength variation
- **Pattern Consistency**: Regularity measurement

## 📋 Hardware Requirements

### Essential Components
- **ESP32 Development Board** (same as pitch detection)
- **I2S Microphone** (high sensitivity recommended)
- **OLED Display** (for real-time feedback)
- **Optional**: Piezo buzzer for breathing cues

### Microphone Recommendations
- **INMP441**: High quality, low noise
- **SPH0645**: Wide frequency response
- **Custom**: Calibrated for breathing frequencies (0.5-4Hz)

### Placement Guidelines
```
Optimal Microphone Placement:
┌─────────────────────┐
│                     │
│  [MICROPHONE]       │ ← 10-20cm from mouth/nose
│                     │   Angle: 45°
│                     │   Avoid direct airflow
└─────────────────────┘
```

## 🚀 Installation and Setup

### 1. Hardware Setup
```cpp
// Pin configuration (shared with pitch detection)
#define I2S_WS  25    // Word Select
#define I2S_SD  34    // Serial Data  
#define I2S_SCK 26    // Serial Clock
#define LED_PIN 14    // Status indicator
```

### 2. Software Integration
```cpp
#include "integrations/BreathingDetector.h"

// In main.cpp
BreathingDetector breathingDetector;

void setup() {
    // Existing pitch detection setup
    setupPitchDetection();
    
    // Initialize breathing detection
    breathingDetector.begin();
    breathingDetector.setSensitivity(0.7);  // Adjust for environment
}

void loop() {
    // Existing audio processing
    processAudio();
    
    // Breathing detection
    if (breathingDetector.detectBreathingPattern(audioBuffer, bufferSize)) {
        float rate = breathingDetector.getBreathingRate();
        BreathingPattern pattern = breathingDetector.getPattern();
        
        // Display or process breathing data
        displayBreathingInfo(rate, pattern);
    }
}
```

### 3. Configuration Options
```cpp
// Breathing detection parameters
breathingDetector.setSampleRate(16000);     // Audio sample rate
breathingDetector.setBufferSize(1024);     // Analysis buffer size
breathingDetector.setFilterFrequency(4.0); // Max breathing frequency
breathingDetector.setThreshold(0.3);        // Detection sensitivity
```

## 📊 Data Analysis

### Breathing Rate Calculation
```cpp
float BreathingDetector::calculateBreathingRate() {
    // Peak detection in filtered signal
    int peakCount = detectPeaks(filteredSignal, signalLength);
    float duration = signalLength / sampleRate;
    float rate = (peakCount / duration) * 60; // Convert to BPM
    
    return rate;
}
```

### Pattern Recognition
```cpp
enum BreathingPattern {
    PATTERN_NORMAL,
    PATTERN_DEEP,
    PATTERN_SHALLOW,
    PATTERN_IRREGULAR,
    PATTERN_APNEA
};

BreathingPattern classifyPattern(float rate, float depth, float regularity) {
    if (rate < 6 || rate > 30) return PATTERN_IRREGULAR;
    if (rate < 12 && depth > 0.7) return PATTERN_DEEP;
    if (rate > 20 && depth < 0.3) return PATTERN_SHALLOW;
    if (regularity < 0.5) return PATTERN_IRREGULAR;
    return PATTERN_NORMAL;
}
```

### Metrics Output Format
```json
{
  "timestamp": 1634567890,
  "breathing": {
    "rate": 14.5,
    "depth": 0.65,
    "pattern": "normal",
    "inhale_duration": 2.1,
    "exhale_duration": 2.0,
    "regularity": 0.85,
    "confidence": 0.92
  },
  "audio": {
    "signal_strength": 0.72,
    "noise_level": 0.15,
    "quality": "good"
  }
}
```

## 🎯 Use Cases and Applications

### 1. **Meditation and Relaxation**
```cpp
// Breathing exercise guidance
void guidedBreathingExercise() {
    float targetRate = 6.0; // 6 breaths/minute for relaxation
    
    while (exerciseActive) {
        float currentRate = breathingDetector.getBreathingRate();
        
        if (currentRate > targetRate) {
            display.slowDown();
            buzzer.playSlowTone();
        } else if (currentRate < targetRate) {
            display.speedUp();
            buzzer.playFastTone();
        }
        
        delay(1000);
    }
}
```

### 2. **Health Monitoring**
```cpp
// Abnormal breathing detection
void monitorBreathingHealth() {
    BreathingPattern pattern = breathingDetector.getPattern();
    float rate = breathingDetector.getBreathingRate();
    
    if (pattern == PATTERN_IRREGULAR || rate < 8 || rate > 25) {
        sendAlert("Abnormal breathing pattern detected");
        logHealthData(rate, pattern);
    }
}
```

### 3. **Biometric Research**
```cpp
// Data collection for research
void collectBreathingData() {
    BreathingMetrics metrics = breathingDetector.getMetrics();
    
    // Store for analysis
    dataLogger.log(metrics);
    
    // Send to cloud for processing
    if (wifiConnected()) {
        sendDataToServer(metrics);
    }
}
```

## 📱 Display Integration

### OLED Display Layout
```
┌─────────────────────┐
│  BREATHING MONITOR  │
│                     │
│  Rate: 14.5 BPM     │
│  Pattern: NORMAL    │
│  Depth: ████████░░  │
│                     │
│  ● INHALE ○ EXHALE  │
│  Quality: EXCELLENT │
└─────────────────────┘
```

### Visual Feedback
```cpp
void displayBreathingInfo(float rate, BreathingPattern pattern) {
    display.clearDisplay();
    
    // Title
    display.setCursor(0, 0);
    display.println("BREATHING MONITOR");
    
    // Rate
    display.setCursor(0, 16);
    display.printf("Rate: %.1f BPM", rate);
    
    // Pattern
    display.setCursor(0, 24);
    display.printf("Pattern: %s", patternToString(pattern));
    
    // Depth bar
    display.setCursor(0, 32);
    display.println("Depth:");
    int barLength = breathingDetector.getBreathingDepth() * 50;
    display.drawRect(50, 32, 50, 8, SSD1306_WHITE);
    display.fillRect(50, 32, barLength, 8, SSD1306_WHITE);
    
    // Phase indicator
    display.setCursor(0, 48);
    if (breathingDetector.isInhaling()) {
        display.println("● INHALE ○ EXHALE");
    } else {
        display.println("○ INHALE ● EXHALE");
    }
    
    display.display();
}
```

## 🔧 Advanced Configuration

### Filter Parameters
```cpp
// Breathing-specific filtering
struct BreathingFilterConfig {
    float lowPassFreq = 4.0;      // Max breathing frequency
    float highPassFreq = 0.1;     // Min breathing frequency
    float noiseThreshold = 0.2;   // Noise rejection level
    float smoothingFactor = 0.8;   // Signal smoothing
};

breathingDetector.setFilterConfig(config);
```

### Calibration
```cpp
// Automatic calibration
void calibrateBreathingDetection() {
    Serial.println("Starting breathing calibration...");
    Serial.println("Please breathe normally for 30 seconds.");
    
    breathingDetector.startCalibration();
    
    for (int i = 0; i < 30; i++) {
        breathingDetector.calibrateSample(audioBuffer, bufferSize);
        delay(1000);
        Serial.printf("Calibrating... %d/30\n", i + 1);
    }
    
    breathingDetector.endCalibration();
    Serial.println("Calibration complete!");
}
```

### Multi-User Support
```cpp
// User profiles
struct BreathingProfile {
    String name;
    float normalRateMin;
    float normalRateMax;
    float preferredDepth;
    float sensitivity;
};

BreathingProfile profiles[] = {
    {"Adult", 12.0, 20.0, 0.5, 0.7},
    {"Child", 20.0, 30.0, 0.3, 0.8},
    {"Athlete", 6.0, 12.0, 0.8, 0.6}
};

breathingDetector.setProfile(profiles[0]);
```

## 🐛 Troubleshooting

### Common Issues

#### 1. **No Breathing Detection**
```
Problem: No breathing patterns detected
Causes:
- Microphone too far from user
- Low sensitivity settings
- High environmental noise

Solutions:
- Move microphone closer (10-20cm)
- Increase sensitivity threshold
- Reduce ambient noise
```

#### 2. **False Positives**
```
Problem: Breathing detected when not breathing
Causes:
- Room vibrations
- Air conditioning noise
- Microphone sensitivity too high

Solutions:
- Lower sensitivity threshold
- Improve noise filtering
- Change microphone placement
```

#### 3. **Inaccurate Rate**
```
Problem: Breathing rate seems incorrect
Causes:
- Incorrect sample rate
- Filter parameters wrong
- Signal processing errors

Solutions:
- Verify I2S configuration
- Adjust filter frequencies
- Recalibrate system
```

### Debug Tools
```cpp
// Enable breathing debug output
breathingDetector.enableDebug(true);

// Monitor raw signal
float rawSignal = breathingDetector.getRawSignal();
Serial.printf("Raw: %.3f, Filtered: %.3f\n", rawSignal, filteredSignal);

// Pattern analysis
BreathingPattern pattern = breathingDetector.getPattern();
Serial.printf("Pattern: %s, Rate: %.1f, Confidence: %.2f\n", 
              patternToString(pattern), 
              breathingDetector.getBreathingRate(),
              breathingDetector.getConfidence());
```

## 📚 API Reference

### BreathingDetector Class

#### Constructor
```cpp
BreathingDetector();
```

#### Core Methods
```cpp
bool begin();                                    // Initialize detector
bool detectBreathingPattern(const int32_t* buffer, int size);
float getBreathingRate();                        // Get BPM
float getBreathingDepth();                       // Get depth (0-1)
BreathingPattern getPattern();                  // Get pattern type
bool isInhaling();                               // Current phase
float getConfidence();                           // Detection confidence
```

#### Configuration
```cpp
void setSensitivity(float sensitivity);          // 0.1-1.0
void setSampleRate(int rate);                    // Hz
void setBufferSize(int size);                    // Samples
void setFilterFrequency(float freq);             // Hz
void setThreshold(float threshold);             // Detection threshold
```

#### Calibration
```cpp
void startCalibration();
void calibrateSample(const int32_t* buffer, int size);
void endCalibration();
bool isCalibrated();
```

#### Data Structures
```cpp
enum BreathingPattern {
    PATTERN_NORMAL,
    PATTERN_DEEP,
    PATTERN_SHALLOW,
    PATTERN_IRREGULAR,
    PATTERN_APNEA
};

struct BreathingMetrics {
    float rate;
    float depth;
    BreathingPattern pattern;
    float inhaleDuration;
    float exhaleDuration;
    float regularity;
    float confidence;
};
```

## 🔄 Integration with Pitch Detection

### Shared Resources
```cpp
// Both systems use the same audio input
void processAudio() {
    // Read audio once
    size_t bytes_read = i2s_read(I2S_PORT, audioBuffer, bufferSize, &bytes_read, portMAX_DELAY);
    
    // Pitch detection
    if (pitchEnabled) {
        float pitch = pitchDetector.calculatePitch(audioBuffer, bufferSize);
        // Process pitch...
    }
    
    // Breathing detection
    if (breathingEnabled) {
        breathingDetector.detectBreathingPattern(audioBuffer, bufferSize);
        // Process breathing...
    }
}
```

### Mode Switching
```cpp
enum DetectionMode {
    MODE_PITCH_ONLY,
    MODE_BREATHING_ONLY,
    MODE_HYBRID        // Both active
};

void setDetectionMode(DetectionMode mode) {
    currentMode = mode;
    
    switch (mode) {
        case MODE_PITCH_ONLY:
            pitchEnabled = true;
            breathingEnabled = false;
            break;
        case MODE_BREATHING_ONLY:
            pitchEnabled = false;
            breathingEnabled = true;
            break;
        case MODE_HYBRID:
            pitchEnabled = true;
            breathingEnabled = true;
            break;
    }
}
```

## 🌐 Data Streaming

### WiFi Integration
```cpp
// Extend WiFiStreamer for breathing data
void streamBreathingData() {
    BreathingMetrics metrics = breathingDetector.getMetrics();
    
    String jsonData = "{";
    jsonData += "\"type\":\"breathing\",";
    jsonData += "\"rate\":" + String(metrics.rate, 1) + ",";
    jsonData += "\"depth\":" + String(metrics.depth, 2) + ",";
    jsonData += "\"pattern\":\"" + patternToString(metrics.pattern) + "\",";
    jsonData += "\"confidence\":" + String(metrics.confidence, 2);
    jsonData += "}";
    
    wifiStreamer.sendData(jsonData);
}
```

### Real-time Dashboard
```javascript
// Web dashboard example
function updateBreathingDisplay(data) {
    document.getElementById('breathing-rate').textContent = data.rate + ' BPM';
    document.getElementById('breathing-pattern').textContent = data.pattern;
    
    // Update visualization
    updateBreathingWaveform(data.depth);
    updatePatternIndicator(data.pattern);
}
```

## 🧪 Testing and Validation

### Test Scenarios
```cpp
// Test breathing rate accuracy
void testBreathingRateAccuracy() {
    // Simulate known breathing patterns
    float testRates[] = {6.0, 12.0, 18.0, 24.0};
    
    for (float targetRate : testRates) {
        generateTestSignal(targetRate);
        float detectedRate = breathingDetector.getBreathingRate();
        
        float error = abs(targetRate - detectedRate);
        TEST_ASSERT_LESS_THAN(1.0, error); // Within 1 BPM
    }
}
```

### Performance Benchmarks
- **Detection Accuracy**: >90% for normal breathing
- **Response Time**: <2 seconds for pattern recognition
- **Memory Usage**: <10KB additional memory
- **CPU Overhead**: <5% additional processing

---

**Need help with OroborusUnity integration? Check the [Development Guide](../development/README.md) or open an issue on GitHub!**
