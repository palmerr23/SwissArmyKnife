#ifndef MYSCPI_RELAY_H
#define MYSCPI_RELAY_H

// one file for each device type

//uint8_t _relayChan = 0; // index value, SCPI value == +1
bool _relayVal[RELAYS] = {0,0};

void setRelay(int chan, int val)
{
    _relayVal[chan] = (val == 1) ? true : false;
    digitalWrite(relayPin[chan], val);  
    valChanged((chan == 0)? SET_R1 : SET_R2);
}
void RelaySet1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val;
  if (parameters.Size() == 0) 
    return;
  val = String(parameters[0]).toInt();
  setRelay(0, val);
}
void RelaySet2(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val;
  if (parameters.Size() == 0) 
    return;
  val = String(parameters[0]).toInt();
  setRelay(1, val);
}

void RelayGet1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println((_relayVal[0])? 1 : 0);
}
void RelayGet2(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println((_relayVal[1])? 1 : 0);
}

void SCPI_relay_setup()
{  
  for (int i = 0; i < RELAYS; i++) 
  {
    pinMode(relayPin[i], OUTPUT);  
    digitalWrite(relayPin[i], 0);
  }

  my_instrument.SetCommandTreeBase("SOURce");
    my_instrument.RegisterCommand(":R1", &RelaySet1);
    my_instrument.RegisterCommand(":R1?",&RelayGet1);
    
    my_instrument.RegisterCommand(":R2", &RelaySet2);
    my_instrument.RegisterCommand(":R2?",&RelayGet2);
}
void printRsettings(void)
{
   Serial.print("Relay: ");
  for(int i = 0; i < RELAYS; i++)
    Serial.printf("[%i] = %i, ",i,_relayVal[i]);
   //Serial.printf("OutChan = %i\n", _relayChan);
}
	
#endif
