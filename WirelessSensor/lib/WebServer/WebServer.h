#include <Arduino.h>
#include <string.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

#include "WiFiTools.h"
#include  "SystemTools.h"
#include "../eeprom_map.h"
//#include "../SystemSettings/SystemSettings.h"


//Init ap, web server and wait client settings.
void initWebServer(bool pageType);

//Return page with wifi settings.
void createWebServerWithDefaultPage();

//Create server with mane page.
void createWebServer();

void runWebServer();
void handleClient();