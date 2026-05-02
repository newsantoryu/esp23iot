#pragma once
#include <WiFi.h>

class DebugDashboard {
public:
  static void update(
    float pitch,
    String note,
    float cents,
    float confidence,
    bool isActive,
    bool noteStable
  ) {
    static unsigned long last = 0;
    if (millis() - last < 1000) return;
    last = millis();

    Serial.println();
    Serial.println("══════════ ESP32 SYSTEM DASHBOARD ══════════");

    // ───────── WIFI ─────────
    wl_status_t status = WiFi.status();
    Serial.print("WiFi: ");

    switch (status) {
      case WL_CONNECTED:
        Serial.print("CONNECTED");
        Serial.print(" (");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm)");
        break;
      case WL_DISCONNECTED:
        Serial.println("DISCONNECTED");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("FAILED");
        break;
      default:
        Serial.println("CONNECTING...");
        break;
    }

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // ───────── AUDIO ─────────
    Serial.println();
    Serial.print("Audio: ");
    Serial.println(isActive ? "ACTIVE" : "IDLE");

    Serial.print("Pitch: ");
    Serial.println(pitch > 0 ? String((int)pitch) + " Hz" : "---");

    Serial.print("Note: ");
    Serial.println(note);

    Serial.print("Cents: ");
    Serial.println((int)cents);

    Serial.print("Confidence: ");
    Serial.println(confidence, 2);

    Serial.print("Stable: ");
    Serial.println(noteStable ? "YES" : "NO");

    // ───────── SYSTEM ─────────
    Serial.println();
    Serial.print("Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" s");

    Serial.println("════════════════════════════════════════════");
  }
};