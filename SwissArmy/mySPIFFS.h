#ifndef MYSPIFFS_H
#define MYSPIFFS_H
#include "FS.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true
#include <ArduinoJson.h>
#define BLKSIZE 384
char proFile[] = "/profile.json";
//StaticJsonDocument<BLKSIZE> jDoc;
DynamicJsonDocument jDoc(BLKSIZE);
uint8_t fileBuf[BLKSIZE];	

void printProfile(void);
void printCal(void);
bool setProfile(bool create =  false);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void errorMsg(String error, bool restart);

bool SPIFFSstart()
{
  if(SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
  {
    Serial.println("SPIFFS Mounted"); 
    // listDir(SPIFFS, "/", 4);
     return true;
  } 
  else
    Serial.println("SPIFFS Mount Failed");
  return false;
} 

// profile is SSID, Password and Hostname in JSON format
// assumes whole file is < 512 bytes.
bool getProfile(void)
{
  jDoc.clear();
	File file = SPIFFS.open(proFile);

	if(!file)
	{
		 Serial.printf("%s − failed to open for reading\n", proFile);
		 setProfile(true);	// create a fresh profile
		 return false;
	}
	int len = file.size();
	file.read(fileBuf, len);
#ifdef PRINT_DIAGNOSTICS
	Serial.printf("read [%s]\n", fileBuf);
#endif
	
	DeserializationError error = deserializeJson(jDoc, fileBuf);
	if(error) 
	{
	  Serial.println("Parsing JSON failed");
    setProfile(true);  // create a fresh profile
	  return false;
	}
	file.close();
  // ssid should be always present
  if(!jDoc.containsKey("ssid"))
  {
    setProfile(true);  // create a fresh profile
    return false;
  }
	strcpy(ssid, jDoc["ssid"]);
	strcpy(pass, jDoc["pass"]);
	strcpy(hostname, jDoc["hostname"]);

  // Cal values
  // these won't be set until first calibration has been saved
  if(jDoc.containsKey("DAC1"))
    DACcalVal = jDoc["DAC1"];
  if(jDoc.containsKey("ADC1"))
    ADCcalVal[0] = jDoc["ADC1"];
  if(jDoc.containsKey("ADC2"))
    ADCcalVal[1] = jDoc["ADC2"];
#ifdef PRINT_DIAGNOSTICS	
	printProfile();
#endif
	return true;
}
bool setProfile(bool create)
{
  char * bp = (char *)fileBuf; // serializeJson() needs char, file.write needs uint8_t
	fileBuf[0] = 0; // empty the buffer (char NULL)
  jDoc.clear();
	//char lbuf[64];

  if(create) // dummy values, can be updated by SCPI (serial only!)
  {
    jDoc["ssid"] = "dummySSID";
    jDoc["pass"] = "dummyPass";
    jDoc["hostname"] = "SwissArmy"; 
    errorMsg("Creating new profile with dummy WiFi credentials.\nRe-upload ESP32 Sketch Data or set values via serial SCPI commands.", false);
  }
  else // update
  {
    jDoc["ssid"] = ssid;
    jDoc["pass"] = pass;
    jDoc["hostname"] = hostname;    
  }

  jDoc["DAC1"] = DACcalVal;
  jDoc["ADC1"] = ADCcalVal[0];
  jDoc["ADC2"] = ADCcalVal[1];
   /* */
  //  Serial.println("JSON: "); 
  //  serializeJsonPretty(jDoc, Serial);
  //return false; // serialize clears the jDoc?
 
  serializeJsonPretty(jDoc, bp, BLKSIZE);
	int len = strlen((char *)fileBuf);
//  Serial.printf("Saving JSON. len = %i [%s]\n", len, fileBuf);

  File file = SPIFFS.open(proFile ,FILE_WRITE);
  if(!file)
  {
     Serial.printf("%s − failed to open for writing\n", proFile);
     return false;
  }
	int savd = file.write(fileBuf, len+1); // write the closing \0 as well.
	file.close();
 // Serial.printf("Saved %i bytes\n", savd);
	return true;
}
void printProfile(void)
{
	Serial.printf("Profile:\nssid : [%s]\npass : [%s]\nhostname : [%s]\n", ssid, pass, hostname);
  printCal();
}
// levels = recursion depth
void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
              Serial.printf("Recursive to %s\n",file.name());
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
#endif
