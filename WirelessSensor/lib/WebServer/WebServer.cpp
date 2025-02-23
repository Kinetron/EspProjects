#include "WebServer.h"
#include "webPages/default.h"
#include "webPages/mainPage.h"
#include "webPages/adminPage.h"

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
//ESP8266HTTPUpdateServer httpUpdater;
const char* host = "esp8266-webupdate";

String webServerContent; //web server content
int webStatusCode; //Web server last status code.
extern String wifi_stations; //Available access points, from scan.
bool UPDATE;
String errorText; //Text with error for answer user.

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
      page.replace("@gazValue", "Gaz value: " + getGazDetectorValue());	 

      server.send(200, "text/html", page);
    });    

   server.on("/reboot", HTTP_POST,[]() {
      rebootDevice();
      String page = responsePage;
	    page.replace("@response", "Begin reboot...");
       server.send(200, "text/html", page);
   });

   server.on("/admin", []() {
    String page = adminPage;
    readSettingsFromFlash();
    page.replace("@botToken",  getTgBotToken());
    page.replace("@receiveId0", getReceiveId(0));
    page.replace("@receiveId1", getReceiveId(1));
    
    server.send(200, "text/html", page);
   });

   server.on("/saveSettings", HTTP_POST,[]() {
    postSaveSettings();
   });
}

void runWebServer()
{  
  //MDNS.begin(host);
  //httpUpdater.setup(&server);
  server.begin();
  //MDNS.addService("http", "tcp", 80); 
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
 // MDNS.update();
}

void postSaveSettings()
{
  String botToken = server.arg("botToken");
  String receiveId0 = server.arg("receiveId0"); //First user id can't empty.
  String receiveId1 = server.arg("receiveId1"); //First user id can't empty.

  String answer = adminPageAnswer;

  //Check user input.
  if(!validateBotSettings(botToken, receiveId0))
  {
    answer.replace("@infoText", "Error!");
    answer.replace("@errorText", errorText);

    server.send(200, "text/html", answer);
    return;  
  }

  setTgBotToken(botToken);
  setReceiveId(receiveId0, 0);
  setReceiveId(receiveId1, 1);
  writeSettingsToFlash();
  /*
   if(!saveTgBotToken(botToken))
   {
    answer.replace("@infoText", "Error!");
    answer.replace("@errorText", "Can't save data to flash!");
    server.send(200, "text/html", answer);
    return; 
   }
*/

   answer.replace("@infoText", "Successfully!");
   answer.replace("@errorText", "");
   server.send(200, "text/html", answer);
}

//Check user input.
bool validateBotSettings(String botToken, String userId)
{
  if(botToken.length() == 0)
  {
    errorText = "Bot token is empty.";
    return false;
  }
  
  if(userId.length() == 0)
  {
    errorText = "UserId id is empty.";
    return false;
  }  

  if(botToken.length() > EEPROM_TELEGRAM_BOT_TOKEN_LEN)
  {
    errorText = "Bot token too long.";
    return false;
  }
  
  if(userId.length() > EEPROM_TELEGRAM_CLIENT_ID_LEN)
  {
    errorText = "UserId id too long .";
    return false;
  } 

  return true;
}