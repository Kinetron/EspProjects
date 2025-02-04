#include "SystemTools.h"

#define TEMPERATURESENSOR_MAX_COUNT 3
#define ONE_WIRE_BUS 0 // Pin OneWire bus, 0 (D3)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensors(&oneWire);

DeviceAddress temperatureSensorAdress[TEMPERATURESENSOR_MAX_COUNT];

int blinkLedInterval = BLINK_INTERVAL_DEFAULT;
int resetDeviceFlag = 0; //Flag for begin reboot esp8266.

String temperatureHtml = "Wait temperature.."; 
int counterReadTemperature = 1;

bool systemLedStatus;

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
   delay(1000);
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

//Change blink interval
void status_ConnectWifi()
{
  blinkLedInterval = BLINK_INTERVAL_WIFI_CONNECT;
}

void status_NoConnectWifi()
{
  blinkLedInterval = BLINK_INTERVAL_DEFAULT;
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

   //Process module reset.
   switch(resetDeviceFlag) //Flag for begin reboot esp8266.)
   {
    case 1: resetDeviceFlag = 2; //Wait 2sec for some time to send answer in telegram.
    break;
    case 2:
      ESP.restart();
    break;
    default: break;
   } 

  timer0_write(ESP.getCycleCount() + TIMER0_DIV_VALUE); //Тактовая частота 80MHz, получаем секунду 

  //Read temperature.
  if(counterReadTemperature < INTERVAL_READ_TEMPERATURE )
  {
    counterReadTemperature ++;
  }
  else
  {
    counterReadTemperature = 1;
   
  }
}

void initTemperatureSensors()
{
   temperatureSensors.begin();
   readTemperatureSensorsAdress();
}

float getFirstSensorTemperature()
{
  return temperatureSensors.getTempC(temperatureSensorAdress[0]);
}

int readTemperatureSensorsAdress()
{
  uint8_t temperatureSensorsCount = temperatureSensors.getDeviceCount();
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

String getTemperatureHtmlList()
{
  /*
  int count = readTemperatureSensorsAdress();
  if(count == 0)
  {
    return "Any temperature sensor not found";
  }
*/
int count = 1;
 
  float temperature = 0;
  temperatureSensors.requestTemperatures();
  String html = "<ol>";
  for (int i = 0; i < count; ++i)
  {
    String addr = addressToString(temperatureSensorAdress[i]);
    temperature = temperatureSensors.getTempC(temperatureSensorAdress[i]);
    html += "<li>";
    html += "Temperature: " + String(temperature) + " " + addr;
    html += "</li>";
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