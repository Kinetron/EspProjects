#include "WiFiTools.h"
String wifi_stations; //Available access points, from scan.

bool connectWiFi()
{
  //Device connected.
  if(WiFi.status() == WL_CONNECTED)
  {
    return true;
  }

  String ssid = readStringEeprom(EEPROM_INIT_WORD_LEN, EEPROM_CLIENT_SSID_LEN); 
  String password = readStringEeprom(EEPROM_INIT_WORD_LEN + EEPROM_CLIENT_SSID_LEN, EEPROM_CLIENT_SSID_LEN + EEPROM_CLIENT_PASSWORD_LEN);

  delay(2000);
  WiFi.begin(ssid.c_str(), password.c_str());
  int attempts = 0; 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    if(attempts > WIFI_СONNECTION_ATTEMPTS)
    {
      return false;
    }
    attempts ++;
  }
  return true;
}

//Enable AP.
void initAp()
{
  WiFi.mode(WIFI_STA);
  delay(500);  
  wifi_stations = scanNetworks(); //Get list available networks.
  WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD);
}

String scanNetworks()
{
  delay(100);

  int networks = WiFi.scanNetworks();
 
  if (networks == 0)
  {
     return "No networks found";
  }

  String stations = "<ol>";
  for (int i = 0; i < networks; ++i)
  {
    // Print SSID and RSSI for each network found
    stations += "<li>";
    stations += WiFi.SSID(i) + " (ch:"+ String(WiFi.channel(i))+ " " + String(100 + WiFi.RSSI(i)) + "%) ";
    stations += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "open" : "protected";
    stations += "</li>";
  }
  stations += "</ol>";
  return stations;
}