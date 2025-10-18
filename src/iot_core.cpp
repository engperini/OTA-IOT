#include "iot_core.h"
#include "config.h"
#include "wifi_manager.h"
#include "ui_server.h"

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#if USE_SPIFFS
  #include <SPIFFS.h>
  #define FS_CLASS SPIFFS
#else
  #include <LittleFS.h>
  #define FS_CLASS LittleFS
#endif

WebServer server(HTTP_PORT);
WebSocketsServer webSocket(WS_PORT);

static ws_handler_t g_ws_handler = nullptr;
static unsigned long lastCmdMs = 0;

void iot_register_ws_handler(ws_handler_t cb) { g_ws_handler = cb; }

void iot_broadcast(const String& msg) {
  String finalMsg = "[" + wifi_ip_as_string() + "] " + msg;
  Serial.println(finalMsg);
  webSocket.broadcastTXT(finalMsg);
}

void iot_reboot() { ESP.restart(); }

static void blinkConfirm() {
  for (int i=0;i<3;i++){ digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW?LOW:HIGH); delay(150);
                         digitalWrite(LED_BUILTIN, LED_ACTIVE_LOW?HIGH:LOW); delay(150); }
}

static void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t len) {
  if (type == WStype_TEXT) {
    unsigned long now = millis();
    if (now - lastCmdMs < WS_DEBOUNCE_MS) return;
    lastCmdMs = now;

    String msg((char*)payload, len);
    if (g_ws_handler) g_ws_handler(num, msg);
    else webSocket.sendTXT(num, "No handler registered");
  }
}

static void setupOtaRoutes() {
  // /update (POST) -> recebe binário OTA; /ota.html é servido pelo ui_server
  server.on("/update", HTTP_POST, [](){
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    blinkConfirm();
    ESP.restart();
  }, [](){
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_END) {
      if (!Update.end(true)) Update.printError(Serial);
    }
  });
}

void iot_init() {
  Serial.println("\n[IoT] Booting...");

  // FS
  if (!FS_CLASS.begin(true)) {
    Serial.println("❌ FS mount failed");
  } else {
    Serial.println("✅ FS mounted");
  }

  // Wi-Fi (STA ou AP fallback)
  bool sta = wifi_begin();
  Serial.println(sta ? "✅ WiFi: STA connected" : "⚠️ WiFi: AP fallback");
  Serial.print("IP: "); Serial.println(wifi_ip_as_string());

  // mDNS
  if (!MDNS.begin(MDNS_HOST)) Serial.println("⚠️ mDNS failed");
  else Serial.printf("mDNS: http://%s.local\n", MDNS_HOST);

  // HTTP rotas (UI + WiFi portal + OTA page)
  ui_init();
  setupOtaRoutes();
  server.begin();

  // WebSocket
  webSocket.begin();
  webSocket.onEvent(onWsEvent);

  // Mensagem de boas-vindas
  iot_broadcast("✅ Server started! IP: " + wifi_ip_as_string());
}

void iot_loop() {
  server.handleClient();
  webSocket.loop();
  yield();
}
