// web server & SPIFFS routines
#ifndef MYSWEB_H
#define MYSWEB_H

#define INCHANS 1 // not sure we need this
//#include "myLInst.h"
//#include "myLoad.h"
#include "FS.h"
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include "my_mx_defs.h"


void slashRequest(AsyncWebServerRequest *request, int client);
void webCommands(AsyncWebServerRequest *request);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void makeJSONreads(char screen, int client);
void makeJSONsets(char screen, int client);
void printParamsX(AsyncWebServerRequest *request);
void dumpHex(const void* data, size_t size) ;
void makeJSONpreset(int preNum);
void printTail(char * str, int tailLen);
int getClient(AsyncWebServerRequest *request);
char getScreen(AsyncWebServerRequest *request);
int registerHTTPslave(AsyncWebServerRequest *request, int session);
void printParams(AsyncWebServerRequest *request);

void setDig(int chan, int val);
void makeLog(void);
void makeLogJSON(int);
uint8_t setModeB(uint8_t bMode);
uint8_t setMode(uint8_t mode);
void onOff(int8_t channel, bool status);
int startBAT(int x);
int startST(int x);
 
bool screenChanged = true; // not valid for multiple HTTP sessions - so ignore

const String PARAM_INPUT_CMD = "cmd";
const String PARAM_SCREEN_CMD = "screen";
const String PARAM_CLIENT_CMD = "clientID";
const String PARAM_SESSION_CMD = "sessionID";
const String PARAM_INPUT_1 = "value_1";
const String PARAM_INPUT_2 = "value_2";
#define CMD_TOGGLE 1
#define CMD_SCREEN 2
#define CMD_PAD 3
#define CMD_FADER 4
#define CMD_INAME 5
#define CMD_ONAME 6
#define OUTLEN 16

// cut down web traffic by only sending faders and names occasionally with level packets (unless a chanage has been flagged)
// this needs to be significantly 

#define UPDATESEVERY
#define SENDNAMESEVERY 4
#define SENDFADERSEVERY 3

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
int levelLoops = 0;

void webServerStart()
{      
 // sptr[WEBSTREAM] = WEBSTREAMP;  
  // Servicing for input and Output page requests (new screen)
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
  {
    // screen changes
    screenChanged = true;	
	//Serial.print("/ ");
  valChanged(SET_ALL); // new page request gets all settings.
	_thisClient = getClient(request); // used in processor()
	_thisScreen = getScreen(request);
	//printParams(request);
	//Serial.printf("Sending / to client %i\n", _thisClient);	
    slashRequest(request, _thisClient);  
	
	//Serial.print("Request from ");	
	//Serial.print(request->client()->remoteIP());
	//	Serial.printf(":R%i ",request->client()->remotePort());
	//	Serial.printf(":L%i\n",request->client()->localPort());
	//	Serial.printf(":L%i\n",request->client()->localPort());
    request->send_P(200, "text/html", index_html, processor);
  });

  // settings screen page 
  server.on("/settings", HTTP_ANY, [](AsyncWebServerRequest *request)
  {
    // screen changes
	 Serial.print("/settings ");
	_thisClient = getClient(request);
	_thisScreen = getScreen(request);
	//printParams(request);
    slashRequest(request, _thisClient);     
    request->send_P(200, "text/html",  index_html, processor); //settings_html
  });

  //fast GET from web client: send readings screenID and other data as needed 
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request)
  { 
    char scrn[2] = "M";      // default to main screen
    bool faderCh, padCh, namesCh;

  //Serial.print("/readings ");
 //   	printParams(request);
	_thisClient = getClient(request);
	_thisScreen = getScreen(request);
	//Serial.printf("Get Screen %c, client %i\n",_thisScreen, _thisClient);
	//printParams(request);

	strcpy((char *)&JSONstring, "{\n");
    makeJSONreads(_thisScreen, _thisClient); // may only have "Screen"
	//String temp = JSONstring;
	//	Serial.print("JSON R|");
//	Serial.println(JSONstring); 
	//Serial.print("|\n");
		
// ******************** only append these if valChanged_broadcast
	strcat((char *)&JSONstring, ",\n");
	makeJSONsets(_thisScreen, _thisClient); 
	strcat((char *)&JSONstring, "\n}");
	
	//Serial.print("JSON: |");
	//Serial.print(JSONstring);
//	Serial.print("|\n");

    request->send(200, "text/plain", JSONstring);
  });

  // input controls from Web page
  // Process a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  // JS GET. process commands from web page - update sliders, buttons, change screen
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
	  // Serial.print("/update ");
	  //printParams(request);
	 _thisClient = getClient(request);
	 _thisScreen = getScreen(request);
     webCommands(request);
     request->send(200, "text/plain", "OK");
  });
  
  
// generic calls, no need to know which client requests.
  // load CSS, JS and icon files
  server.on("/myKnife.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/myKnife.css", "text/css");
  });
  server.on("/myKnife.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/myKnife.js", "text/javascript");
  });
    server.on("/jogDial.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/jogDial.js", "text/javascript"); // "/jogDial.min.js"
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico", "text/plain");
  });

  server.on("/base_bg.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/base_bg.png", "image/png");
  });
  server.on("/base_knob.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/base_knob.png", "image/png");
  });

  // Start server
  server.begin();
}

// getClient() and getScreen() may be from GET or POST requests. 
// All other variables are GET only

int getClient(AsyncWebServerRequest *request)
{
	String bufS, bufC;
	bool gotClient = false, gotSession = false;
	int session = -1, bufI;
	// find clientID parameter in the request
	// none? flag error
	// if the clientID == -1, register new client
	// return the clientID
	
	if(request->hasParam(PARAM_SESSION_CMD, true)) // POST 
	{
		//Serial.print(" SE_POST ");
		bufS = request->getParam(PARAM_SESSION_CMD, true)->value();	
		//bufI = request->getParam(PARAM_SESSION_CMD, true)->value();	
		gotSession = true;
		session = bufS.toInt();
	}
	
	if(request->hasParam(PARAM_SESSION_CMD)) // GET 
	{
		//Serial.print(" SE_GET ");
		bufS = request->getParam(PARAM_SESSION_CMD)->value();	
		//bufI = request->getParam(PARAM_SESSION_CMD)->value();	
		gotSession = true;
		session = bufS.toInt();
	}
	//Serial.printf("SessionID = |%s| %i\n", bufS.c_str(), session);
	_thisSession = session;
	
	return 1;
}

char getScreen(AsyncWebServerRequest *request)
{
	String ss;
	char screen;
	bool gotScreen = false;
	if (request->hasParam(PARAM_SCREEN_CMD)) // GET
    {
		 //Serial.print(" SC_GET ");
		 ss = request->getParam(PARAM_SCREEN_CMD)->value();	
		 gotScreen = true;
	}
		// convert string to character
	if(request->hasParam(PARAM_SCREEN_CMD, true)) // POST 
	{
		//Serial.print(" SC_POST ");
		ss = request->getParam(PARAM_SCREEN_CMD, true)->value();	
		gotScreen = true;
	}
	if(gotScreen)
	{
		screen = (ss.c_str())[0];	
		return screen;
	}		
	
	Serial.printf("Missing screen ID in request\n");
	return 'M';	// default to Input
}
// look for a spare slot, otherwise kick out first sleepy one

// service "/" request
// just upload the page (done in server.on() above)
// make sure the next /update includes screen info
void slashRequest(AsyncWebServerRequest *request, int client)
{ 
		currentWebScreen = 'M';	// default for new page requests
}

// service "/update" request
// send JSON of required data
// always readings, screen info and other data as required.
/*************** WEB COMMANDS ********************************/
void webCommands(AsyncWebServerRequest *request)
{
    int cs = currentWebScreen - '0'; // char to int conversion
    cs = constrain(cs, 0,7);
    int btn;
    char val1[4];	
    bool foundCmd = false;
	  screenChanged = true; 
    String inputCmd = "No cmd.";
    String inputParam1 = "No param_1.";
    String inputParam2 = "No param_2.";
    char val_1[128] ="No val_1.";
    char val_2[128] ="No val_2.";
    char val_s[16] ="X";
    char scrnChar = 'X';
	  int scrn;	//[0..7 or 29 for input]

   // all requests should have cmd and scn; may have value-1 and value_2 parameters.
    int cmd = -1;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
     if (!request->hasParam(PARAM_INPUT_CMD)) // every request should have this argument
     {
      Serial.println("Error: request missing CMD parameter");
      return;
     }
    
    inputCmd = request->getParam(PARAM_INPUT_CMD)->value();
    cmd = inputCmd.toInt();
    // all requests must have cmd and scrn parameters.
    // Serial.printf("/update: cmd=|%s|",inputCmd);

    if (request->hasParam(PARAM_INPUT_1)) 
    {
       inputParam1 = request->getParam(PARAM_INPUT_1)->value();
       inputParam1.toCharArray(val_1, 128);
       
       // val_1 format: "pad_X" 
       strncpy((char *)&val1, val_1, 3);  // first three chars tell object type
       btn = val_1[4] - '0' ;  // index to button array  
      // Serial.printf(" value_1=|%s|, btn %i; ",val_1, btn);
    }
    if (request->hasParam(PARAM_INPUT_2)) 
    {
      inputParam2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2.toCharArray(val_2, 128);
      //Serial.printf(" value_2=|%s|\n",val_2);
    }
	  char ssc = getScreen(request);
    if (ssc != 'X') // every request should have this argument
    {
      //inputParam2 = request->getParam(PARAM_SCREEN_CMD)->value();
      //inputParam2.toCharArray(val_s, 16);
      scrnChar = ssc;
	  scrn = scrnChar - '0';
     // Serial.printf(" screen=|%s|%c| \n",val_s, scrnChar);
    }
    else
    {
      Serial.println("Error: request missing SCREEN parameter");
      return;
    }
	float tempF = atof(val_2);
  int   tempI = atoi(val_2);

	// process the commands 
    uint8_t dummy, * vp = &dummy;
	// main screen

	if(strcmp(val_1, "volts1SID") == 0)
 {
		setDAC(tempF);	
    foundCmd = true;
  }
    
  if(strcmp(val_1, "freqSID") == 0)
  {
    setSineFreq(tempI); 
    foundCmd = true;
  }
  if(strncmp(val_1, "SSR",3) == 0)
  {
    Serial.printf("SSR: %i\n",tempI);
    setSineScale(tempI); 
    foundCmd = true;
  } 
    
  if(strcmp(val_1, "potSID") == 0)
  {
    setPot(tempI); 
    foundCmd = true;
  }

  // cal updates: come in as float offset to values 
  if(strcmp(val_1, "calD1SID") == 0)
  {
    DACtempCalS = _aOutVal; // current reading
    DACtempCalOff = tempF; // offset
    DACtempCalC = dacVal; //  counts to match reading
    Serial.printf("calD1SID: s = %2.2f, O = %2.2f, C = %i\n",DACtempCalS, DACtempCalOff, DACtempCalC);  
    foundCmd = true;  
  }

  if(strcmp(val_1, "calA1SID") == 0)
  {
    ADCtempCalR[0] = _ainVal[0];
    ADCtempCalOff[0] = tempF; 
    ADCtempCalC[0] = _ainCounts[0];
    Serial.printf("calA1SID: %2.2f\n", ADCtempCalOff[0]);
    foundCmd = true;   
  }
  if(strcmp(val_1, "calA2SID") == 0)
  {
    ADCtempCalR[1] = _ainVal[1];
    ADCtempCalOff[1] = tempF; 
    ADCtempCalC[1] = _ainCounts[1];
    Serial.printf("calA2SID: %2.2f\n", ADCtempCalOff[1]);
    foundCmd = true;   
  }
       
  // update the calibrations and save
  if(strcmp(val_1, "calSave") == 0)
  {
    Serial.printf("calSave\n");
    setCal();    
    foundCmd = true;
  }

  // relays
  if(strcmp(val_1, "R1") == 0)  
  {
    setRelay(0, tempI);
    foundCmd = true;
  }
  if(strcmp(val_1, "R2") == 0) 
  {
    setRelay(1, tempI);
    foundCmd = true;
  }
  if(strcmp(val_1, "R3") == 0) 
  {
    setRelay(2, tempI);
    foundCmd = true;
  }

  // dig out
  if(strcmp(val_1, "D1") == 0)
  {
    //Serial.printf("D1 %i [%s]\n", tempI, val_2);
    setDig(0, tempI);    
    foundCmd = true;
  }
  if(strcmp(val_1, "D2") == 0)
  {
    //Serial.printf("D2 %i [%s]\n", tempI, val_2);
    setDig(1, tempI); 
    foundCmd = true;
  }

  if (!foundCmd)
    Serial.printf("WebCmd not found [%s] = [%s]\n", val_1, val_2);   
}
	
/* JSON
 * Readings only
 * All variable names are of the form "ccccIJ". J is context optional. cccc is generally the name of the HTML id of the control, I is the control number usually [0..7]
 */
 // just the core JSON lines. Calling routine needs to add leading { and trailing , or }
void makeJSONreads(char screen, int client)
{
  char nums[16];  
  char scrStr[20] = "0";

	//Serial.printf("Request /update has %i params:\n",request->params());
	// printParams(request);
	//Serial.printf("/levels to %i: flags 0x%4x: ", client, slave[client].vChanged);
	//printClientChanges(client);

	//Serial.printf("faders & gains %i, pads %i, names %i\n", faderGains, pads, names);
  // screen first
  // only add these immediately after a screen change (or name update?)
  // or pending name changes on web page will be overwritten
  //char sc = screen[0];
  
  int scn = screen - '0'; // integer version of screen 
  if(scn < 0 || scn > INCHANS) 
    scn = -1;
  scrStr[0] = screen;

	strcat((char *)&JSONstring, "\"screen\":\"");
	strcat((char *)&JSONstring, scrStr);
	strcat((char *)&JSONstring, "\", ");
	
	strcat((char *)&JSONstring, "\"device\":\"");
	strcat((char *)&JSONstring, hostname);
	strcat((char *)&JSONstring, "\"");

  // readings
	if(screen =='M')
	{
		strcat((char *)&JSONstring, ",\n\"volts1R\" : ");
		sprintf((char *)&nums, "%3.3f",  _ainVal[0]); 
		strcat((char *)&JSONstring, nums);
    
    strcat((char *)&JSONstring, ", \"volts2R\" : ");
    sprintf((char *)&nums, "%3.3f",  _ainVal[1]); 
    strcat((char *)&JSONstring, nums);

    strcat((char *)&JSONstring, ",\n\"dig1R\" : ");
    sprintf((char *)&nums, "%i",  _dInVal[0]); 
    strcat((char *)&JSONstring, nums);
    
    strcat((char *)&JSONstring, ", \"dig2R\" : ");
    sprintf((char *)&nums, "%i",  _dInVal[1]); 
    strcat((char *)&JSONstring, nums);
	}
	if(screen =='S')
	{
    // ADC
		strcat((char *)&JSONstring, ",\n\"volts1R\" : ");
    sprintf((char *)&nums, "%3.3f",  _ainVal[0]); 
    strcat((char *)&JSONstring, nums);
    
    strcat((char *)&JSONstring, ", \"volts2R\" : ");
    sprintf((char *)&nums, "%3.3f",  _ainVal[1]); 
    strcat((char *)&JSONstring, nums);

    // DAC
    strcat((char *)&JSONstring, ", \"volts1S\" : ");
    sprintf((char *)&nums, "%3.3f", _aOutVal); 
    strcat((char *)&JSONstring, nums);

  }
   
  //  Serial.printf("JSON-readings: scrn %c |%s| %i\n", screen, JSONstring, strlen(JSONstring));  
  // dumpHex((void *)JSONstring,  strlen(JSONstring));
  screenChanged = false;
}

// just the core JSON lines. Calling routine needs to add leading { and trailing , or }
void makeJSONsets(char screen, int client)
{
  char nums[16];  
  char scrStr[20] = "0";
//  bool pads = false, faderGains = false, names = false;
	//Serial.printf("Request /update has %i params:\n",request->params());
	// printParams(request);
	//Serial.printf("/levels to %i: flags 0x%4x: ", client, slave[client].vChanged);
	//printClientChanges(client);
 
	//Serial.printf("faders & gains %i, pads %i, names %i\n", faderGains, pads, names);
  // screen first
  // only add these immediately after a screen change (or name update?)
  // or pending name changes on web page will be overwritten
  //char sc = screen[0];
  
  int scn = screen - '0'; // integer version of screen 
  if(scn < 0 || scn > INCHANS) 
    scn = -1;
  scrStr[0] = screen;
  //strcpy((char *)&JSONstring, "{\n");

	 strcat((char *)&JSONstring, "\"screen\":\"");
	 strcat((char *)&JSONstring, scrStr);
	 strcat((char *)&JSONstring, "\", ");
	 
	 strcat((char *)&JSONstring, "\"device\":\"");
	 strcat((char *)&JSONstring, hostname);
	 strcat((char *)&JSONstring, "\"");
	 if(screen =='M')
	 {
		 // value settings
     if(_changedVal & SET_A1)
     {
    		strcat((char *)&JSONstring, ", \"volts1S\" : ");
    		sprintf((char *)&nums, "%3.3f", _aOutVal); 
    		strcat((char *)&JSONstring, nums);
        changeSent(SET_A1);
     }
     if(_changedVal & SET_SF1)
     {
        strcat((char *)&JSONstring, ", \"freqS\" : ");
        sprintf((char *)&nums, "%i", sineConf.freq); 
        strcat((char *)&JSONstring, nums); 
        changeSent(SET_SF1);
     }  
     if(_changedVal & SET_SS1) // sine scale
     {
        strcat((char *)&JSONstring, ", \"SSR\" : ");
        sprintf((char *)&nums, "%i", cw_scales[sineConf.scale].scaleDiv); // use the scale multiplier here
        strcat((char *)&JSONstring, nums); 
        changeSent(SET_SS1);
     }    
     
     if(_changedVal & SET_D1)
     {
      //  strcat((char *)&JSONstring, ",\"dig1S\" : ");
          strcat((char *)&JSONstring, ", \"D1\" : ");
        sprintf((char *)&nums, "%i", _dOutVal[0]); 
        strcat((char *)&JSONstring, nums);  
        changeSent(SET_D1);
     }  
     if(_changedVal & SET_D2)
     {
       // strcat((char *)&JSONstring, ",\"dig2S\" : ");
          strcat((char *)&JSONstring, ", \"D2\" : ");
        sprintf((char *)&nums, "%i", _dOutVal[1]); 
        strcat((char *)&JSONstring, nums);  
        changeSent(SET_D2);
     }
     if(_changedVal & SET_R1)
     {
       // strcat((char *)&JSONstring, ",\"rly1S\" : ");
        strcat((char *)&JSONstring, ", \"R1\" : ");
        sprintf((char *)&nums, "%i", _relayVal[0]); 
        strcat((char *)&JSONstring, nums);  
        changeSent(SET_R1);
     }
     if(_changedVal & SET_R2)
     {
       // strcat((char *)&JSONstring, ", \"rly2S\" : ");
         strcat((char *)&JSONstring, ", \"R2\" : ");
        sprintf((char *)&nums, "%i", _relayVal[1]); 
        strcat((char *)&JSONstring, nums);  
        changeSent(SET_R2);
     }      
     if(_changedVal & SET_R3)
     {
        //strcat((char *)&JSONstring, ", \"rly3S\" : ");
         strcat((char *)&JSONstring, ", \"R3\" : ");
        sprintf((char *)&nums, "%i", _relayVal[2]); 
        strcat((char *)&JSONstring, nums);  
        changeSent(SET_R3);
     }            
     if(_changedVal & SET_POT)
     {
        strcat((char *)&JSONstring, ", \"potS\" : ");
        sprintf((char *)&nums, "%i", _dPotVal); 
        strcat((char *)&JSONstring, nums);  
        
        strcat((char *)&JSONstring, ", \"potPcS\" : "); // % value as well
        sprintf((char *)&nums, "%i", _dPotVal * 100 / 255); 
        strcat((char *)&JSONstring, nums);  
        changeSent(SET_POT);
     }
	 }
  //  Serial.printf("JSON-settings: scrn %c |%s| %i\n", screen, JSONstring, strlen(JSONstring));  
  //Serial.printf("Changes 0x%x\n", _changedVal);
  // dumpHex((void *)JSONstring,  strlen(JSONstring));
  screenChanged = false;
}
 

void printTail(char * str, int tailLen)
{
  int sl = strlen(str);
  if(sl  <  tailLen)
    tailLen = sl;
  str += (sl - tailLen);
  Serial.printf("Tail: ...|%s|\n",str);
}
void dumpHex(const void* data, size_t size) {
  char ascii[17];
  size_t i = 0, j;
  ascii[16] = '\0';
 Serial.printf("\n%3i: ", i);
  for (i = 0; i < size; ++i) {
    Serial.printf("%02X ", ((unsigned char*)data)[i]);
    if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
      ascii[i % 16] = ((unsigned char*)data)[i];
    } else {
      ascii[i % 16] = '.';
    }
    if ((i+1) % 8 == 0 || i+1 == size) {
      Serial.printf(" ");
      if ((i+1) % 16 == 0) {
        Serial.printf("|  %s \n%3i: ", ascii, i+1);
      } else if (i+1 == size) {
        ascii[(i+1) % 16] = '\0';
        if ((i+1) % 16 <= 8) {
          Serial.printf(" ");
        }
        for (j = (i+1) % 16; j < 16; ++j) {
          Serial.printf("   ");
        }
        Serial.printf("|  %s \n", ascii);
      }
    }
  }
}
void printParams(AsyncWebServerRequest *request)
{
	int params = request->params();
	Serial.printf(" Request has %i params\n", params);
	for(int i=0;i<params;i++)
	{
	  AsyncWebParameter* p = request->getParam(i);
	  if(p->isFile()) //p->isPost() is also true
	  {
		Serial.printf(" FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
	  } else 
		  if(p->isPost())
		  {
			Serial.printf(" POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
		  } else 
		  {
			Serial.printf(" GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
		  }
	}
}
void printParamsX(AsyncWebServerRequest *request)
{
  int paramsNr = request->params();

  Serial.printf("Request has %i params:\n",paramsNr);
  for(int i=0;i<paramsNr;i++)
  { 
     AsyncWebParameter* p = request->getParam(i);
 
     Serial.print("Name: |");
     Serial.print(p->name());
 
     Serial.print("| Value: |");
     Serial.print(p->value());    
     Serial.println("|");
  }
   Serial.println("------");
}

#endif
