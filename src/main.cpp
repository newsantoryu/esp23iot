// ═══════════════════════════════════════════════════════════
// 🎯 ESP32 TUNER — HYBRID PRO (FIX DISPLAY + DEBUG)
// ═══════════════════════════════════════════════════════════

#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "utils/Logger.h"
#include "utils/NoiseFilter.h"
#include "interfaces/WiFiStreamer.h"
#include "utils/DebugDashboard.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ───────── PINOS ─────────
#define LED_PIN 14
#define I2S_WS  25
#define I2S_SD  34
#define I2S_SCK 26

#define I2S_PORT I2S_NUM_0
#define BUFFER_LEN 1024  // 🔥 AUMENTADO para maior precisão
#define A4_FREQ 432.0


int32_t sBuffer[BUFFER_LEN];
float yinBuffer[BUFFER_LEN/2];

// ───────── AUDIO ─────────
float envelope = 0;
float alpha = 0.1;
float dynamicMax = 100;
bool isActive = false;

NoiseFilter noiseFilter;
WiFiStreamer streamer;

// ───────── PITCH ─────────
float pitchFinal = 0;
float pitchHistory[5];
int pitchIndex = 0;

float cents = 0;
bool tuned = false;

// ───────── NOTE WITH OCTAVE ─────────
String displayNoteWithOctave = "---";

// 🔥 SISTEMA DE CONFIANÇA
float confidenceLevel = 0.0;
const float MIN_CONFIDENCE = 0.3;  // 🔥 Reduzido para permitir reconhecimento

// ───────── PITCH VALIDATION ─────────
float lastValidPitch = 0;
unsigned long lastValidPitchTime = 0;
const float MIN_PITCH_FREQ = 60.0;   // 🔥 Reduzido para C2 (65.4Hz)
const float MAX_PITCH_FREQ = 800.0;  // Frequência máxima válida
const float PITCH_COHERENCE_TOLERANCE = 0.3; // 30% tolerância
const unsigned long PITCH_TIMEOUT = 2000; // 2s timeout

// ───────── NOTE ─────────
String currentNote = "---";
String lockedNote  = "---";
int stabilityCounter = 0;
int noteChangeCounter = 0;
const int NOTE_STABILITY_THRESHOLD = 8;  // 🔥 Reduzido para permitir reconhecimento
const int NOTE_CHANGE_THRESHOLD = 6;     // Mais histerese para mudanças
const unsigned long NOTE_COOLDOWN = 300;  // 🔥 Reduzido cooldown para 300ms

// ───────── DEBUG MODE ─────────
bool debugMode = true;   // 🔥 ATIVADO para ver logs de oitava
bool testMode = false;    // Modo de teste sequencial
unsigned long noteChangeCount = 0;
unsigned long lastNoteChangeTime = 0;

// ───────── DISPLAY ESTABILITY ─────────
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100; // 10Hz max
String displayNote = "---";
bool noteStable = false;

const char* noteNames[] = {
  "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

// ───────── FILTRO PASSA-BAIXA ─────────
float lowPassFilter(float input, float cutoff, float sampleRate) {
  static float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
  
  float RC = 1.0 / (2 * 3.14159 * cutoff);
  float dt = 1.0 / sampleRate;
  float alpha = dt / (RC + dt);
  
  y2 = y1 + alpha * (input - y1);
  y1 = y2;
  
  return y1;
}

// ═══════════════════════════════════════════════════════════
// 🎯 ZERO CROSS COM VALIDAÇÃO ROBUSTA
// ═══════════════════════════════════════════════════════════
float detectPitch(int32_t* buffer, int len, int sampleRate) {

  // 🔥 APLICAR FILTRO PASSA-BAIXA
  float filteredBuffer[len];
  for (int i = 0; i < len; i++) {
    int16_t sample = buffer[i] >> 16;
    filteredBuffer[i] = lowPassFilter((float)sample, 800.0, sampleRate);
  }

  int crossings = 0;
  long signalSum = 0;
  long signalSquared = 0;

  for (int i = 1; i < len; i++) {
    float a = filteredBuffer[i-1];
    float b = filteredBuffer[i];
    
    // Acumular estatísticas do sinal
    signalSum += fabs(a);
    signalSquared += (long)a * a;

    // Zero cross detection
    if ((a < 0 && b >= 0) || (a > 0 && b <= 0)) {
      crossings++;
    }
  }

  // Calcular frequência primeiro
  float calculatedFreq = (crossings * sampleRate) / (2.0 * len);

  // Validação mínima de crossings (ajustada por frequência)
  int minCrossings = 6;
  if (calculatedFreq < 100.0) minCrossings = 8;  // Mais exigente para baixas frequências
  if (calculatedFreq < 80.0) minCrossings = 10; // Ainda mais exigente para C2
  
  if (crossings < minCrossings) return 0;

  // Filtrar frequências inválidas
  if (calculatedFreq < MIN_PITCH_FREQ || calculatedFreq > MAX_PITCH_FREQ) {
    Logger::pitch("Pitch out of range: " + String(calculatedFreq));
    return 0;
  }

  // Calcular SNR simples
  float avgSignal = (float)signalSum / len;
  float avgPower = (float)signalSquared / len;
  float snr = (avgPower > 0) ? (avgSignal * avgSignal) / avgPower : 0;
  
  // Rejeitar se SNR muito baixo (ruído)
  if (snr < 0.01) {
    Logger::pitch("Low SNR pitch rejected: " + String(calculatedFreq) + " SNR:" + String(snr));
    return 0;
  }

  Logger::pitch("Valid pitch: " + String(calculatedFreq) + " SNR:" + String(snr));
  return calculatedFreq;
}

// ═══════════════════════════════════════════════════════════
// 🎯 SMOOTH
// ═══════════════════════════════════════════════════════════
float smooth(float v) {

  pitchHistory[pitchIndex] = v;
  pitchIndex = (pitchIndex + 1) % 5;

  float sum = 0;
  int count = 0;

  for (int i = 0; i < 5; i++) {
    if (pitchHistory[i] > 0) {
      sum += pitchHistory[i];
      count++;
    }
  }

  return count ? sum / count : 0;
}

// ═══════════════════════════════════════════════════════════
// 🎯 ANALISE (NUNCA FICA SEM NOTA)
// ═══════════════════════════════════════════════════════════
// ═══════════════════════════════════════════════════════════
// 🎯 ANALISE COM BLOQUEIO DE RUÍDO
// ═══════════════════════════════════════════════════════════
void analyze(float freq) {

  // 🔥 BLOQUEIO CRÍTICO: Não analisar se sinal inativo
  if (!isActive) {
    Logger::pitch("Signal inactive - blocking analysis");
    // Resetar estado apenas se estava ativo antes
    if (currentNote != "---") {
      currentNote = "---";
      lockedNote = "---";
      displayNote = "---";
      displayNoteWithOctave = "---";  // Resetar nota com oitava
      noteStable = false;
      tuned = false;
      stabilityCounter = 0;
      noteChangeCounter = 0;
      Logger::pitch("Reset state due to inactivity");
    }
    return;
  }

  Logger::pitch("Freq: " + String(freq) + " Current: " + currentNote + " Locked: " + lockedNote);

  // Validação de frequência
  if (freq < MIN_PITCH_FREQ || freq > MAX_PITCH_FREQ) {
    Logger::pitch("Frequency out of valid range: " + String(freq));
    currentNote = "---";
    displayNoteWithOctave = "---";  // Resetar nota com oitava
    tuned = false;
    noteStable = false;
    return;
  }

  // Verificação de coerência com último pitch válido
  unsigned long currentTime = millis();
  if (lastValidPitch > 0 && (currentTime - lastValidPitchTime) < PITCH_TIMEOUT) {
    float coherence = fabs(freq - lastValidPitch) / lastValidPitch;
    if (coherence > PITCH_COHERENCE_TOLERANCE) {
      Logger::pitch("Incoherent pitch rejected: " + String(freq) + " Coherence:" + String(coherence));
      return; // Rejeitar pitch incoerente
    }
  }

  // Atualizar último pitch válido
  lastValidPitch = freq;
  lastValidPitchTime = currentTime;

  float note = 69 + 12 * log2(freq / A4_FREQ);
  int rounded = round(note);

  int idx = (rounded % 12 + 12) % 12;
  String newNote = noteNames[idx];

  // 🔥 CÁLCULO DA OITAVA CORRIGIDO
  // Fórmula 1: Padrão MIDI (C-1=0, C0=12, C1=24, C2=36, C3=48, C4=60, A4=69)
  int octave1 = (rounded - 12) / 12;
  
  // Fórmula 2: Ajuste direto (testar qual funciona)
  int octave2 = (rounded / 12) - 1;
  
  // 🔥 DEBUG COMPARATIVO
  Logger::pitch("OCTAVE COMPARISON - Freq: " + String(freq) + 
                " Note: " + String(note) + 
                " Rounded: " + String(rounded) + 
                " Octave1: " + String(octave1) +
                " Octave2: " + String(octave2) +
                " A4_FREQ: " + String(A4_FREQ));
  
  // 🔥 ESCOLHER FÓRMULA CORRETA (testar octave1 primeiro)
  int octave = octave1;  // Tentar padrão MIDI
  
  // 🔥 VALIDAÇÃO ESPECÍFICA
  if (freq > 430 && freq < 435) {  // A4 range
    if (octave != 4) {
      Logger::pitch("ERROR: A4 octave should be 4, got " + String(octave) + " - trying formula 2");
      octave = octave2;  // Fallback para fórmula 2
    }
  }
  
  String noteWithOctave = newNote + String(octave);  // "A4", "C3", "G5"
  displayNoteWithOctave = noteWithOctave;

  float ideal = A4_FREQ * pow(2, (rounded - 69) / 12.0);
  cents = 1200 * log2(freq / ideal);

  // 🔥 VERIFICAÇÃO 432Hz PATTERN
  Logger::pitch("432Hz Reference - Freq: " + String(freq) + 
                " Note: " + newNote + 
                " Ideal: " + String(ideal) + 
                " Cents: " + String(cents) + 
                " A4_REF: " + String(A4_FREQ));

  // Contador de mudanças de nota para debug
  if (newNote != currentNote && millis() - lastNoteChangeTime > 1000) {
    noteChangeCount++;
    lastNoteChangeTime = millis();
    Logger::pitch("Note change #" + String(noteChangeCount) + ": " + currentNote + " -> " + newNote);
  }

  // 🔥 COOLDOWN ENTRE MUDANÇAS
  static unsigned long lastNoteChange = 0;
  if (newNote != currentNote && (millis() - lastNoteChange) < NOTE_COOLDOWN) {
    Logger::pitch("Note change blocked by cooldown: " + newNote);
    return; // Bloquear mudança muito rápida
  }

  // Sistema melhorado de estabilidade com confiança
  if (newNote == currentNote) {
    stabilityCounter++;
    noteChangeCounter = 0;
    // 🔥 AUMENTAR CONFIANÇA GRADUALMENTE
    confidenceLevel = fmin(confidenceLevel + 0.05, 1.0);
  } else {
    noteChangeCounter++;
    confidenceLevel = 0.0; // Resetar confiança na mudança
    // Só muda se tiver histerese suficiente E confiança mínima
    if (noteChangeCounter >= NOTE_CHANGE_THRESHOLD && confidenceLevel >= MIN_CONFIDENCE) {
      currentNote = newNote;
      stabilityCounter = 0;
      noteChangeCounter = 0;
      lastNoteChange = millis(); // 🔥 Registrar mudança
      Logger::pitch("Note changed to: " + newNote + " (confidence: " + String(confidenceLevel) + ")");
    }
  }

  // Nota considerada estável apenas após threshold E confiança
  if (stabilityCounter >= NOTE_STABILITY_THRESHOLD && confidenceLevel >= MIN_CONFIDENCE) {
    lockedNote = currentNote;
    displayNote = lockedNote;
    displayNoteWithOctave = noteWithOctave;  // Atualizar nota com oitava
    noteStable = true;
    Logger::pitch("Note locked: " + lockedNote + " (" + noteWithOctave + ") - confidence: " + String(confidenceLevel) + ")");
  } else if (currentNote != "---" && stabilityCounter > 1) {
    // Mostra nota detectada mas não confirmada
    displayNote = currentNote;
    displayNoteWithOctave = noteWithOctave;  // Atualizar nota com oitava
    noteStable = false;
  }

  tuned = fabs(cents) < 5 && noteStable;
}

// ═══════════════════════════════════════════════════════════
// 🎯 UI COM CONTROLE RIGOROSO (432Hz PATTERN)
// ═══════════════════════════════════════════════════════════
void drawUI(float norm) {

  // 🔥 BLOQUEIO CRÍTICO: Não atualizar se sinal inativo
  if (!isActive) {
    static unsigned long lastInactiveUpdate = 0;
    unsigned long currentTime = millis();
    // Só atualizar display inativo a cada 2 segundos
    if (currentTime - lastInactiveUpdate < 2000) {
      return;
    }
    lastInactiveUpdate = currentTime;
  } else {
    // Controle de taxa para sinal ativo
    unsigned long currentTime = millis();
    if (currentTime - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
      return; // Pula atualização se muito rápido
    }
    lastDisplayUpdate = currentTime;
  }

  display.clearDisplay();

  // ───────── LINHA 1 (INFO COMPACTA COM 432Hz) ─────────
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("Hz:");
  display.print((int)pitchFinal);

  display.setCursor(64, 0);
  display.print("432:"); // Indicador de padrão
  if (pitchFinal > 0 && isActive) {
    float deviation = pitchFinal - 432.0;
    display.print((int)deviation);
  } else {
    display.print("---");
  }

  // ───────── NOTA GRANDE CENTRAL COM OITAVA ─────────
  display.setTextSize(3);
  display.setCursor(34, 14);

  if (displayNoteWithOctave != "---" && isActive) {
    // Indicador visual de estabilidade
    if (noteStable) {
      // Ajustar tamanho se "A4" não couber
      if (displayNoteWithOctave.length() <= 2) {
        display.print(displayNoteWithOctave); // "A4", "C3", etc.
      } else {
        // Nota com oitava longa - reduzir tamanho
        display.setTextSize(2);
        display.setCursor(24, 18);
        display.print(displayNoteWithOctave);
      }
    } else {
      // Nota detectada mas não estável - mostra menor
      display.setTextSize(2);
      display.setCursor(44, 18);
      display.print(displayNoteWithOctave);
    }
  } else {
    display.print("--");
  }

  // ───────── STATUS MELHORADO ─────────
  display.setTextSize(1);
  display.setCursor(0, 44);

  if (!isActive) {
    display.print("."); // Idle
  } else if (tuned) {
    display.print("OK"); // Afinação perfeita em 432Hz
  } else if (cents > 0) {
    display.print("DN"); // Desce
  } else {
    display.print("UP"); // Sobe
  }

  // ───────── CENTS + INDICADOR DE ESTABILIDADE ─────────
  display.setCursor(90, 44);
  if (noteStable && isActive) {
    display.printf("%+d", (int)cents);
  } else {
    display.print("~"); // Indicador de instabilidade
  }

  // ───────── BARRA DE VOLUME ─────────
  int bar = norm * 128;
  display.drawRect(0, 58, 128, 4, SSD1306_WHITE);
  display.fillRect(0, 58, bar, 4, SSD1306_WHITE);

  display.display();
}
// ═══════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════
void setup() {

  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // 🔥 INICIALIZA SISTEMA DE LOGS
  Logger::setLevel(LOG_INFO); // Nível de produção para reduzir spam
  Logger::info("=== ESP32 Audio Pitch Engine Starting ===");
  Logger::info("Version: 0.3.0 - Standalone (No Backend)");
  Logger::info("Activity threshold: 0.35 (increased from 0.25)");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  // WIFI STREAMER DESATIVADO - Não usando backend
  Logger::info("=== WiFi Streaming Disabled ===");
  Logger::info("ESP32 operating in standalone mode");

  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .dma_buf_count = 4,
    .dma_buf_len = 128
  };

  i2s_pin_config_t pins = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
}

// ───────── LOOP RATE CONTROL ─────────
unsigned long lastLoopTime = 0;
const unsigned long LOOP_RATE_LIMIT = 100; // 10Hz max para output

// ═══════════════════════════════════════════════════════════
// LOOP
// ═══════════════════════════════════════════════════════════
void loop() {

  // 🔥 WiFiStreamer desativado - não usando backend
  // streamer.loop();

  size_t bytesIn = 0;
  i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytesIn, portMAX_DELAY);

  int samples = bytesIn / sizeof(int32_t);
  if (samples == 0) return;

  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += abs(sBuffer[i] >> 16);
  }

  float amp = (float)sum / samples;
  envelope += alpha * (amp - envelope);

  if (envelope > dynamicMax) dynamicMax = envelope;
  dynamicMax *= 0.999;

  float norm = envelope / dynamicMax;
  
  // 🔥 MELHOR FILTRAGEM DE RUÍDO COM NOISEFILTER
  isActive = noiseFilter.isSignalActive(amp, norm);

  // 🔥 DETECÇÃO SIMPLES (FUNCIONA SEM BUG)
  float pitch = detectPitch(sBuffer, samples, 16000);
  pitchFinal = smooth(pitch);

  // 🔥 CONVERTER PARA PADRÃO 432 Hz
  if (pitchFinal > 0) {
    pitchFinal = pitchFinal * (432.0 / 440.0); // Converter de 440 Hz para 432 Hz
  }

  analyze(pitchFinal);

  // 🔥 STREAM DE DADOS PARA IOT MANAGER - DESATIVADO
  // Não usando backend - ESP32 operando como dispositivo standalone

  digitalWrite(LED_PIN, tuned && isActive);

  drawUI(pitchFinal);

  // 🔥 OUTPUT CONTROLADO COM RATE LIMITING
  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime >= LOOP_RATE_LIMIT) {
    // Só mostrar Hz se tiver pitch válido
    if (pitchFinal > 0 && isActive) {
      String output = "Hz:" + String((int)pitchFinal) + 
                       " Note:" + currentNote + 
                       " cents:" + String((int)cents);
      Logger::info(output);
    } else if (!isActive) {
      Logger::info("Signal inactive - Hz:--- Note:---");
    }
    lastLoopTime = currentTime;
  }
  
  // 🔥 DEBUG DASHBOARD
  DebugDashboard::update(
    pitchFinal,
    currentNote,
    cents,
    confidenceLevel,
    isActive,
    noteStable
  );
}