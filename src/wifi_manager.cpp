#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>
#include <ESPmDNS.h>

#if USE_SPIFFS
  #include <SPIFFS.h>
  #define FS_CLASS SPIFFS
#else
  #include <LittleFS.h>
  #define FS_CLASS LittleFS
#endif

static Preferences prefs;
static String g_ssid, g_pass;

static bool loadCreds() {
  prefs.begin("WiFi", true);
  g_ssid = prefs.getString("ssid", "");
  g_pass = prefs.getString("pass", "");
  prefs.end();
  return g_ssid.length() > 0;
}

bool wifi_save_credentials(const String& ssid, const String& pass) {
  prefs.begin("WiFi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.end();
  return true;
}

void wifi_erase_credentials() {
  prefs.begin("WiFi", false);
  prefs.clear();
  prefs.end();
  WiFi.disconnect(true, true);
}

static void startAP() {
  WiFi.mode(WIFI_AP);
  IPAddress ip(AP_IP), gw(AP_GATEWAY), mask(AP_NETMASK);
  WiFi.softAPConfig(ip, gw, mask);
  WiFi.softAP(AP_SSID, AP_PASS);
}

bool wifi_is_connected() {
  return WiFi.isConnected() && WiFi.getMode() & WIFI_STA;
}

String wifi_ip_as_string() {
  if (wifi_is_connected()) return WiFi.localIP().toString();
  return WiFi.softAPIP().toString();
}

bool wifi_begin() {
  WiFi.mode(WIFI_STA);
  bool have = loadCreds();
  if (have) {
    WiFi.begin(g_ssid.c_str(), g_pass.c_str());
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - t0) < WIFI_CONNECT_TIMEOUT_MS) {
      delay(250);
    }
    if (WiFi.status() == WL_CONNECTED) {
      return true; // STA ok
    }
  }
  // STA falhou -> AP fallback
  startAP();
  return false;
}
