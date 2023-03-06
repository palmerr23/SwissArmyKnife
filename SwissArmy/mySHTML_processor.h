//Processor code 
// Replaces placeholders in the web page literal code
// Control element numbers have first three chars identify control, 5th char = channel number [0.. INCHANS-1]
#ifndef MYSHTTPX_H
#define MYSHTTPX_H
//#include <sstream>
char JSONstring[4096];
char currentWebScreen = 'M'; // start with iNput screen
char xbuf[64];
#define NUMSCREENS 2
// MAIN, STEP, LOG
char screenNames [][8] = {"Main","Cal", "Log"}; // goes on the button and in headline
char screenList[] = {'M','S','L'}; // actual reference
String readingsBlock(void);
String settingsBlock(void);
String calSettingsBlock(void);
String calValuesBlock(void);
String calSave(void);
String digOutButtons(void);
String sRangeButtons(void);
String knobBlock(uint8_t pre,uint8_t post, int8_t whichBut);
void printSet();

int _thisClient = -1; // index to the client array, so that the right update data can be sent to each separate session.
char _thisScreen = 'M'; // HTTP screen that the request was issued from, defaults to Main settings screen
int _thisSession = -1;  // unique (randomID for session)
String processor(const String& var){
  //Serial.println(var);
   String buttons = "";

// generate entire channel group = N *(cName, level, Fader, Pad)
  if(var == "SETVALUES")
  { 
	if(_thisScreen == 'M')
	{
		buttons += "<div class=\"row\" id=\"all_sets\">";   
    buttons += readingsBlock( );
   // buttons += aRangeButtons( ); 	
  
    buttons += settingsBlock();
    buttons += knobBlock(4,1,0);
    
    buttons += digOutButtons();
		buttons += "</div>";    
	}
	else
		if(_thisScreen == 'S')
		{
			 buttons += "<div class=\"row\" id=\"all_sets\">";    
			 buttons += calValuesBlock( );
       buttons += calSettingsBlock();
			 buttons += knobBlock(1,2, -1);
       buttons += calSave();
			 buttons += "</div></div>";
		}
    return buttons;
  }
  // simplify this - only one screen
  if(var == "SCREENBUTTONS")
  {
	 String scn = (String)_thisScreen;   // current client screen
	 String cli = (String)_thisClient;   // this client's ID (-1 if undefined, but shouldn't be!)
	 String sess = (String)_thisSession;
     String action;

     for (int i = 0; i < NUMSCREENS; i++)
     {
       String cc = (String)screenList[i];  // actual ID
	     char   c  = screenList[i];
       String cn = (String)screenNames[i]; // button and heading name
	  
       if(cc == "S")
          action = "/settings"; // was settings now need to fix Main/Log?.
       else
        action = "/";
       buttons += "<form action=\"" + action + "\" class=\"buttonForm\" method=\"post\">"; // 
       buttons += "<input type=\"text\" id=\"screenFI" + cc + "\" name=\"screen\" value=\"" + cc + "\" style=\"display: none;\">";
	     buttons += "<input type=\"hidden\" id=\"sessionID"+cc+"\" name=\"sessionID\" value=\"" + sess + "\" style=\"display: none;\">"; // dummy for SessionID value - filled in by JS on client
       buttons += "<input type=\"submit\" size= 5 id=\"submit"+cc+"\" class=\"screenChangeL\" value=\" ";
       buttons +=  cn + "\"  style=\"background-color : #9cf\">"; //; display: none;
       buttons += "</form> ";
     }

	 // screen ID just used by JS
	  buttons += "<input type=\"hidden\" id=\"screenID\" name=\"screenID\" value=\"" + scn + "\" style=\"display: none;\">";
     return buttons;
  }
  // for js to decodes /levels fetches
  if (var == "BODYTYPE") // not called from settings screen
  {
	  String btype ="O";
	  if(_thisScreen == 'N')
	  {
		  btype = "N";
	  }
	  buttons += "<body onload=\"setScreen()\" id=\"body"+ btype+ "\">";
	  return buttons;
  }
   return String(); // empty string
}

//the readings block
String readingsBlock(void)
{
  char id = '0';
  String html = "";
  String idS = ""; // char id to string
  String bit;
  idS += id;
  int idN = idS.toInt();

  // embedded in a container and Row
   html += "<div class=\"column\"id=\"rCol\"  style=\"background-color: #ffe4e1; width:110px;\" >";  

		//READINGS	 - not editable - AIN
	   html += "<div class=\"row\" style=\"font-weight: bold; \">Readings</div>\n"; // width:120px;
	   html += "<div class=\"row\"><input readonly type=\"text\" class=\"reads\" id=\"volts1RID\"  name=\"volts1R\"  value=\" \" style=\"color:red;\" maxlength=\"8\" size=\"3\"><span class=\"unit\" style=\"color:red;\">V1</span></div>\n"; //size 3 seems to be big enough
     html += "<div class=\"row\"><input readonly type=\"text\" class=\"reads\" id=\"volts2RID\"  name=\"volts2R\"  value=\" \" style=\"color:red;\" maxlength=\"8\" size=\"3\"><span class=\"unit\" style=\"color:red;\">V2</span></div>\n";
        html += "<BR>";
    // DIN
    html += "<div id =\"Ddig1R\" class=\"divzr\" ><label id =\"Ldig1R\" class=\"lpad\" >";
      html += "<input type=\"checkbox\" id=\"dig1R\" disabled name=\"Ndig1R\" class=\"btnr\" /><span id =\"Sdig1R\" class=\"btnzr\">D1</span></label></div><BR> "; 
    html += "<div id =\"Ddig2R\" class=\"divzr\" ><label id =\"Ldig2R\" class=\"lpad\" >";
      html += "<input type=\"checkbox\" id=\"dig2R\" disabled name=\"Ndig2R\" class=\"btnr\" /><span id =\"Sdig2R\" class=\"btnzr\">D2</span></label></div><BR> "; 
   html += "</div>"; // col
   return html;
}

// main screen settings block
String settingsBlock(void)
{
  char id = '0';
  String html = "";
  String idS = ""; // char id to string
  String bit;
  idS += id;
  //char buf[20];
 // String mins, maxs;
  //std::ostringstream cvt;
  int idN = idS.toInt();
   // add min, max pattern attributes???
    html += "<div class=\"column\" id=\"sCol\" style=\"background-color: #cce2ff; width:115px; \" >";  
	   html += "<div class=\"row\" style=\"font-weight: bold; \">Settings</div>\n"; 
	   // for settings need to both select this one (one mousedown) for jog wheel; and validate any changes
	   html += "<div class=\"row\"><input type=\"none\" class=\"reads\" id=\"volts1SID\"  name=\"volts1S\" onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\" max=\""+ String(VMAXOP) +"\" min=\"" + String(0) + "\" value=\" \" style=\"color:red; \" size=\"3\"><span class=\"unit\" style=\"color:red;\">V</span></div>\n";

	   html += "<div class=\"row\"><input type=\"none\" class=\"reads\" id=\"freqSID\"   name=\"freqS\"  onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\" max=\""+ String(FMAX) +"\" min=\"" + String(FMIN) + "\" value=\" \" style=\"color:blue; \" size=\"3\"><span class=\"unit\" style=\"color:blue;\">Hz</span></div>\n";
 
   html += sRangeButtons();
   
	   html += "<div class=\"row\"><input type=\"none\" class=\"reads\" id=\"potSID\"  name=\"potS\" onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\" max=\""+ String(POTMAX) +"\" min=\"" + String(POTMIN) + "\" value=\" \" style=\"color:DarkGreen; \" size=\"3\"><span class=\"unit\" style=\"color:DarkGreen;\">PotC</span></div>";
	   html += "<div class=\"row\"><input type=\"none\" class=\"reads\" id=\"potPcSID\"  name=\"potPcS\" onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\" max=\""+ String(100) +"\" min=\"" + String(0) + "\" value=\" \" style=\"color:DarkOrange; \" size=\"3\"><span class=\"unit\" style=\"color:DarkOrange;\">Pot%%</span></div>";
   html += "</div>"; // col
   return html;
}

String digOutButtons(void)
{
  String html = "";
 html += "<div class=\"column\" id=\"sCol\" style=\"background-color: #cce2ff;\" padding-top: 10px; padding-bottom: 0px;>";  // not sure we need to identify this
   // buttons
   //html += "<div class=\"row\">";
  html += "<BR>";
  html += "<div id =\"DR1\" class=\"divr\"><label id =\"LR1\" class=\"lpad\" >";
  html += "<input type=\"checkbox\" id=\"R1\" name=\"NR1\" class=\"btnr\" onclick=\"clickDiv(this)\"/><span id =\"SR1\" class=\"btnzr\">R1</span></label></div><BR> "; // R1 but     

  html += "<div id =\"DR2\" class=\"divr\"><label id =\"LR2\" class=\"lpad\" >";
  html += "<input type=\"checkbox\" id=\"R2\" name=\"NR2\" class=\"btnr\" onclick=\"clickDiv(this)\"/><span id =\"SR2\" class=\"btnzr\">R2</span></label></div><BR> "; // R2 but     

  html += "<HR class=\"hrInvis\"> "; 
  
  html += "<div id =\"DD1\" class=\"divr\"><label id =\"LD1\" class=\"lpad\" >";
  html += "<input type=\"checkbox\" id=\"D1\" name=\"ND1\" class=\"btnr\" onclick=\"clickDiv(this)\"/><span id =\"SD1\" class=\"btnzr\">D1</span></label></div><BR> "; // D1 but  

  html += "<div id =\"DD2\" class=\"divr\"><label id =\"LD2\" class=\"lpad\" >";
  html += "<input type=\"checkbox\" id=\"D2\" name=\"ND2\" class=\"btnr\" onclick=\"clickDiv(this)\"/><span id =\"SD2\" class=\"btnzr\">D2</span></label></div><BR> "; // D2 but  

 html += "</div>"; // column - </div> row
 return html;
}

// calibration display values
String calValuesBlock(void)
{
  char id = '0';
  String html = "";
  String idS = ""; // char id to string
  String bit;
  idS += id;
  //char buf[20];
 // String mins, maxs;
  //std::ostringstream cvt;
  int idN = idS.toInt();

  // embedded in a container and Row
   html += "<div class=\"column\"id=\"rCol\"  style=\"background-color: #ffe4e1; width:140px;\" >";  

     //Current values   - not editable 
     html += "<div class=\"row\" style=\"font-weight: bold; float: right; margin: 0px; padding: 0px;\">Ext = Value</div>\n"; // width:120px;
     //DAC
     html += "<div class=\"row\"><input readonly type=\"text\" class=\"reads\" id=\"volts1SID\"  name=\"volts1S\"  value=\" \" style=\"color:red;\" maxlength=\"8\" size=\"3\"><span class=\"unit\" style=\"color:red;\">DAC1</span></div>\n"; //size 3 seems to be big enough
     html += "<BR>";  
     //ADC
     html += "<div class=\"row\"><input readonly type=\"text\" class=\"reads\" id=\"volts1RID\"  name=\"volts1R\"  value=\" \" style=\"color:blue;\" maxlength=\"8\" size=\"3\"><span class=\"unit\" style=\"color:blue;\">ADC1</span></div>\n"; //size 3 seems to be big enough
     html += "<div class=\"row\"><input readonly type=\"text\" class=\"reads\" id=\"volts2RID\"  name=\"volts2R\"  value=\" \" style=\"color:blue;\" maxlength=\"8\" size=\"3\"><span class=\"unit\" style=\"color:blue;\">ADC2</span></div>\n";

   html += "</div>"; // col
   return html;
}
// calibration settings block
String calSettingsBlock(void)
{
  String html = "";
   // embedded in a container and Row
    html += "<div class=\"column\" id=\"sCol\" style=\"background-color: #cce2ff; width:140px; \" >";  
     html += "<div class=\"row\" style=\"font-weight: bold; \">+ difference</div>\n"; 
     // for settings need to both select this one (one mousedown) for jog wheel; and validate any changes
     //DAC
     html += "<div class=\"row\"><input type=\"none\" class=\"reads\" id=\"calD1SID\"  name=\"calD1S\" onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\" max=\""+ String(CALMAXTWEAK) +"\" min=\"" + String(-CALMAXTWEAK) + "\" value=0.00 style=\"color:red; \" size=\"3\"><span class=\"unit\" style=\"color:red;\">DAC1</span></div>\n";
     html += "<BR>";
     //ADC
     html += "<div class=\"row\"><input type=\"none\" class=\"reads\" id=\"calA1SID\"  name=\"calA1S\" onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\" max=\""+ String(CALMAXTWEAK) +"\" min=\"" + String(-CALMAXTWEAK) + "\" value=0.00 style=\"color:blue; \" size=\"3\"><span class=\"unit\" style=\"color:blue;\">ADC1</span></div>";
     html += "<div class=\"row\"><input type=\"none\" class=\"reads\" id=\"calA2SID\"  name=\"calA2S\" onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\" max=\""+ String(CALMAXTWEAK) +"\" min=\"" + String(-CALMAXTWEAK) + "\" value=0.00 style=\"color:blue; \" size=\"3\"><span class=\"unit\" style=\"color:blue;\">ADC2</span></div>";
    
   html += "</div>"; // col
   return html;
}
String calSave(void)
{
  String html = "";
  html += "<BR>";
  html += "<div id =\"DcalSave\" class=\"divcr\"><label id =\"LcalSave\" class=\"lpad\" >";
  html += "<input type=\"button\" id=\"calSave\" name=\"NcalSave\" class=\"btnc\" onclick=\"calSave(this)\"/><span id =\"ScalSave\" class=\"btnc\">Save</span></label></div><BR> "; // R1 but     

 return html;
}

// pre & post: number of digits before and after decimal
String knobBlock(uint8_t pre,uint8_t post, int8_t whichBut)
{ 
  int i;
	String html = "";
	html += "<div class=\"column\" id=\"dColID\" name=\"dCol\"  style=\"width:120px; background-color: #cce2ff;\" >";  
	//KNOB - attaches to last selected setting
	 html += "<div id=\"jog_dial\" style=\"height:110px; width:110px;\"></div>"; //class=\"dialX\" 
  //000.0 buttons	
  for (i = pre - 1; i >= 0; i--)
  {
    int mulNum = pow(10, i);
    html += "<input type=\"radio\" name=\"mulBut\" id=\"mulBut"+String(mulNum)+"\" value="+String(mulNum)+" size=1 ";
    if(whichBut == i)
      html += " checked";
     html += ">";
  }
  html += "<span style=\"font-weight: bold; font-size:24px;\">.</span>";
  for (i = 1; i <= post; i++)
  {
    float mulNum = pow(10, -i);
    html += "<input type=\"radio\" name=\"mulBut\" id=\"mulBut0"+String(i)+"\" value="+String(mulNum,i)+" size=1 ";
     if(whichBut == -i)
      html += " checked";
     html += ">";
  }

	// hidden calculation fields - remove from here and js file for production.
	html += "<input type=\"text\" hidden id=\"dialVal\" value=0 size=1> ";	// hide when not needed - code uses them
	 html += "<input type=\"text\" hidden id=\"deltaVal\" value=0 size=1>";	
	 html += "<input type=\"text\" hidden id=\"testVal\" value=0 size=1>";	
	 // error line for all inputs
	 html += "<input type=\"text\" maxlength=\"16\" class=\"errline\" readonly name=\"input_error\" id=\"input_errorID\" value =\" \" style=\"color: red;\" size=\"9\">\n"; 
	html += "</div>"; // col
	
    return html;
}
String sRangeButtonsXXX(void)
{
  String html = ""; 
  int whichBut = 2;
 html += "<span style=\"font-weight: bold; font-size:24px;\">.</span>";
  for (int i = 0; i < 4; i++)
  {
    float mulNum = pow(2, i);
    html += "<input type=\"radio\" name=\"SineS\" id=\"SineS"+String(mulNum)+"\" value="+String(mulNum)+" size=1 ";
     if(whichBut == i)
      html += " checked";
     html += ">";
  }
  return html;
}
String sRangeButtons(void)
{
	String html = ""; 

// Button names and IDs follow the 1,2 4, 8 values 
	html += "<div id =\"DAR1\" class=\"divr\"><label id =\"LAR1\" class=\"lpad\" >";
	html += "<input type=\"checkbox\" id=\"SSR1\" name=\"SSR[]\" class=\"btnr\"  onclick=\"clickDiv(this)\" value=1/><span id =\"SAR1\" class=\"btnzr\">Lo</span></label></div>"; 

  html += "<div id =\"DAR8\" class=\"divr\"><label id =\"LAR8\" class=\"lpad\" >";
  html += "<input type=\"checkbox\" id=\"SSR8\" name=\"SSR[]\" class=\"btnr\"  onclick=\"clickDiv(this)\" value=8/><span id =\"SAR8\" class=\"btnzr\">Hi</span></label></div>"; 
  return html;
}

#endif
