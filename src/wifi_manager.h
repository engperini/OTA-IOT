#pragma once
#include <Arduino.h>

bool wifi_begin();                 // tenta STA; se falhar, inicia AP
bool wifi_is_connected();          // estado STA
String wifi_ip_as_string();        // IP atual (STA ou AP)
bool wifi_save_credentials(const String& ssid, const String& pass);
void wifi_erase_credentials();     // opcional (não usado automaticamente)
