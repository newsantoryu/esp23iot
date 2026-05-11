# Examples - ESP32 Audio Pitch Engine

This directory contains example code demonstrating various features and use cases of the ESP32 Audio Pitch Engine.

## 📁 Available Examples

### 1. **basic_usage.cpp** - Basic Pitch Detection
A simple example that demonstrates the core functionality:
- I2S audio input setup
- Basic pitch detection using autocorrelation
- OLED display output
- Real-time frequency and note display

**Features:**
- Minimal configuration for quick setup
- Simplified pitch detection algorithm
- Basic noise filtering
- Visual feedback on display

**Usage:**
```bash
# Copy to main.cpp or use as reference
cp examples/basic_usage.cpp src/main.cpp
pio run --target upload
pio device monitor
```

### 2. **wifi_streaming.cpp** - WiFi Data Streaming
Demonstrates how to stream pitch data over WiFi:
- WiFi connection management
- JSON data formatting
- HTTP POST to remote server
- Error handling and reconnection

**Features:**
- Real-time data streaming
- Automatic reconnection
- Buffer management
- Health monitoring

### 3. **breathing_detection.cpp** - OroborusUnity Integration
Demonstrates breathing pattern detection using the OroborusUnity integration:
- Real-time breathing pattern analysis
- Breathing rate calculation (BPM)
- Pattern classification (normal, deep, shallow, irregular)
- Visual feedback and data streaming

**Features:**
- Breathing pattern recognition
- Biometric health monitoring
- Meditation support
- WiFi streaming of breathing data

### 4. **advanced_filtering.cpp** - Advanced Noise Filtering
Shows advanced noise filtering techniques:
- Multi-stage filtering
- Adaptive threshold adjustment
- SNR-based validation
- Coherence checking

**Features:**
- High noise immunity
- Configurable sensitivity
- Performance optimization
- Detailed diagnostics

### 5. **multi_instrument.cpp** - Multi-Instrument Profiles
Demonstrates instrument-specific tuning:
- Different tuning profiles
- Instrument selection
- Custom frequency ranges
- Profile persistence

**Features:**
- Guitar, violin, vocal profiles
- Custom tunings
- Profile management
- Quick switching

## 🚀 Getting Started

### Prerequisites
- ESP32 development board
- OLED display (SSD1306)
- I2S microphone or audio input
- PlatformIO installed

### Basic Setup
1. Choose an example based on your needs
2. Copy the example to `src/main.cpp`
3. Configure pins and settings
4. Build and upload

### Hardware Connections
```
ESP32 → OLED Display
GPIO21 → SDA
GPIO22 → SCL
3.3V  → VCC
GND   → GND

ESP32 → I2S Microphone
GPIO25 → WS
GPIO26 → SCK
GPIO34 → SD
3.3V  → VCC
GND   → GND
```

## 📖 Example Details

### Basic Usage Example

This example is perfect for:
- First-time users
- Quick testing
- Learning the basics
- Simple applications

**Key Components:**
- I2S configuration
- Basic pitch detection
- Display management
- Simple UI

**Expected Output:**
```
Note: A4, Freq: 440.2Hz, Cents: +2.1
Note: G3, Freq: 196.0Hz, Cents: -0.5
```

### WiFi Streaming Example

This example shows how to:
- Connect to WiFi networks
- Stream data to servers
- Handle connection issues
- Format JSON data

**Data Format:**
```json
{
  "device_id": "esp32_pitch_001",
  "timestamp": 1634567890,
  "frequency": 440.2,
  "note": "A",
  "octave": 4,
  "cents": 2.1,
  "confidence": 0.95,
  "stable": true
}
```

### Advanced Filtering Example

Demonstrates professional-grade filtering:
- Multi-stage noise rejection
- Adaptive threshold adjustment
- SNR-based validation
- Performance metrics

**Filtering Stages:**
1. **Amplitude Thresholding**: Basic activity detection
2. **Noise Floor Estimation**: Adaptive noise level
3. **SNR Validation**: Signal quality check
4. **Coherence Testing**: Frequency stability
5. **Hysteresis**: Prevent flickering

### Multi-Instrument Example

Shows instrument-specific tuning:
- Different frequency ranges
- Custom tunings
- Instrument profiles
- Quick switching

**Supported Instruments:**
- **Guitar**: Standard (E-A-D-G-B-E), Drop D, Open G
- **Violin**: Standard tuning (G-D-A-E)
- **Vocal**: Voice range optimization
- **Custom**: User-defined profiles

## 🔧 Customization

### Modifying Examples

Each example can be customized:

#### Pin Configuration
```cpp
// Change I2S pins
#define I2S_WS  25
#define I2S_SD  34
#define I2S_SCK 26

// Change display pins
#define OLED_SDA 21
#define OLED_SCL 22
```

#### Audio Settings
```cpp
// Sample rate
#define SAMPLE_RATE 16000

// Buffer size
#define BUFFER_SIZE 1024

// Update rate
#define UPDATE_RATE 100  // ms
```

#### Display Settings
```cpp
// Screen size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C address
#define OLED_ADDRESS 0x3C
```

### Adding New Features

1. **Copy existing example** as starting point
2. **Modify configuration** for your needs
3. **Add new functionality** step by step
4. **Test thoroughly** with real hardware
5. **Document changes** for future reference

## 🐛 Troubleshooting

### Common Issues

#### No Audio Detection
- Check I2S connections
- Verify microphone power
- Adjust sensitivity threshold
- Check sample rate settings

#### Display Not Working
- Verify I2C connections
- Check OLED address (0x3C vs 0x3D)
- Ensure proper power supply
- Test with I2C scanner

#### WiFi Connection Issues
- Verify SSID and password
- Check signal strength
- Test with phone hotspot
- Review firewall settings

#### Build Errors
- Update PlatformIO libraries
- Check board configuration
- Verify include paths
- Clean build directory

### Debug Tips

#### Serial Output
```bash
# Monitor with filtering
pio device monitor --filter esp32_exception_decoder

# Verbose output
pio device monitor -b 115200 --echo
```

#### Performance Monitoring
```cpp
// Add timing measurements
unsigned long start = micros();
// ... code ...
unsigned long duration = micros() - start;
Serial.printf("Processing time: %lu us\n", duration);
```

#### Memory Usage
```cpp
// Check free heap
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("Min free heap: %d bytes\n", ESP.getMinFreeHeap());
```

## 📚 Learning Resources

### Documentation
- [User Guide](../docs/user/README.md)
- [API Reference](../docs/api/API_REFERENCE.md)
- [Development Guide](../docs/development/README.md)

### External Resources
- [ESP32 Arduino Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [I2S Audio Tutorial](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html)
- [SSD1306 OLED Guide](https://learn.adafruit.com/adafruit-gfx-graphics-library)
- [PlatformIO Documentation](https://docs.platformio.org/)

### Community
- **GitHub Issues**: Report bugs and request features
- **Discussions**: Ask questions and share ideas
- **Discord**: Real-time chat with community

## 🤝 Contributing

Have an idea for a new example? We'd love to see it!

### Submitting Examples

1. **Create a new file** in the examples directory
2. **Follow naming convention**: `descriptive_name.cpp`
3. **Add documentation** at the top of the file
4. **Update this README** with your example
5. **Test thoroughly** on real hardware
6. **Submit a pull request**

### Example Template

```cpp
/*
 * ESP32 Audio Pitch Engine - Example Name
 * 
 * Description of what this example demonstrates
 * 
 * Features:
 * - Feature 1
 * - Feature 2
 * - Feature 3
 * 
 * Hardware Requirements:
 * - ESP32 board
 * - OLED display
 * - I2S microphone
 * 
 * Author: Your Name
 * Date: YYYY-MM-DD
 */

#include <Arduino.h>
// ... includes ...

void setup() {
    // ... setup code ...
}

void loop() {
    // ... main loop ...
}
```

---

**Ready to start? Choose an example and begin building your audio application!**
