/*
 * ESP32 Audio Pitch Engine - Basic Usage Example
 * 
 * This example demonstrates the basic usage of the pitch detection
 * system with minimal configuration for quick setup and testing.
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "utils/NoiseFilter.h"
#include "interfaces/WiFiStreamer.h"

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2S pins
#define I2S_WS  25
#define I2S_SD  34
#define I2S_SCK 26
#define I2S_PORT I2S_NUM_0

// Audio buffer
#define BUFFER_LEN 1024
int32_t audioBuffer[BUFFER_LEN];

// Pitch detection
float currentFrequency = 0;
String currentNote = "";
int currentOctave = 0;
float currentCents = 0;

// Activity detection
float envelope = 0;
bool isActive = false;

// Components
NoiseFilter noiseFilter;

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Audio Pitch Engine - Basic Example");
    
    // Initialize display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Display initialization failed");
        return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Pitch Engine");
    display.println("Initializing...");
    display.display();
    
    delay(1000);
    
    // Initialize I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
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
    i2s_set_clk(I2S_PORT, 16000, 32, I2S_CHANNEL_MONO);
    
    // Initialize noise filter
    noiseFilter.setActivityThreshold(0.35);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Ready!");
    display.println("Play instrument");
    display.display();
    
    Serial.println("System ready. Play an instrument to detect pitch.");
}

void loop() {
    // Read audio data
    size_t bytes_read = 0;
    i2s_read(I2S_PORT, audioBuffer, sizeof(audioBuffer), &bytes_read, portMAX_DELAY);
    
    if (bytes_read > 0) {
        int samples_read = bytes_read / sizeof(int32_t);
        
        // Calculate envelope (RMS)
        float sum = 0;
        for (int i = 0; i < samples_read; i++) {
            float sample = (float)audioBuffer[i] / 2147483648.0f; // Normalize to -1 to 1
            sum += sample * sample;
        }
        envelope = sqrt(sum / samples_read);
        
        // Check for activity
        isActive = noiseFilter.isSignalActive(envelope, envelope);
        
        if (isActive) {
            // Simple pitch detection (simplified YIN algorithm)
            float frequency = calculatePitch(audioBuffer, samples_read);
            
            if (frequency > 80 && frequency < 800) {
                currentFrequency = frequency;
                currentNote = frequencyToNote(frequency, currentOctave);
                currentCents = calculateCents(frequency, currentNote, currentOctave);
                
                updateDisplay();
                
                // Serial output
                Serial.printf("Note: %s%d, Freq: %.2fHz, Cents: %+2.1f\n", 
                             currentNote.c_str(), currentOctave, currentFrequency, currentCents);
            }
        } else {
            // Clear display when no signal
            if (currentFrequency > 0) {
                currentFrequency = 0;
                currentNote = "";
                currentCents = 0;
                updateDisplay();
            }
        }
    }
    
    delay(10); // 100Hz update rate
}

// Simplified pitch detection (basic autocorrelation)
float calculatePitch(int32_t* buffer, int size) {
    // This is a simplified version - the full implementation
    // uses the YIN algorithm for better accuracy
    
    // Find the period using autocorrelation
    int min_period = 16000 / 800; // 800Hz max
    int max_period = 16000 / 80;  // 80Hz min
    
    float max_correlation = 0;
    int best_period = min_period;
    
    for (int period = min_period; period < max_period; period++) {
        float correlation = 0;
        float energy = 0;
        
        for (int i = 0; i < size - period; i++) {
            correlation += buffer[i] * buffer[i + period];
            energy += buffer[i] * buffer[i];
        }
        
        correlation /= energy;
        
        if (correlation > max_correlation) {
            max_correlation = correlation;
            best_period = period;
        }
    }
    
    if (max_correlation > 0.3) {
        return 16000.0f / best_period;
    }
    
    return 0;
}

// Convert frequency to musical note
String frequencyToNote(float frequency, int& octave) {
    const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    if (frequency <= 0) return "";
    
    // A4 = 440Hz (or 432Hz depending on tuning)
    const float A4 = 440.0f;
    
    // Calculate note number (where A4 = 69)
    float note_num = 69 + 12 * log2(frequency / A4);
    
    int note_index = round(note_num) % 12;
    octave = (round(note_num) - note_index) / 12;
    
    if (note_index < 0) note_index += 12;
    if (octave < 0) octave = 0;
    
    return String(notes[note_index]);
}

// Calculate cents deviation from perfect pitch
float calculateCents(float frequency, String note, int octave) {
    if (note.isEmpty()) return 0;
    
    // Calculate perfect frequency for this note
    const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int note_index = 0;
    
    for (int i = 0; i < 12; i++) {
        if (note == String(notes[i])) {
            note_index = i;
            break;
        }
    }
    
    // Calculate perfect frequency
    const float A4 = 440.0f;
    float perfect_freq = A4 * pow(2.0f, (octave - 4) + (note_index - 9) / 12.0f);
    
    // Calculate cents
    return 1200 * log2(frequency / perfect_freq);
}

void updateDisplay() {
    display.clearDisplay();
    
    if (currentFrequency > 0) {
        // Note and frequency
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.printf("%s%d", currentNote.c_str(), currentOctave);
        
        display.setTextSize(1);
        display.setCursor(90, 8);
        display.printf("%.1fHz", currentFrequency);
        
        // Cents
        display.setCursor(0, 24);
        display.printf("Cents: %+.1f", currentCents);
        
        // Tuning indicator
        display.setCursor(0, 36);
        if (abs(currentCents) < 5) {
            display.println("IN TUNE");
        } else if (currentCents > 0) {
            display.println("TOO HIGH");
        } else {
            display.println("TOO LOW");
        }
        
        // Volume bar
        display.setCursor(0, 48);
        display.println("Volume:");
        int bar_length = envelope * 50;
        display.drawRect(50, 48, 50, 8, SSD1306_WHITE);
        display.fillRect(50, 48, bar_length, 8, SSD1306_WHITE);
        
        // Activity indicator
        display.setCursor(0, 58);
        display.println(isActive ? "● ACTIVE" : "○ IDLE");
        
    } else {
        // No signal
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.println("No signal detected");
        display.println("Play instrument");
        display.println("or check connections");
    }
    
    display.display();
}
