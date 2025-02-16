#include <Arduino.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include "EepromTools.h"

#define WIFI_СONNECTION_ATTEMPTS 30 //Max attempts count connect to WiFi.
#define DEFAULT_SSID "SettingsWiFi"
#define DEFAULT_PASSWORD "88888888"

#define DELAY_WIFI_AFTER_RUN 2000
#define DELAY_WIFI_CONNECT_INTERVAL 500

//While connect fast blink led.
void blinkBlueLed();
bool connectWiFi();
//Enable AP.
void initAp();
String scanNetworks();
