#pragma once

// ---------- Hardware ----------
#ifndef LED_BUILTIN
#define LED_BUILTIN 21   // XIAO ESP32S3: GPIO21 (LED onboard) - ajuste se precisar
#endif
#define LED_ACTIVE_LOW 1 // 1 = LED acende em LOW

// ---------- Rede ----------
#define MDNS_HOST               "esp32"
#define WS_PORT                 81
#define HTTP_PORT               80

// Tempo p/ tentativa STA antes de cair em AP (ms)
#define WIFI_CONNECT_TIMEOUT_MS 10000

// AP fallback
#define AP_SSID   "ESP32-Setup"
#define AP_PASS   ""        // vazio = aberto; defina se quiser senha
#define AP_IP      192,168,4,1
#define AP_GATEWAY 192,168,4,1
#define AP_NETMASK 255,255,255,0

// ---------- Relatórios ----------
#define SENSOR_PUSH_MS          10000   // status a cada 10s (se variar)
#define WS_DEBOUNCE_MS          300

// ---------- SPIFFS/LittleFS ----------
#define USE_SPIFFS 1          // 1 = SPIFFS, 0 = LittleFS
