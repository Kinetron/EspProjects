#include "SystemTools.h"

#define TEMPERATURESENSOR_MAX_COUNT 3
#define ONE_WIRE_BUS 0 // Pin OneWire bus, 0 (D3)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&oneWire);

DeviceAddress temperatureSensorAdress[TEMPERATURESENSOR_MAX_COUNT];
uint8_t temperatureSensorsCount; //Quantity of sensors

int rebootDeviceFlag = 0; //Flag for begin reboot esp8266.

String temperatureHtml = "Wait temperature.."; 

bool systemLedStatus; //For blink led.

//If timer1 handle interrupt, this flag set true.
bool hasOneSecondTick = false;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sensor5_temperature");

FastBot bot(TG_BOT_TOKEN);

bool deviceFirstRun; //For send bot message on start.
/*
void eepromClear(int beginPos, int endPos)
{
  for (int i = beginPos; i < endPos; ++i)
  {
    EEPROM.write(i, 0);
	yield();
  }
}

void writeStringEeprom(int beginPos, const String &data)
{
    int pos = 0;
    for (int i = beginPos; i < beginPos + data.length(); ++i)
    {
      EEPROM.write(i, data[pos]);
	  yield();
      pos ++;
    }
}
*/

//Save wifi settings to eeprom. 
bool saveWifiSettings(const String &ssid, const String &password)
{
    //Clearing eeprom
    int endDataPos = EEPROM_INIT_WORD_LEN + EEPROM_CLIENT_SSID_LEN + EEPROM_CLIENT_PASSWORD_LEN;
    eepromClear(0, endDataPos);

    writeStringEeprom(0, INIT_WORD); //Set flag - device no first run.
    writeStringEeprom(EEPROM_INIT_WORD_LEN, ssid); //Writing wifi ssid.
    writeStringEeprom(EEPROM_INIT_WORD_LEN + EEPROM_CLIENT_SSID_LEN, password); //Writing wifi pass.
    
    if(!EEPROM.commit()){
        return false;
    }; 

    return true;
}

//Blink led and show system status.
void blinkSystemLed()
{ 
  if(systemLedStatus)
  {
    digitalWrite(LED_BUILTIN, LOW); 
    systemLedStatus = false;
  }
  else
  {
     digitalWrite(LED_BUILTIN, HIGH);
     systemLedStatus = true;
  }  
}

void interruptsConfig()
{
  noInterrupts();
  timer0_isr_init(); //Timer interrupts.
  timer0_attachInterrupt(timer0_interrupt_handler); 
  timer0_write(ESP.getCycleCount() + TIMER0_DIV_VALUE); //80mhz, one second
  interrupts();
}

void timer0_interrupt_handler(void)
{
  //pingHost();

  timer0_write(ESP.getCycleCount() + TIMER0_DIV_VALUE);
  hasOneSecondTick = true;
}

void initTemperatureSensors()
{
   temperatureSensors.begin();
   readTemperatureSensorsAdress();
}

int readTemperatureSensorsAdress()
{
  temperatureSensorsCount = temperatureSensors.getDeviceCount();
  if(temperatureSensorsCount > TEMPERATURESENSOR_MAX_COUNT)
  {
    temperatureSensorsCount = TEMPERATURESENSOR_MAX_COUNT;
  }

  for (uint8_t i = 0; i < temperatureSensorsCount; i++)
  {
    temperatureSensors.getAddress(temperatureSensorAdress[i], i);
  }

  return temperatureSensorsCount;
}

String addressToString(DeviceAddress deviceAddress)
{
  String str;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) 
    {
      str += "0";
    }

    str += String(deviceAddress[i], HEX);
  }

  return str;
}

//Read temperature and create html block with data.
String getTemperatureHtmlList()
{
  float temperature = 0;
  temperatureSensors.requestTemperatures();
  String html = "<ol>";
  for (int i = 0; i < temperatureSensorsCount; ++i)
  {
    String addr = addressToString(temperatureSensorAdress[i]);
    temperature = temperatureSensors.getTempC(temperatureSensorAdress[i]);
    html += "<li>";
    html += "Temperature: " + String(temperature) + "&deg;C " + addr;
    html += "</li>";

    if(i == 0)
    {
      Temperature.publish(temperature);
    }
  }

  html += "</ol>";

  temperatureHtml = html;
  return html;
}

//Return block for show in page.
String getTemperatureHtml()
{
  return temperatureHtml;
} 

void systemScheduler()
{
    if(!hasOneSecondTick)
    {
      return;
    }
    
    blinkSystemLed();
    
    if(rebootDeviceFlag > 0) //Reboot delay if has reboot command.
    {
      beginReboot();
    }

    if(!deviceFirstRun)
    {
      deviceFirstRun = true;
      sendRunHelloMsg();
    }

    getTemperatureHtmlList(); //Read temperature.
    MQTT_connect();
    hasOneSecondTick = false;
    
}

void MQTT_connect() 
{
  int8_t ret;
  if (mqtt.connected()) 
  {
    return;
  }

  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected

       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
  }
 
}

//Wait and reboot device.
void rebootDevice()
{
  //Init reboot process
   if(rebootDeviceFlag == 0)
   {
     rebootDeviceFlag = 1;
   }
}

void beginReboot()
{
    if(rebootDeviceFlag < DELAY_REBOOT_INTERVAL)
    {
      rebootDeviceFlag ++;
    }
    else
    {
      ESP.restart();
    }
}

//Init telegram bot.
void initTgBot()
{
   bot.attach(tgBotMsgHandler);
}

//Message handler for bot.
void tgBotMsgHandler(FB_msg& msg) {

 //Update firmware.


   executeBotCommand(msg);
}

//If device run -send messege to user.
void sendRunHelloMsg()
{
  bot.sendMessage("The device is loaded. FW Version " + String(FIRMWARE_VERSION), EXT_USER_ID);
}

void botTick()
{
  bot.tick();
}

//Handler user commands.
void executeBotCommand(FB_msg msg)
{

  //Update firmware.
  if (msg.OTA && msg.text == FIRMWARE_UPDATE_PASSWORD) 
  {
    int status = bot.update();
    if(status != 1)  bot.sendMessage(String(status), msg.chatID);     
  }

  String userCommand = msg.text;
  String responseMsg = "";
  if (userCommand == "/help")
	{
    String help = botHelp;
    responseMsg = help;
	}
  else if (userCommand == "/ver")
	{
	  responseMsg = "FW Ver = " + String(FIRMWARE_VERSION);
	}
  else if (userCommand == "/data")
	{
	  responseMsg = temperatureHtml;
	}
  else if (userCommand == "/reboot")
  {
    responseMsg = "Wait untill device reboot.";
    rebootDevice();
  }

  bot.sendMessage(responseMsg, EXT_USER_ID);
}