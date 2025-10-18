#pragma once
#include <Arduino.h>

void sensors_init();
void sensors_loop();

// handler para mensagens WebSocket vindas da UI
void sensors_on_ws(uint8_t clientNum, const String& msg);

// em src/sensors.h (coloque junto das outras declarações de sensors)
#ifdef USE_BMP280
void bmp280_init();
bool bmp280_ok();
float bmp280_get_temperature();   // em °C
float bmp280_get_pressure_hpa();  // em hPa
float bmp280_get_altitude_m(float seaLevel_hPa = 1013.25); // opcional
#endif
