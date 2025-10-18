#include "sensors.h"
#include "config.h"
#include "utils.h"
#include <WiFi.h>

// Funções exportadas por iot_core (para broadcast e reboot)
void iot_broadcast(const String& msg);
void iot_reboot();

static unsigned long lastSensorMs = 0;
static float lastTemp = 0;
static long  lastRSSI = 0;
static float lastUpMin = 0;
static int   lastLedState = -1;

static inline void ledWrite(bool on) {
#if LED_ACTIVE_LOW
  digitalWrite(LED_BUILTIN, on ? LOW : HIGH);
#else
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
#endif
}
static inline bool ledRead() {
#if LED_ACTIVE_LOW
  return digitalRead(LED_BUILTIN) == LOW;
#else
  return digitalRead(LED_BUILTIN) == HIGH;
#endif
}

static float chipTemp() { return temperatureRead(); }

void sensors_init() {
  pinMode(LED_BUILTIN, OUTPUT);
  ledWrite(false); // start OFF
}

void sensors_loop() {
  // periodic status (a cada SENSOR_PUSH_MS, só se variar)
  unsigned long now = millis();
  if (now - lastSensorMs >= SENSOR_PUSH_MS) {
    lastSensorMs = now;

    float t = chipTemp();
    long rssi = WiFi.isConnected() ? WiFi.RSSI() : 0;
    float upm = (millis()/1000.0)/60.0;

    if (fabs(t - lastTemp) > 0.5 || labs(rssi - lastRSSI) > 2 || fabs(upm - lastUpMin) >= 0.2) {
      lastTemp = t; lastRSSI = rssi; lastUpMin = upm;
      iot_broadcast("🌡 Temp: " + String(t,1) + "°C | 📶 RSSI: " + String(rssi) +
                    " dBm | ⏱ Uptime: " + formatUptimeHMS());
    }
  }

  // reporta mudança de LED
  int cur = ledRead() ? 1 : 0;
  if (cur != lastLedState) {
    lastLedState = cur;
    iot_broadcast(String("💡 LED changed: ") + (cur ? "ON" : "OFF"));
  }
}

// comandos vindos do WebSocket (UI)
void sensors_on_ws(uint8_t num, const String& msg) {
  if (msg == "reboot") {
    iot_broadcast("🔁 Command: REBOOT");
    delay(300);
    iot_reboot();
  } else if (msg == "led_on") {
    iot_broadcast("💡 Command: LED ON");
    ledWrite(true);
  } else if (msg == "led_off") {
    iot_broadcast("💡 Command: LED OFF");
    ledWrite(false);
  }
}
