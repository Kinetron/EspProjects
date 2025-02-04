#include <Arduino.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include "EepromTools.h"

#define WIFI_СONNECTION_ATTEMPTS 30 //Max attempts count connect to WiFi.
#define DEFAULT_SSID "SettingsWiFi"
#define DEFAULT_PASSWORD "88888888"

bool connectWiFi();
//Enable AP.
void initAp();
String scanNetworks();