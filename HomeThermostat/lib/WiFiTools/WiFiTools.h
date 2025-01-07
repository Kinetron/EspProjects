#include <Arduino.h>
#include <string.h>
#include <ESP8266WiFi.h>

#define WIFI_СONNECTION_ATTEMPTS 30 //Max attempts count connect to WiFi.
#define DEFAULT_SSID "home_thermostat"
#define DEFAULT_PASSWORD "termo2025"

bool connectWiFi(const char *ssid, const char *password);
//Enable AP.
void initAp();
String scanNetworks();