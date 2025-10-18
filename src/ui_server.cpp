#include "ui_server.h"
#include "config.h"
#include "wifi_manager.h"
#include "sensors.h"

#include <WebServer.h>
#if USE_SPIFFS
  #include <SPIFFS.h>
  #define FS_CLASS SPIFFS
#else
  #include <LittleFS.h>
  #define FS_CLASS LittleFS
#endif

// Estes objetos existem no iot_core.cpp
extern WebServer server;

// Rotas:
//   "/"          -> /index.html (UI principal)
//   "/ota"       -> /ota.html   (tela de serviço)
//   "/wifi"      -> /wifi.html  (config Wi-Fi)
//   "/wifi/save" -> POST credenciais

void ui_init() {
  // Arquivos estáticos
  server.serveStatic("/",      FS_CLASS, "/index.html");
  server.serveStatic("/ota",   FS_CLASS, "/ota.html");
  server.serveStatic("/wifi",  FS_CLASS, "/wifi.html");

  // Salvar credenciais Wi-Fi
  server.on("/wifi/save", HTTP_POST, [](){
    if (!server.hasArg("ssid") || !server.hasArg("pass")) {
      server.send(400, "text/plain", "Missing ssid/pass");
      return;
    }
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    wifi_save_credentials(ssid, pass);
    server.send(200, "text/plain", "OK, rebooting...");
    delay(300);
    ESP.restart();
  });


}
