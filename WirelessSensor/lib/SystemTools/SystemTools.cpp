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
Adafruit_MQTT_Publish temperatureMqtt = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sensor5_temperature");
Adafruit_MQTT_Publish gazMqtt = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gazValue");

float lastTemperature = 0; 
int sendParamTimer = 0;

FastBot bot(TG_BOT_TOKEN);

bool deviceFirstRun; //For send bot message on start.

unsigned long  lastBlinkTime = 0; //For blink blue led.
unsigned long  ledBlinkPeriod = 0;  

const int gazDetectorPin = A0;  // ESP8266 Analog Pin ADC0 = A0 for gaz detector MQ 7.
int gazDetectorAdcValue;

//System settings.
struct FlashSettingsStruct_t systemSettings;
//Admin page data.
struct AdminPageStruct_t adminPageSettings;

String tgBotToken = "";

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

static uint16_t reset = 0;
uint16_t onTime = 1000, cycleTime = 1000;

uint16_t littleMillis()
{
  return millis() & 65535;
}

//Blink led and show system status.
void blinkSystemLed()
{     
  unsigned long currentTime = millis();
 
  if (currentTime - lastBlinkTime < ledBlinkPeriod)
  {
    return;
  }
  
  lastBlinkTime = currentTime;
 
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

  /*
  timer1_attachInterrupt(timer1_interrupt_handler);
  timer1_write(31250); // 80MHz / 256 (TIM_DIV256) / 312500 => 1s
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  */
  interrupts();
}

void timer1_interrupt_handler(void)
{  
}

void timer0_interrupt_handler(void)
{
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
      lastTemperature = temperature;
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
    blinkSystemLed();

    if(!hasOneSecondTick)
    {
      return;
    }
    hasOneSecondTick = false;
       
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
    publichData();    
    //pingHost();

    readAdc();
}

void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    ledBlinkNormalMode();
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {  // connect will return 0 for connected
      
      ledBlinkModeFindWifi(); //Fast blink - connected problem.
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("Retrying MQTT connection in 5 seconds...");
      mqtt.disconnect();
      delay(5000);  // wait 5 seconds
      
      retries--;
      
      if (retries == 0) {
        Serial.println("MQTT can't connected!");
         return;
       }
  }
  Serial.println("MQTT Connected!"); 
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
  bot.sendMessage("The device is loaded. FW Version " + String(FIRMWARE_VERSION) + ". Local web access: http://" + WiFi.localIP().toString() +"/", EXT_USER_ID);
}

void botTick()
{
  bot.tick();
}

//Handler user commands.
void executeBotCommand(FB_msg msg)
{

  /*
  //Update firmware.
  if (msg.OTA && msg.text == FIRMWARE_UPDATE_PASSWORD) 
  {
    int status = bot.update();
    if(status != 1)  bot.sendMessage(String(status), msg.chatID);     
  }
*/
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

void publichData()
{
  if(sendParamTimer < INTERVAL_SEND_PARAMS)
  {
    sendParamTimer ++;
  }
  else
  {
    sendParamTimer = 0;
    temperatureMqtt.publish(lastTemperature);
    gazMqtt.publish(gazDetectorAdcValue);
  }  
}

void initPingWatchDog()
{
  initPingWatchdog();
}

//Set blink period find wifi.
void ledBlinkModeFindWifi()
{
  ledBlinkPeriod = LED_CONNECT_BLINK_INTERVAL;
}

//Set normal blink mode.
void ledBlinkNormalMode()
{
  ledBlinkPeriod = LED_NORMAL_BLINK_INTERVAL;
}

//Read analog value from A0 pin.
void readAdc()
{
  gazDetectorAdcValue = analogRead(gazDetectorPin);
}

//Create string for html.
String getGazDetectorValue()
{
  return String(gazDetectorAdcValue);
}

String getReceiveId(int number)
{
  switch(number)
  {
    case 0:  return String(systemSettings.receiveId0);
    case 1:  return String(systemSettings.receiveId1);
    default: return String(systemSettings.receiveId0);
  }  
}

void setReceiveId(String userId, int number)
{ 
  switch(number)
  {
    case 0: 
      userId.toCharArray(systemSettings.receiveId0, userId.length()+1);
      break;
    case 1: 
     userId.toCharArray(systemSettings.receiveId1, userId.length()+1);
      break;
    default: 
      userId.toCharArray(systemSettings.receiveId0, userId.length()+1);
  }  
}

String getTgBotToken()
{
  return String(systemSettings.botToken);
}

void setTgBotToken(String token)
{
  token.toCharArray(systemSettings.botToken, token.length() + 1);
}

//Store system settings to flash.
void writeSettingsToFlash()
{
  int addr = EEPROM_INIT_WORD_LEN + EEPROM_CLIENT_SSID_LEN + EEPROM_CLIENT_PASSWORD_LEN;

  EEPROM.begin(4095);
  EEPROM.put(addr, systemSettings);
  delay(200);
  EEPROM.commit();                      // Only needed for ESP8266 to get data written
  EEPROM.end();                         // Free RAM copy of structure
}

//Read system settings from flash.
void readSettingsFromFlash()
{
  int addr = EEPROM_INIT_WORD_LEN + EEPROM_CLIENT_SSID_LEN + EEPROM_CLIENT_PASSWORD_LEN;
  EEPROM.begin(4095);
  EEPROM.get(addr, systemSettings);
  EEPROM.end();
}