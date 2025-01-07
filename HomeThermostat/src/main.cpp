#include <Arduino.h>
#include <WiFiTools.h>
#include <WebServer.h>

// put function declarations here:
//int myFunction(int, int);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  initAp();
}

void loop() {
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);  
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}