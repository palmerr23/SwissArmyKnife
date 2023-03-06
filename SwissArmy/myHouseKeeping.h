#ifndef MYHOUSEKEEPING_H
#define MYHOUSEKEEPING_H
#include <ESPmDNS.h>
// Wifi and other housekeeping
void errorMsg(String error, bool restart = true);
void comms_setup(void);

uint16_t _changedVal = 0xffff; // bitmap - all values need refreshing on startup

static float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setupSerial(long speed) {
  Serial.begin(speed);
  while (!Serial) {
  }
  delay(200);  
}

#define CONNWAIT 15
bool WifiConnected;
IPAddress myIP;
bool wifi_begin(char * ssid, char * pass)
{
   int i = 0;
   if (WiFi.status() == WL_CONNECTED) 
     WiFi.disconnect();
#ifdef PRINT_DIAGNOSTICS
   Serial.printf("Connecting to Wifi with SSID = \"%s\" and Passphrase = \"%s\"\n",ssid, pass);
#endif

   WiFi.begin(ssid, pass);
   i = 0;
   while ((WiFi.status() != WL_CONNECTED) && (i++ < CONNWAIT)) 
   {
      delay(1000); 
	  Serial.print(">");
   }

   if (WiFi.status() == WL_CONNECTED) 
   {
      Serial.printf("\nConnected to WiFi network: SSID %s\n",ssid);
      myIP = WiFi.localIP();
	    //myBroadcastIP = WiFi.broadcastIP();
      //mySubnetMask = WiFi.subnetMask();
	    //setHostNameIP ();
	    WifiConnected = true;
      ip = WiFi.localIP();
      Serial.print("IP ");
      Serial.println(ip); 

      if(MDNS.begin(hostname)) 
      {
        Serial.printf("Hostname is %s.local\n", hostname);     
        return true;
      }
   }  
   errorMsg("\nError connecting to WiFi", false);
   return false;
}

bool isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

void errorMsg(String error, bool restart) 
{
  Serial.print(error);
  strcat(tstBuf, error.c_str());
  if (restart) {
    Serial.println("Rebooting now...");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
}
void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  Serial.println(">>IDN*");
  interface.println("Platy,SwissArmy,00,v0.1");
}
void selfTest(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
    interface.println(tstBuf);
}

// flag a changed value to update other streams
void valChanged(uint16_t valId)
{
      _changedVal |= valId; // set
}
// register change has been broadcast
void changeSent(uint16_t valId)
{
      _changedVal &= ~valId; //clear
}

#endif
