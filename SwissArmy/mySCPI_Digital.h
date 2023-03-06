#ifndef MYSCPI_DIGITAL_H
#define MYSCPI_DIGITAL_H

// one file for each device type
//  *********** OUTPUTS ****************
//uint8_t _dOutChan = 0; // index value, SCPI value == +1
bool _dOutVal[DOUT] = {0,0};
//uint8_t _dInChan = 0; // index value, SCPI value == +1
bool _dInVal[DIN] = {0,0};

void setDig(int chan, int val)
{
  //Serial.printf("setDig: [%i] = %i\n", chan, val); 
    _dOutVal[chan] = val;
    digitalWrite(dOutPin[chan], val);  
    valChanged((chan == 0)? SET_D1 : SET_D2);
}

void dOutSet1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  //Serial.println("dOutSet1");
  String param = String(parameters[0]);
  if (parameters.Size() == 0) 
    return;
 
  if (String(parameters[0]).toInt() == 1) 
    setDig(0,true);
  else
    setDig(0,false);
}

void dOutSet2(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  String param = String(parameters[0]);
   if (parameters.Size() == 0) 
    return;

  if (String(parameters[0]).toInt() == 1) 
    setDig(1,true);
  else
    setDig(1,false);  
}

void dOutGet1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
   //Serial.printf("dOutGet1 %i",_dOutVal[0]);
  interface.println((_dOutVal[0])? 1 : 0);
}

void dOutGet2(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println((_dOutVal[1])? 1 : 0);
}

//  *********** INPUTS ****************
int readDig(int chan)
{
  // read, set LED
  _dInVal[chan] = (digitalRead(dInPin[chan])) ? 1 : 0 ;
  digitalWrite(dInLED[chan], _dInVal[chan]);
  return  _dInVal[chan];
}

void dInMeas1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{    
  interface.println(readDig(0));
}
void dInMeas2(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{    
  interface.println(readDig(1));
}

void SCPI_digital_setup()
{  
  for (int i = 0; i < DOUT; i++) 
  {
    pinMode(dOutPin[i], OUTPUT);  
    digitalWrite(dOutPin[i], 0);
    // Din LEDs
    pinMode(dInLED[i], OUTPUT);  
    digitalWrite(dInLED[i], 0);

    pinMode(dInPin[i], INPUT_PULLDOWN); // INPUT_PULLDOWN allowed for ESP32
  }
   
  my_instrument.SetCommandTreeBase("SOURce"); 
    my_instrument.RegisterCommand(":D1",  &dOutSet1);
    my_instrument.RegisterCommand(":D1?", &dOutGet1);
    
    my_instrument.RegisterCommand(":D2", &dOutSet2);
    my_instrument.RegisterCommand(":D2?",&dOutGet2);
    
  my_instrument.SetCommandTreeBase("MEASure"); 
    my_instrument.RegisterCommand(":D1?",&dInMeas1);
    my_instrument.RegisterCommand(":D2?",&dInMeas2);

}
void printDreadings(void)
{
  Serial.print("Dig: ");
  for(int i = 0; i < DIN; i++)
    Serial.printf("[%i] = %i, ",i,_dInVal[i]);
 // Serial.printf("InChan = %i\n", _dInChan);

}
void printDsettings(void)
{
  Serial.print("Dig: ");
  for(int i = 0; i < DOUT; i++)
    Serial.printf("[%i] = %i, ",i,_dOutVal[i]);
  //Serial.printf("OutChan = %i\n", _dOutChan);
}

#endif
