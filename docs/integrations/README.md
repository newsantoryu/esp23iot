# Integrations - ESP32 Audio Pitch Engine

This directory contains documentation for various integrations and extensions of the ESP32 Audio Pitch Engine.

## 📁 Available Integrations

### 🌊 [OroborusUnity](./oroborusunity.md)
**Breathing Pattern Detection System**

Advanced breathing analysis using the I2S microphone to:
- Monitor breathing patterns in real-time
- Calculate breathing rate (BPM)
- Detect different breathing patterns (normal, deep, shallow, irregular)
- Provide biometric feedback for health and wellness applications
- Integrate seamlessly with existing pitch detection functionality

**Key Features:**
- Real-time breathing pattern recognition
- Configurable sensitivity and filtering
- Multiple breathing pattern classification
- Health monitoring capabilities
- Meditation and relaxation support
- WiFi streaming for remote monitoring

## 🔧 Integration Architecture

All integrations follow a consistent architecture:

```
Audio Input (I2S) → Signal Processing → Feature Extraction → Application Logic → Data Output
```

### Shared Components
- **Audio Buffer**: Common audio input from I2S microphone
- **Signal Processing**: Shared filtering and analysis
- **Display Integration**: Unified OLED display management
- **WiFi Streaming**: Common data transmission framework
- **Configuration**: Centralized parameter management

## 🚀 Getting Started with Integrations

### Prerequisites
- ESP32 Audio Pitch Engine base system
- I2S microphone setup
- OLED display (recommended)
- WiFi connection (for streaming features)

### Basic Integration Setup
```cpp
#include "integrations/BreathingDetector.h"

// Initialize integration
BreathingDetector breathingDetector;

void setup() {
    // Base system setup
    setupAudioProcessing();
    setupDisplay();
    
    // Integration setup
    breathingDetector.begin();
    breathingDetector.setSensitivity(0.7);
}

void loop() {
    // Process audio (shared)
    processAudioBuffer();
    
    // Integration-specific processing
    breathingDetector.detectBreathingPattern(audioBuffer, bufferSize);
    
    // Update display with combined data
    updateDisplay();
}
```

### Mode Selection
```cpp
enum DetectionMode {
    MODE_PITCH_ONLY,        // Musical pitch detection
    MODE_BREATHING_ONLY,    // Breathing pattern analysis
    MODE_HYBRID             // Both systems active
};

void setDetectionMode(DetectionMode mode) {
    currentMode = mode;
    updateSystemConfiguration();
}
```

## 📊 Data Formats

### Unified Data Structure
```json
{
  "timestamp": 1634567890,
  "mode": "hybrid",
  "audio": {
    "signal_strength": 0.72,
    "noise_level": 0.15,
    "quality": "good"
  },
  "pitch": {
    "frequency": 440.2,
    "note": "A",
    "octave": 4,
    "cents": 2.1,
    "confidence": 0.95
  },
  "breathing": {
    "rate": 14.5,
    "depth": 0.65,
    "pattern": "normal",
    "confidence": 0.92
  }
}
```

## 🎯 Use Cases

### Musical Applications
- **Instrument Tuning**: Traditional pitch detection
- **Vocal Training**: Pitch accuracy and breathing control
- **Music Therapy**: Combined audio and biometric feedback

### Health and Wellness
- **Meditation Guidance**: Breathing pattern coaching
- **Stress Monitoring**: Biometric stress indicators
- **Sleep Analysis**: Breathing patterns during rest

### Research and Development
- **Biometric Research**: Respiratory pattern studies
- **Audio Analysis**: Advanced signal processing
- **IoT Applications**: Remote health monitoring

## 🔧 Configuration

### Integration Settings
```cpp
// Global integration configuration
struct IntegrationConfig {
    bool pitchEnabled = true;
    bool breathingEnabled = false;
    DetectionMode mode = MODE_PITCH_ONLY;
    int updateRate = 10;  // Hz
    bool streamingEnabled = false;
};

IntegrationConfig config;
```

### Performance Tuning
```cpp
// Optimize for different use cases
void optimizeForMeditation() {
    config.pitchEnabled = false;
    config.breathingEnabled = true;
    config.updateRate = 5;  // Slower updates for relaxation
    breathingDetector.setSensitivity(0.8);
}

void optimizeForMusic() {
    config.pitchEnabled = true;
    config.breathingEnabled = false;
    config.updateRate = 20;  // Faster updates for real-time tuning
    pitchDetector.setPrecision(HIGH);
}
```

## 📱 Display Integration

### Multi-Mode Display
```
┌─────────────────────┐
│  ESP32 AUDIO ENGINE │
│                     │
│  MODE: HYBRID       │
│                     │
│  Pitch: A4 440Hz    │
│  Breath: 14.5 BPM   │
│                     │
│  ● ACTIVE ○ STREAM  │
└─────────────────────┘
```

### Mode Switching
```cpp
void updateDisplay() {
    display.clearDisplay();
    
    switch (currentMode) {
        case MODE_PITCH_ONLY:
            displayPitchInfo();
            break;
        case MODE_BREATHING_ONLY:
            displayBreathingInfo();
            break;
        case MODE_HYBRID:
            displayHybridInfo();
            break;
    }
    
    display.display();
}
```

## 🌐 Streaming and Connectivity

### Unified WiFi Streaming
```cpp
void streamIntegrationData() {
    String jsonData = buildUnifiedJSON();
    wifiStreamer.sendData(jsonData);
}

String buildUnifiedJSON() {
    String json = "{";
    json += "\"mode\":\"" + modeToString(currentMode) + "\",";
    
    if (config.pitchEnabled) {
        json += "\"pitch\":" + getPitchJSON() + ",";
    }
    
    if (config.breathingEnabled) {
        json += "\"breathing\":" + getBreathingJSON() + ",";
    }
    
    json += "\"timestamp\":" + String(millis()) + "}";
    return json;
}
```

### Remote Dashboard
- Real-time data visualization
- Historical trend analysis
- Alert system for abnormal patterns
- Multi-device support

## 🧪 Testing Integration

### Test Framework
```cpp
// Integration testing
void testHybridMode() {
    // Test with combined audio signals
    generateTestSignal(440.0, 14.5);  // A4 note + breathing rate
    
    // Verify both systems work
    TEST_ASSERT_TRUE(pitchDetector.hasPitch());
    TEST_ASSERT_TRUE(breathingDetector.hasPattern());
    
    // Check for interference
    TEST_ASSERT_FLOAT_WITHIN(0.1, 440.0, pitchDetector.getFrequency());
    TEST_ASSERT_FLOAT_WITHIN(0.5, 14.5, breathingDetector.getRate());
}
```

### Performance Validation
- **Latency**: <100ms for combined processing
- **Accuracy**: >90% for both pitch and breathing
- **Memory**: <32KB total additional usage
- **CPU**: <15% overhead for hybrid mode

## 🔮 Future Integrations

### Planned Modules
- **Heart Rate Detection**: PPG sensor integration
- **Movement Analysis**: Accelerometer-based activity detection
- **Environmental Monitoring**: Temperature, humidity, air quality
- **Voice Analysis**: Speech pattern recognition
- **Sleep Tracking**: Advanced sleep stage detection

### Extension Framework
```cpp
// Base class for new integrations
class AudioIntegration {
public:
    virtual bool begin() = 0;
    virtual bool process(const int32_t* buffer, int size) = 0;
    virtual String getJSON() = 0;
    virtual void display() = 0;
};

// New integrations inherit from this base class
class HeartRateDetector : public AudioIntegration {
    // Implementation...
};
```

## 📚 Documentation Structure

```
docs/integrations/
├── README.md              # This file - overview and setup
├── oroborusunity.md       # Breathing detection integration
└── [future integrations]  # Additional integration docs
```

## 🤝 Contributing

### Adding New Integrations
1. **Create integration class** inheriting from AudioIntegration
2. **Implement required methods** (begin, process, getJSON, display)
3. **Add documentation** in this directory
4. **Create tests** for the new integration
5. **Update examples** with usage scenarios

### Integration Guidelines
- **Shared Resources**: Use common audio input and display
- **Performance**: Minimize CPU and memory overhead
- **Consistency**: Follow established patterns and APIs
- **Documentation**: Comprehensive setup and usage guides
- **Testing**: Include unit and integration tests

---

**Ready to explore integrations? Start with [OroborusUnity](./oroborusunity.md) or check the [Development Guide](../development/README.md) for technical details!**
