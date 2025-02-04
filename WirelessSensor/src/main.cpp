/*
 Default ip 192.168.4.1
*/
#include <Arduino.h>
#include <WiFiTools.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <EepromTools.h>
#include "../lib/eeprom_map.h"

bool firstRunCheck();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  EEPROM.begin(512); //Initialasing Flash block as EEPROM
  interruptsConfig(); 
  initTemperatureSensors();

  //Device first run?
  if(firstRunCheck())
  {
    initAp();
    initWebServer(true);

    while ((WiFi.status() != WL_CONNECTED))
    {
       delay(200);
       handleClient();
    }  

    return;
  }
  
  if(!connectWiFi())
  {
    /*
    if(waitPressFwButton()) //Check if enabled set wifi settings mode.
    {      
      blinkBlueLed();
      blinkBlueLed();
      blinkBlueLed();
      initWebServer();
      return;
    } 
    */   
    
    ESP.restart();
  } 
   
   status_ConnectWifi();
   //ESP.wdtEnable(15000);

   initWebServer(false); //Init with main page.
}

void loop() { 
  handleClient(); //Processing incoming requests
  blinkSystemLed(); 
 
  //Check wifi autoreconnect.
   getTemperatureHtmlList();

}

//Check if device first run.
bool firstRunCheck()
{
  String initWord = readStringEeprom(0, EEPROM_INIT_WORD_LEN);
  if(initWord == INIT_WORD) return false;

  return true;
}
