#include <Arduino.h>
#include "config.h"
#include "iot_core.h"
#include "sensors.h"

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW ? HIGH : LOW); // OFF
  iot_init();
  sensors_init();

  // registra handler WS dos sensores (comandos LED/reboot)
  iot_register_ws_handler(sensors_on_ws);
}

void loop() {
  iot_loop();
  sensors_loop();
}
