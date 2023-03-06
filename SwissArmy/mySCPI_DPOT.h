#ifndef MYSCPI_DPOT_H
#define MYSCPI_DPOT_H
#include "MCP45HVX1.h"
// one file for each device type
#define DPOTADDR 0x3C
MCP45HVX1 digiPot(DPOTADDR);
#define POTMAX 255
#define POTMIN 0

uint8_t _dPotVal = 0;
#define DPOTMAX 255
void setPot(int val)
{  
   _dPotVal = constrain(val, 0, DPOTMAX);
   digiPot.writeWiper(_dPotVal); 
   int tempX = digiPot.readWiper();
   //Serial.printf("dPot set %i, is %i\n",_dPotVal, tempX);  
}
void dPotSet(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
Serial.println("dPotSet");
 if (parameters.Size() == 0) 
    return;
 setPot(constrain(String(parameters[0]).toInt(), 0, DPOTMAX));
 valChanged(SET_POT);
}

void dPotGet(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{ //Serial.println("dPotGet");
  interface.println(_dPotVal);
}

void SCPI_dPot_setup()
{  
  digiPot.begin();  
  digiPot.startup();
  digiPot.connectWiper();
  digiPot.connectTerminalA();
  digiPot.connectTerminalB();
  
  digiPot.writeWiper(1);  
  if(digiPot.readWiper() != 1)
  {
    errorMsg("digiPot not found.\n", false);    
  }
  else 
    Serial.println("dPot found");

  my_instrument.SetCommandTreeBase("SOURce");
    my_instrument.RegisterCommand(":DPOT", &dPotSet);
    my_instrument.RegisterCommand(":DPOT?",&dPotGet);
}

void printPsettings(void)
{  
  Serial.printf("DPOT = %i (%i%%)\n",_dPotVal, _dPotVal*100/256);
}

#endif
