#include <Arduino.h>
#include <string.h>
#include "EepromTools.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "../eeprom_map.h"

#define TIMER0_DIV_VALUE 80000000L * 10 //Сlock frequency 80MHz, 1sec interrupt.

#define BLINK_INTERVAL_DEFAULT 300
#define BLINK_INTERVAL_WIFI_CONNECT 700

#define INTERVAL_READ_TEMPERATURE 1

//void eepromClear(int beginPos, int endPos);
//void writeStringEeprom(int beginPos, const String &data);


//Save wifi settings to eeprom. 
bool saveWifiSettings(const String &ssid, const String &password);

//Blink led and show system status.
void blinkSystemLed();
void status_ConnectWifi(); //Change blink interval
void status_NoConnectWifi(); //Change blink interval

void timer0_interrupt_handler(void);
void interruptsConfig();

void initTemperatureSensors();
int readTemperatureSensorsAdress();
String addressToString(DeviceAddress deviceAddress);
float getFirstSensorTemperature();
String getTemperatureHtmlList();

//Return block for show in page.
String getTemperatureHtml(); 

