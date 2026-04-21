#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// ═══════════════════════════════════════════════════════════
//  REDE — WiFi + UDP para envio de estado ao Unity
// ═══════════════════════════════════════════════════════════
const char* WIFI_SSID     = "Victor - 2.4G-EXT";
const char* WIFI_PASSWORD = "07110589";
const char* PC_IP         = "192.168.15.65";
const int   UDP_PORT      = 9000;

WiFiUDP udp;

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
//  CONGELADO enquanto voz estiver ativa — impede que o
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
//  DEBUG + UDP
// ═══════════════════════════════════════════════════════════
unsigned long lastPrint = 0;
const int     PRINT_MS  = 100;  // 10x por segundo

// ── Barra visual sem alocação dinâmica ──────────────────────
void printBar(float v, int width = 20) {
  int filled = constrain((int)(v * width), 0, width);
  Serial.print("[");
  for (int i = 0; i < width; i++) Serial.print(i < filled ? "#" : "-");
  Serial.print("]");
}

// ── Conecta WiFi ────────────────────────────────────────────
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[WiFi] Conectando");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;

    if (attempts > 30) {
      Serial.println("\n[WiFi] Timeout — reiniciando tentativa...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      attempts = 0;
    }
  }

  Serial.println("\n[WiFi] ✅ CONECTADO");
  Serial.print("[WiFi] IP ESP32: ");
  Serial.println(WiFi.localIP());

  udp.begin(UDP_PORT);

  Serial.print("[UDP] DESTINO: ");
  Serial.print(PC_IP);
  Serial.print(":");
  Serial.println(UDP_PORT);
}

// ── Envia estado via UDP para o Unity ───────────────────────
// JSON: {"state":"VOZ","norm":0.85,"amplitude":320.0,"noiseFloor":35.0}
void sendUDP(float norm, float amplitude, float nFloor, bool active) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[UDP] ⚠ WiFi OFF — pacote não enviado");
    return;
  }

  StaticJsonDocument<128> doc;
  doc["state"]      = active ? "VOZ" : "SILENCIO";
  doc["norm"]       = norm;
  doc["amplitude"]  = amplitude;
  doc["noiseFloor"] = nFloor;

  char buffer[128];
  serializeJson(doc, buffer);

  udp.beginPacket(PC_IP, UDP_PORT);
  udp.write((uint8_t*)buffer, strlen(buffer));

  bool success = udp.endPacket();

  if (!success) {
    Serial.println("[UDP] ❌ ERRO ao enviar pacote");
  } else {
    Serial.print("[UDP] ✔ enviado -> ");
    Serial.println(buffer);
  }
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

  // ── WiFi + UDP ───────────────────────────────────────────
  setupWiFi();

  // ── Configuração I2S ─────────────────────────────────────
  // INMP441: 32 bits por amostra, canal RIGHT (L/R = 3.3V)
  // sample_rate 16kHz suficiente para detecção de voz
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

  // 🔥 AUTO-RECONNECT WiFi
if (WiFi.status() != WL_CONNECTED) {
  static unsigned long lastReconnect = 0;

  if (millis() - lastReconnect > 3000) {
    Serial.println("[WiFi] Reconectando...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastReconnect = millis();
  }
}

  size_t bytesIn = 0;
  i2s_read(I2S_PORT, (void*)sBuffer, sizeof(sBuffer), &bytesIn, portMAX_DELAY);
  if (bytesIn == 0) return;

  // ── 1. Amplitude média absoluta (MAE) ───────────────────
  // shift >>16: descarta os 16 LSBs de ruído do INMP441
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
  if (norm > thresholdOn) {
    lastVoiceMs = millis();
    isActive    = true;
  }
  if (isActive && norm < thresholdOff && millis() - lastVoiceMs > SILENCE_HANGOVER) {
    isActive = false;
  }

  digitalWrite(LED_PIN, isActive);

  // ── 7. Debug + UDP ───────────────────────────────────────
  if (millis() - lastPrint > PRINT_MS) {
    Serial.print("AMP:"); Serial.print(amplitude, 1);
    Serial.print(" ENV:"); Serial.print(envelope,  1);
    Serial.print(" NSE:"); Serial.print(noiseFloor, 1);
    Serial.print(" SIG:"); Serial.print(signal,    1);
    Serial.print(" NRM:"); Serial.print(norm,      2);
    Serial.print(" "); printBar(norm);
    Serial.print(" "); Serial.println(isActive ? ">>> VOZ" : "    SILENCIO");

    // Envia estado para Unity via UDP
    sendUDP(norm, amplitude, noiseFloor, isActive);

    lastPrint = millis();
  }
}