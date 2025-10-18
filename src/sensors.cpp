#include "sensors.h"
#include "config.h"
#include "utils.h"
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP280.h> // Adafruit BMP280

// Funções exportadas por iot_core (para broadcast e reboot)
void iot_broadcast(const String& msg);
void iot_reboot();

static unsigned long lastSensorMs = 0;
static float lastTemp = 0;
static long  lastRSSI = 0;
static float lastUpMin = 0;
static int   lastLedState = -1;

#ifdef USE_BMP280
  static Adafruit_BMP280 bmp;
  static bool bmp_present = false;
  static float bmp_temp = 0.0;
  static float bmp_pressure_hpa = 0.0;
#endif

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


  

#ifdef USE_BMP280
  Wire.begin(); // inicia I2C com pinos default; use Wire.begin(SDA, SCL) se necessário
  // tenta 0x76 e 0x77
  if (bmp.begin(0x76)) {
    bmp_present = true;
    Serial.println("BMP280: encontrado em 0x76");
  } else if (bmp.begin(0x77)) {
    bmp_present = true;
    Serial.println("BMP280: encontrado em 0x77");
  } else {
    bmp_present = false;
    Serial.println("BMP280: nao encontrado");
  }
#endif
}



void sensors_loop() {
  // periodic status (a cada SENSOR_PUSH_MS, só se variar)
  unsigned long now = millis();
  if (now - lastSensorMs >= SENSOR_PUSH_MS) {
    lastSensorMs = now;

    float t = chipTemp();
    long rssi = WiFi.isConnected() ? WiFi.RSSI() : 0;
    float upm = (millis()/1000.0)/60.0;




    #ifdef USE_BMP280
      if (bmp_present) {
        // readTemperature retorna °C, readPressure retorna Pa
        bmp_temp = bmp.readTemperature();
        bmp_pressure_hpa = bmp.readPressure() / 100.0F;
        // opcional: iot_broadcast com pressão
        iot_broadcast(String("BMP280: ") + String(bmp_temp,1) + "C " + String(bmp_pressure_hpa,2) + "hPa");
      }
    #endif

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


