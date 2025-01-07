#include "WiFiTools.h"

bool connectWiFi(const char *ssid, const char *password)
{
  delay(2000);
  WiFi.begin(ssid, password);
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
    stations += WiFi.SSID(i) + " (" + String(100 + WiFi.RSSI(i)) + "%) ";
    stations += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "open" : "protected";
    stations += "</li>";
  }
  stations += "</ol>";
  return stations;
}