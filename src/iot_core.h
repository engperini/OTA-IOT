#pragma once
#include <Arduino.h>

void iot_init();   // Wi-Fi, FS, mDNS, HTTP, WS, OTA
void iot_loop();   // server.handleClient(), webSocket.loop()

// Broadcast p/ todos os clientes e reboot (exportados p/ sensores)
void iot_broadcast(const String& msg);
void iot_reboot();

// Registrar handler de mensagens WS vindo de outros módulos
typedef void (*ws_handler_t)(uint8_t clientNum, const String& msg);
void iot_register_ws_handler(ws_handler_t cb);
