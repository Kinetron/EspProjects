#include <Arduino.h>
#include <string.h>
#include "EepromTools.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "../eeprom_map.h"
#include "../accounts.h"
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <FastBot.h>
#include "../version.h"
#include "../botHelp.h"

#define TIMER0_DIV_VALUE 80000000L //Сlock frequency 80MHz, 1sec interrupt. 80Mhz -> 80*10^6 = 1 second

#define DELAY_REBOOT_INTERVAL 3 //Wait interval after recive reboot command. Second.


//void eepromClear(int beginPos, int endPos);
//void writeStringEeprom(int beginPos, const String &data);


//Save wifi settings to eeprom. 
bool saveWifiSettings(const String &ssid, const String &password);

//Blink led and show system status.
void blinkSystemLed();

void timer0_interrupt_handler(void);
void interruptsConfig();

void initTemperatureSensors();
int readTemperatureSensorsAdress();
String addressToString(DeviceAddress deviceAddress);

//Read temperature and create html block with data.
String getTemperatureHtmlList();

//Return block for show in page.
String getTemperatureHtml(); 

void systemScheduler();
void MQTT_connect();

//Wait and reboot device.
void rebootDevice();
void beginReboot();

//Init telegram bot.
void initTgBot();
//Message handler for bot.
void tgBotMsgHandler(FB_msg& msg);
//If device run -send messege to user.
void sendRunHelloMsg();
void botTick();
//Handler user commands.
void executeBotCommand(FB_msg msg);

