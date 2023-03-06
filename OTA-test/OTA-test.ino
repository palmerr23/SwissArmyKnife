// Swiss Army Knife OTA loader and basic tests

// A useful OTA tutorial:
//https://randomnerdtutorials.com/esp32-over-the-air-ota-programming/

// OTA user/password = admin/admin

const char* ssid = "MYSSID";  // your WiFi credentials go here
const char* password = "MYPASSWD";

const char* host = "SwissArmy";

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Wire.h>
#include "mywifi.h"
#include "web.h"

#define relayPin1 27
#define relayPin2 33

#define digOutPin1 2
#define digOutPin2 4

#define digInLED1 5
#define digInLED2 18

#define dacPin1 25
#define dacPin2 26

#define ADC_ADDR 0x48
#define DPOT_ADDR 0x3C

void setup(void) {
  Serial.begin(115200);
  delay(5000);
  Serial.println("OTA loader and Swiss Army Knife basic tests."); 
  wifiStart();
  webserverStart(); 
  knifeStart();
  knifeTest();  
  Serial.println("Setup done. Now toggling relays and digital outputs, DAC staircases.\n"); 
}

void loop(void) {
  server.handleClient();
  changeOutputs();
  delay(2000);
}

void knifeStart()
{
  Wire.begin();
  pinMode(relayPin1,OUTPUT);
  pinMode(relayPin2,OUTPUT);
  pinMode(digOutPin1,OUTPUT);
  pinMode(digOutPin2,OUTPUT);  
  pinMode(digInLED1,OUTPUT);  
  pinMode(digInLED2,OUTPUT);  
}
void knifeTest(void)
{
  bool res;
  res = probeI2C(ADC_ADDR);
  Serial.printf("ADC %s at I2C address 0x%x\n", (res)? "found" : "NOT found", ADC_ADDR);

  res = probeI2C(DPOT_ADDR);
  Serial.printf("Digital pot %s at I2C address 0x%x\n", (res)? "found" : "NOT found", DPOT_ADDR);
}
bool probeI2C(uint8_t I2Caddr)
{
  Wire.beginTransmission (I2Caddr);
  return (Wire.endTransmission() == 0);
}
bool state = false;
uint8_t dacVal = 0;
void changeOutputs(void)
{
  digitalWrite(relayPin1, state); // toggle relays
  digitalWrite(relayPin2, !state);
  
  digitalWrite(digOutPin1, state); // toggle digital outputs
  digitalWrite(digOutPin2, !state);

  digitalWrite(digInLED1, state); // toggle digital input LEDs
  digitalWrite(digInLED2, !state);
  
  dacWrite(dacPin1, dacVal);  // 6 step staircase
  dacWrite(dacPin2, 255 - dacVal); // reverse staircase
  
  state = !state;
  dacVal = (dacVal + 32) % 256; 
}
