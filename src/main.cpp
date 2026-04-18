#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// ═══════════════════════════════════════════════════════════
//  PINAGEM
// ═══════════════════════════════════════════════════════════
#define I2S_WS   25   // Word Select (LRCLK)
#define I2S_SD   34   // Serial Data (DOUT do mic)
#define I2S_SCK  26   // Bit Clock (BCLK)
#define LED_PIN  14   // LED indicador de atividade de voz

#define I2S_PORT   I2S_NUM_0
#define BUFFER_LEN 64  // 64 amostras x 4 bytes = 256 bytes por leitura

int32_t sBuffer[BUFFER_LEN];

// ═══════════════════════════════════════════════════════════
//  ENVELOPE — suavização da amplitude bruta
//  alpha: 0.0 = lentíssimo | 1.0 = sem suavização
// ═══════════════════════════════════════════════════════════
float envelope = 0;
float alpha    = 0.15;

// ═══════════════════════════════════════════════════════════
//  NOISE FLOOR ADAPTATIVO
//  alphaDown: velocidade de queda em silêncio (lento)
//  alphaUp:   velocidade de subida com ruído ambiente (lento)
//
//  ⚠️  CONGELADO enquanto voz estiver ativa — impede que o
//  noise floor suba junto com a voz e zere o sinal durante
//  notas sustentadas ou fala contínua
// ═══════════════════════════════════════════════════════════
float noiseFloor     = 5.0;
float noiseAlphaDown = 0.001;
float noiseAlphaUp   = 0.005;

// ═══════════════════════════════════════════════════════════
//  DYNAMIC MAX — normaliza o sinal para 0.0~1.0
//  Decai 0.05%/ciclo para se adaptar ao volume do ambiente
//  Congelado junto com noise floor durante voz ativa
// ═══════════════════════════════════════════════════════════
float dynamicMax = 100.0;

// ═══════════════════════════════════════════════════════════
//  DETECTOR DE VOZ — hangover de silêncio
//
//  Comportamento:
//    - LED acende assim que norm > thresholdOn
//    - LED SÓ apaga após SILENCE_HANGOVER ms de silêncio
//      contínuo (norm < thresholdOff por todo esse período)
//    - Durante voz ativa, lastVoiceMs é renovado a cada
//      ciclo — o LED nunca pisca no meio de uma fala
//
//  thresholdOn:      NORM mínima para ativar (↑ = menos sensível)
//  thresholdOff:     NORM mínima para manter (↓ = desativa mais fácil)
//  SILENCE_HANGOVER: ms de silêncio para apagar o LED
// ═══════════════════════════════════════════════════════════
bool  isActive         = false;
float thresholdOn      = 0.50;
float thresholdOff     = 0.15;

unsigned long lastVoiceMs      = 0;
const int     SILENCE_HANGOVER = 1500;

// ═══════════════════════════════════════════════════════════
//  CALIBRAÇÃO INICIAL — aprende o noise floor do ambiente
//  durante os primeiros CALIB_MS ms em silêncio
// ═══════════════════════════════════════════════════════════
bool          calibrating = true;
unsigned long startMs     = 0;
const int     CALIB_MS    = 2000;

// ═══════════════════════════════════════════════════════════
//  DEBUG
// ═══════════════════════════════════════════════════════════
unsigned long lastPrint = 0;
const int     PRINT_MS  = 100;

// ── Barra visual sem alocação dinâmica ──────────────────────
void printBar(float v, int width = 20) {
  int filled = constrain((int)(v * width), 0, width);
  Serial.print("[");
  for (int i = 0; i < width; i++) Serial.print(i < filled ? "#" : "-");
  Serial.print("]");
}

// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("╔══════════════════════════════╗");
  Serial.println("║     VAD — ESP32 + INMP441    ║");
  Serial.println("╚══════════════════════════════╝");
  pinMode(LED_PIN, OUTPUT);

  i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = 16000,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 4,
    .dma_buf_len          = 64,
    .use_apll             = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num   = I2S_SCK,
    .ws_io_num    = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);

  startMs = millis();
  Serial.println("[CALIB] Fique em silencio por 2s...");
}

// ════════════════════════════════════════════════════════════
//  LOOP
// ════════════════════════════════════════════════════════════
void loop() {
  size_t bytesIn = 0;
  i2s_read(I2S_PORT, (void*)sBuffer, sizeof(sBuffer), &bytesIn, portMAX_DELAY);
  if (bytesIn == 0) return;

  // ── 1. Amplitude média absoluta (MAE) ───────────────────
  // shift >>14: descarta os 14 LSBs de ruído do INMP441
  int  samples = bytesIn / sizeof(int32_t);
  long sum     = 0;
  for (int i = 0; i < samples; i++) {
    int32_t v = sBuffer[i] >> 16;
    sum += abs(v);
  }
  float amplitude = (float)sum / samples;

  // ── 2. Envelope exponencial ─────────────────────────────
  envelope += alpha * (amplitude - envelope);

  // ── 3. Calibração inicial ───────────────────────────────
  if (calibrating) {
    noiseFloor += 0.05 * (envelope - noiseFloor);
    if (millis() - startMs > CALIB_MS) {
      calibrating = false;
      dynamicMax  = noiseFloor * 5.0;
      Serial.print("[PRONTO] noiseFloor=");
      Serial.print(noiseFloor, 1);
      Serial.print(" dynamicMax=");
      Serial.println(dynamicMax, 1);
    }
    return;
  }

  // ── 4. Noise floor adaptativo — CONGELADO durante voz ───
  // Enquanto isActive=true, noise floor e dynamicMax ficam
  // congelados no valor aprendido durante o silêncio anterior.
  // Isso garante que notas sustentadas e fala contínua não
  // sejam engolidas pela adaptação do ruído de fundo.
  if (!isActive) {
    if (envelope > noiseFloor)
      noiseFloor += noiseAlphaUp   * (envelope - noiseFloor);
    else
      noiseFloor += noiseAlphaDown * (envelope - noiseFloor);

    if (dynamicMax < noiseFloor * 2) dynamicMax = noiseFloor * 2;
  }

  // ── 5. Sinal limpo + normalização ───────────────────────
  float signal = max(0.0f, envelope - noiseFloor);

  if (signal > dynamicMax) dynamicMax = signal;
  dynamicMax *= 0.9995;

  float norm = constrain(signal / dynamicMax, 0.0f, 1.0f);

  // ── 6. Detecção com hangover de silêncio ────────────────
  // Qualquer pico acima de thresholdOn renova lastVoiceMs —
  // o LED só apaga após SILENCE_HANGOVER ms sem nenhum pico
  if (norm > thresholdOn) {
    lastVoiceMs = millis();
    isActive    = true;
  }
  if (isActive && norm < thresholdOff && millis() - lastVoiceMs > SILENCE_HANGOVER) {
    isActive = false;
  }

  digitalWrite(LED_PIN, isActive);

  // ── 7. Debug ─────────────────────────────────────────────
  if (millis() - lastPrint > PRINT_MS) {
    Serial.print("AMP:"); Serial.print(amplitude, 1);
    Serial.print(" ENV:"); Serial.print(envelope,  1);
    Serial.print(" NSE:"); Serial.print(noiseFloor, 1);
    Serial.print(" SIG:"); Serial.print(signal,    1);
    Serial.print(" NRM:"); Serial.print(norm,      2);
    Serial.print(" "); printBar(norm);
    Serial.print(" "); Serial.println(isActive ? ">>> VOZ" : "    SILENCIO");
    lastPrint = millis();
  }
}