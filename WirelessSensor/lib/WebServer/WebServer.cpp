#include "WebServer.h"
#include "webPages/default.h"
#include "webPages/mainPage.h"

const char responsePage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
	<title>Device settings</title>
</head>
<body>
<a href="/">To main</a><br>
@response
</body>
</html>
)=====";

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* host = "esp8266-webupdate";

String webServerContent; //web server content
int webStatusCode; //Web server last status code.
extern String wifi_stations; //Available access points, from scan.
bool UPDATE;

//Return page whith wifi settings.
void createWebServerWithDefaultPage()
{
	server.on("/", []() {
	 String page = defaultPage;
	 page.replace("@stations", wifi_stations);
     server.send(200, "text/html", page);
    });

	server.on("/scanWifi", []() {
	 String page = defaultPage;
	 wifi_stations = scanNetworks();
	 page.replace("@stations", wifi_stations);
     server.send(200, "text/html", page);
    });	
	
    server.on("/wifiSet", []() {
      String ssid = server.arg("ssid");
      String pass = server.arg("pass");
	  String result = "";
      	  
      if(ssid.length() == 0 || pass.length() == 0 || ssid.length() > EEPROM_CLIENT_SSID_LEN 
      || pass.length() >  EEPROM_CLIENT_PASSWORD_LEN) 
      {
		  result = "{\"Error\":\"Empty ssid or password. Or exceeded max string length.\"}";
      }
      else
      {
        if(!saveWifiSettings(ssid, pass))
		{
           result =  "{\"Error\":\"Error save to flash.\"}";
        }
        else
        {
          result = "{\"Success\":\"Saved to eeprom... Begin reset to boot into new wifi. Wait 10sec.\"}";
          rebootDevice();
        }
      }

      String page = responsePage;
	    page.replace("@response", result);
      server.send(200, "text/html", page); 
    });
}

//Main page on normal mode.
void createWebServer()
{
    server.on("/", []() {
	  String page = mainPage;

	  page.replace("@temperature", getTemperatureHtml());	  
      server.send(200, "text/html", page);
    });    

   server.on("/reboot", HTTP_POST,[]() {
      rebootDevice();
      String page = responsePage;
	    page.replace("@response", "Begin reboot...");
       server.send(200, "text/html", page);
   });

   //server.on("/update", [](){webUpdate();});
}

void runWebServer()
{  
  MDNS.begin(host);
  httpUpdater.setup(&server);
  server.begin();
  MDNS.addService("http", "tcp", 80); 
}

void handleClient()
{
  server.handleClient();
}


//Init ap, web server and wait client settings.
void initWebServer(bool pageType)
{
	if(pageType) //Default init page
	{
		createWebServerWithDefaultPage();
	}
	else
	{
		createWebServer(); // Start the server
	}

  runWebServer();
}

//For OTA update firmware.
void mdnsUpdate()
{
  MDNS.update();
}

/*
void otaStart(const char* linkOTA)
{
  WiFiClientSecure otaWiFi;
  // Запускаем обновление
  t_httpUpdate_return ret = ESPhttpUpdate.update(otaWiFi, linkOTA);
  // Анализируем результат
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("OTA :: Update failed");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("OTA :: Update no Updates");
      break;
    case HTTP_UPDATE_OK:
      // А вот это сообщение не факт, что вы увидите, потому что esp будет перезагружена
      Serial.println("OTA :: Update OK");
      break;
}
      */

void webUpdate()
{
  if(!UPDATE)
  {
    httpUpdater.setup(&server);
     Serial.print("WiFi.localIP:  ");
     Serial.println(WiFi.localIP());
     Serial.println("HTTP UpdateServer started");
      yield();
    UPDATE = true;
  }
}