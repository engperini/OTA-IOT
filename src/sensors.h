#pragma once
#include <Arduino.h>

void sensors_init();
void sensors_loop();

// handler para mensagens WebSocket vindas da UI
void sensors_on_ws(uint8_t clientNum, const String& msg);
