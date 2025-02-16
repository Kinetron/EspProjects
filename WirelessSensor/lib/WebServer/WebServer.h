#include <Arduino.h>
#include <string.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>

#include "WiFiTools.h"
#include  "SystemTools.h"
#include "../eeprom_map.h"
//#include "../SystemSettings/SystemSettings.h"


//Init ap, web server and wait client settings.
void initWebServer(bool pageType);

//Return page with wifi settings.
void createWebServerWithDefaultPage();

void webUpdate();
//Create server with mane page.
void createWebServer();

void runWebServer();
void handleClient();
//For OTA update firmware.
void mdnsUpdate();
void otaStart(const char* linkOTA);
