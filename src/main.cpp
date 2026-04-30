// ═══════════════════════════════════════════════════════════
// 🎯 ESP32 TUNER — HYBRID PRO (FIX DISPLAY + DEBUG)
// ═══════════════════════════════════════════════════════════

#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "utils/Logger.h"
#include "utils/NoiseFilter.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ───────── PINOS ─────────
#define LED_PIN 14
#define I2S_WS  25
#define I2S_SD  34
#define I2S_SCK 26

#define I2S_PORT I2S_NUM_0
#define BUFFER_LEN 256
#define A4_FREQ 432.0

int32_t sBuffer[BUFFER_LEN];
float yinBuffer[BUFFER_LEN/2];

// ───────── AUDIO ─────────
float envelope = 0;
float alpha = 0.1;
float dynamicMax = 100;
bool isActive = false;

NoiseFilter noiseFilter;

// ───────── PITCH ─────────
float pitchFinal = 0;
float pitchHistory[5];
int pitchIndex = 0;

float cents = 0;
bool tuned = false;

// ───────── PITCH VALIDATION ─────────
float lastValidPitch = 0;
unsigned long lastValidPitchTime = 0;
const float MIN_PITCH_FREQ = 80.0;   // Frequência mínima válida
const float MAX_PITCH_FREQ = 800.0;  // Frequência máxima válida
const float PITCH_COHERENCE_TOLERANCE = 0.3; // 30% tolerância
const unsigned long PITCH_TIMEOUT = 2000; // 2s timeout

// ───────── NOTE ─────────
String currentNote = "---";
String lockedNote  = "---";
int stabilityCounter = 0;
int noteChangeCounter = 0;
const int NOTE_STABILITY_THRESHOLD = 5; // mais exigente
const int NOTE_CHANGE_THRESHOLD = 3; // histerese para mudanças

// ───────── DEBUG MODE ─────────
bool debugMode = false;
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

// ═══════════════════════════════════════════════════════════
// 🎯 ZERO CROSS COM VALIDAÇÃO ROBUSTA
// ═══════════════════════════════════════════════════════════
float detectPitch(int32_t* buffer, int len, int sampleRate) {

  int crossings = 0;
  long signalSum = 0;
  long signalSquared = 0;

  for (int i = 1; i < len; i++) {
    int16_t a = buffer[i-1] >> 16;
    int16_t b = buffer[i] >> 16;
    
    // Acumular estatísticas do sinal
    signalSum += abs(a);
    signalSquared += (long)a * a;

    // Zero cross detection
    if ((a < 0 && b >= 0) || (a > 0 && b <= 0)) {
      crossings++;
    }
  }

  // Validação mínima de crossings
  if (crossings < 6) return 0;

  // Calcular frequência
  float freq = (crossings * sampleRate) / (2.0 * len);

  // Filtrar frequências inválidas
  if (freq < MIN_PITCH_FREQ || freq > MAX_PITCH_FREQ) {
    Logger::pitch("Pitch out of range: " + String(freq));
    return 0;
  }

  // Calcular SNR simples
  float avgSignal = (float)signalSum / len;
  float avgPower = (float)signalSquared / len;
  float snr = (avgPower > 0) ? (avgSignal * avgSignal) / avgPower : 0;
  
  // Rejeitar se SNR muito baixo (ruído)
  if (snr < 0.01) {
    Logger::pitch("Low SNR pitch rejected: " + String(freq) + " SNR:" + String(snr));
    return 0;
  }

  Logger::pitch("Valid pitch: " + String(freq) + " SNR:" + String(snr));
  return freq;
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

  float ideal = A4_FREQ * pow(2, (rounded - 69) / 12.0);
  cents = 1200 * log2(freq / ideal);

  // Contador de mudanças de nota para debug
  if (newNote != currentNote && millis() - lastNoteChangeTime > 1000) {
    noteChangeCount++;
    lastNoteChangeTime = millis();
    Logger::pitch("Note change #" + String(noteChangeCount) + ": " + currentNote + " -> " + newNote);
  }

  // Sistema melhorado de estabilidade
  if (newNote == currentNote) {
    stabilityCounter++;
    noteChangeCounter = 0;
  } else {
    noteChangeCounter++;
    // Só muda se tiver histerese suficiente
    if (noteChangeCounter >= NOTE_CHANGE_THRESHOLD) {
      currentNote = newNote;
      stabilityCounter = 0;
      noteChangeCounter = 0;
      Logger::pitch("Note changed to: " + newNote);
    }
  }

  // Nota considerada estável apenas após threshold maior
  if (stabilityCounter >= NOTE_STABILITY_THRESHOLD) {
    lockedNote = currentNote;
    displayNote = lockedNote;
    noteStable = true;
    Logger::pitch("Note locked: " + lockedNote);
  } else if (currentNote != "---" && stabilityCounter > 1) {
    // Mostra nota detectada mas não confirmada
    displayNote = currentNote;
    noteStable = false;
  }

  tuned = fabs(cents) < 5 && noteStable;
}

// ═══════════════════════════════════════════════════════════
// 🎯 UI FIXADA (SEMPRE VISÍVEL)
// ═══════════════════════════════════════════════════════════
void drawUI(float norm) {

  // Controle de taxa de atualização
  unsigned long currentTime = millis();
  if (currentTime - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
    return; // Pula atualização se muito rápido
  }
  lastDisplayUpdate = currentTime;

  display.clearDisplay();

  // ───────── LINHA 1 (INFO COMPACTA) ─────────
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("Hz:");
  display.print((int)pitchFinal);

  display.setCursor(64, 0);
  display.print("Nt:");
  display.print(displayNote);

  // ───────── NOTA GRANDE CENTRAL ─────────
  display.setTextSize(3);
  display.setCursor(34, 14);

  if (displayNote != "---") {
    // Indicador visual de estabilidade
    if (noteStable) {
      display.print(displayNote); // Nota estável
    } else {
      // Nota detectada mas não estável - mostra menor
      display.setTextSize(2);
      display.setCursor(44, 18);
      display.print(displayNote);
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
    display.print("OK"); // Afinação perfeita
  } else if (cents > 0) {
    display.print("DN"); // Desce
  } else {
    display.print("UP"); // Sobe
  }

  // ───────── CENTS + INDICADOR DE ESTABILIDADE ─────────
  display.setCursor(90, 44);
  if (noteStable) {
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
  Logger::setLevel(LOG_DEBUG); // Mudar para LOG_INFO em produção
  Logger::info("=== ESP32 Audio Pitch Engine Starting ===");
  Logger::info("Version: 0.2.1 - Noise Filter + Debug");
  Logger::info("Activity threshold: 0.35 (increased from 0.25)");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);
display.setTextWrap(false);

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

// ═══════════════════════════════════════════════════════════
// LOOP
// ═══════════════════════════════════════════════════════════
void loop() {

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

  analyze(pitchFinal);

  digitalWrite(LED_PIN, tuned && isActive);

  drawUI(pitchFinal);

  Serial.print("Hz:");
  Serial.print(pitchFinal);
  Serial.print(" Note:");
  Serial.print(currentNote);
  Serial.print(" cents:");
  Serial.println(cents);
}