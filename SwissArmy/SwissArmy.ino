//#define PRINT_DIAGNOSTICS
//#define REAL_WIFI
#include <esp_task_wdt.h> 
#include "ESPTelnet.h"    // https://github.com/LennartHennigs/ESPTelnet         
#include <string>
#include <TelnetStream.h> // https://github.com/jandrassy/TelnetStream // includes Stream.h
#define SCPIPORT 5025

ESPTelnet telnet;
IPAddress ip;

#define SSIDLEN 32
#define PASSLEN 63
#define HOSTLEN 16
// Values read from profile.json
char ssid[SSIDLEN+1];   // your network SSID (name)
char pass[PASSLEN+1];    // your network password
char hostname[HOSTLEN+1] = "SwissArmy";

// Note: doubled the standard quantities of Tokens etc in the .h file
#include "Vrekrer_scpi_parser.h" // https://github.com/Vrekrer/Vrekrer_scpi_parser
SCPI_Parser my_instrument;

#define RELAYS 2
#define DACS  1
#define SINE  1
//#define AOUT (DACS) // +1 is for DAC2's +/- setting 
#define POTS  1
#define DOUT  2
#define DIN  2
#define AIN 2   // differential reads

#define NUMSETS (RELAYS+DOUT+DACS+POTS) 
#define SET_A1  0x0001 
#define SET_SF1  0x0002
#define SET_SS1  0x0004
#define SET_D1  0x0008
#define SET_D2  0x0010
#define SET_R1  0x0020
#define SET_R2  0x0040
#define SET_R3  0x0080
#define SET_POT 0x0100
#define SET_AR1 0x1000 
#define SET_AR2 0x2000 
#define SET_AR3 0x4000 
#define SET_AR4 0x8000 
#define SET_ALL 0xffff  //  bitmap 16

#define VMAXOP (10.0)
#define VMIN (0.0)
#define CALMAXTWEAK (2.0)
#define SMALLBIT (0.005) // float: not zero

const int relayPin[RELAYS] = {27, 33}; // power + 2 reed
const int dacPin = 25; 
const int dInPin[DIN] = {34,35}; 
const int dInLED[DIN] = {5,18}; 
const int ledPin = 2; // on module
const int dOutPin[DOUT] = {2,4};
const int aReadyPin = 13; // not used

char tstBuf[256] = "";
#include "myHouseKeeping.h"
//#include "mySCPI_PWM.h"
#include "mySCPI_relay.h"
#include "mySCPI_analog.h"
#include "mySCPI_sine.h"
#include "mySCPI_digital.h"
#include "mySCPI_DPOT.h"
#include "mySCPI_COMMS.h"
#include "mySPIFFS.h"
#include "mySHTML_literal.h"
#include "mySHTML_processor.h"
#include "mySwebserver.h"


#define SERIAL_SPEED    115200 // boot messages are at this speed
long thisTime;
void setup() {
  setupSerial(SERIAL_SPEED);
  delay(2000);
  Serial.println("Swiss Army Knife");
  // Register SCPI base commands before setting up others.
  // All re-uses of a token must have EXACTLY the same form
  my_instrument.RegisterCommand("*IDN?", &Identify); // common across all devices
  my_instrument.RegisterCommand("*TST?", &selfTest); // common across all devices
  // hardware
  SCPI_relay_setup();
  SCPI_digital_setup();
  SCPI_analog_setup();
  SCPI_sine_setup();
  SCPI_dPot_setup();
  SCPI_comms_setup();

  // profile
  SPIFFSstart();
  // listDir(SPIFFS, "/", 0);
  getProfile();
  Serial.print("Wifi: ");
  wifi_begin(ssid, pass);
  TelnetStream.begin(SCPIPORT);
  webServerStart();
    
  if(strlen(tstBuf) == 0)
    strcpy(tstBuf, "No self-test errors detected.\n");

  Serial.printf("Setup complete: %s\n",tstBuf);
  // my_instrument.PrintDebugInfo();
  thisTime = millis();
}

uint8_t thisADC = 0;
uint8_t thisDig = 0;

#define REPORTEVERY 10000 // diagnostic output
void loop() {
  // read one analog and digital input per cycle
  readADCx(thisADC);
  thisADC = ++thisADC % AIN;
  yield();
  readDig(thisDig);
  //Serial.printf("Dig [%i] = %i\n", thisDig, readDig(thisDig));
  thisDig = ++thisDig % DIN;
  yield();
  // process remote requests. Web requests are asynchronous.
  my_instrument.ProcessInput(Serial, "\n"); // this SCPI works fine on Serial.
  yield();
  my_instrument.ProcessInput(TelnetStream, "\n"); // 
  esp_task_wdt_reset(); // feed the watchdog regularly or it may time out.
  yield();
  if(millis() - thisTime > REPORTEVERY)
  {
    thisTime = millis();  
#ifdef PRINT_DIAGNOSTICS
    Serial.printf("\nChanges: 0x%x\n",_changedVal);
    printSettings();
    printReadings(); 
#else
   Serial.printf("T = %i\n",millis()/1000);
#endif
  }
}
void printReadings(void)
{
  Serial.println("\nReadings:");
  printAreadings(false);
  printDreadings();
}

void printSettings(void)
{
  Serial.println("\nSettings:");
  printAsettings();
  printSsettings();
  printPsettings();
  printRsettings();
  printDsettings();
 
 
}
