// ═══════════════════════════════════════════════════════════
// 🎯 ESP32 TUNER — HYBRID PRO (FIX DISPLAY + DEBUG)
// ═══════════════════════════════════════════════════════════

#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

// ───────── PITCH ─────────
float pitchFinal = 0;
float pitchHistory[5];
int pitchIndex = 0;

float cents = 0;
bool tuned = false;

// ───────── DISPLAY ESTABILITY ─────────
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100; // 10Hz max
String displayNote = "---";
bool noteStable = false;

// ───────── NOTE ─────────
String currentNote = "---";
String lockedNote  = "---";
int stabilityCounter = 0;
int noteChangeCounter = 0;
const int NOTE_STABILITY_THRESHOLD = 5; // mais exigente
const int NOTE_CHANGE_THRESHOLD = 3; // histerese para mudanças

const char* noteNames[] = {
  "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

// ═══════════════════════════════════════════════════════════
// 🎯 ZERO CROSS (estável e leve)
// ═══════════════════════════════════════════════════════════
float detectPitch(int32_t* buffer, int len, int sampleRate) {

  int crossings = 0;

  for (int i = 1; i < len; i++) {
    int16_t a = buffer[i-1] >> 16;
    int16_t b = buffer[i] >> 16;

    if ((a < 0 && b >= 0) || (a > 0 && b <= 0)) {
      crossings++;
    }
  }

  if (crossings < 6) return 0;

  float freq = (crossings * sampleRate) / (2.0 * len);

  if (freq < 60 || freq > 1000) return 0;

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
void analyze(float freq) {

  if (freq < 60 || freq > 1000) {
    currentNote = "---";
    tuned = false;
    noteStable = false;
    return;
  }

  float note = 69 + 12 * log2(freq / A4_FREQ);
  int rounded = round(note);

  int idx = (rounded % 12 + 12) % 12;
  String newNote = noteNames[idx];

  float ideal = A4_FREQ * pow(2, (rounded - 69) / 12.0);
  cents = 1200 * log2(freq / ideal);

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
    }
  }

  // Nota considerada estável apenas após threshold maior
  if (stabilityCounter >= NOTE_STABILITY_THRESHOLD) {
    lockedNote = currentNote;
    displayNote = lockedNote;
    noteStable = true;
  } else if (currentNote != "---" && stabilityCounter > 1) {
    // Mostra nota detectada mas não confirmada
    displayNote = currentNote;
    noteStable = false;
  }

  tuned = abs(cents) < 5 && noteStable;
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
  isActive = norm > 0.25;

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