#ifndef MYSCPI_ANALOG_H
#define MYSCPI_ANALOG_H
#include <ADS1X15.h>  // https://github.com/RobTillaart/ADS1X15
// one file for each device type
#define ADS1115_ADDRESS  0x48
ADS1115 ads;  
void printCal(void);
bool setProfile(bool create);
void aOut(float value);

#define DACSTEPS 255
#define DACMAX 255
#define DACCAL 245
#define DACMINCAL 200
#define DACMINCALV (VMAXOP * 0.8)
#define DACHALFSTEP (0.02)  // ~40mV for a full step

// ADS1015 only counts to 32752 (32768 - 16)
#define OVERSAMPL 16         // oversampling
#define ACOUNT_MAX 32750
#define ACOUNT_CAL ACOUNT_MAX/2 // counts will be half if other ADS terminal at V/2
#define ACOUNT_OFLOW 32751
#define ADCMINCAL 10000
#define ADCMINCALV (VMAXOP * 0.8)

//#define VMAXOP (VMAXOP)   
//#define AMAXVL (10.0/8)

#define AMAXVCOUNTS  32750 // only 15 bits positive.
//#define AINHI 1   // high range  10V (2.048V PGA)
//#define AINLO 0   // low range 1.25V = 10V/8 (256mv PGA)

//DAC
//uint8_t _aOutChan = 0; // index value, SCPI value == +1
float _aOutVal = 0;
uint8_t dacVal = 0;

// cal change - temp values. Updated as value DACtempCalOff changes (web interface). 
float   DACtempCalS;   // set value 
float   DACtempCalOff;   // offset value set from Cal screen
uint8_t DACtempCalC; // actual values in counts
// cal value at which VMAXOP is reached. 
uint8_t DACcalVal = DACCAL; // Counts for full scale volts. Udated by setCal, saved in Profile.
 
// ADC
//uint8_t _aInChan = 0; // index value, SCPI value == +1
float _ainVal[AIN] = {0,0};
int16_t _ainCounts[AIN] = {0,0};
int _ainCountbuf[AIN][OVERSAMPL];
int bufIndx =0;
//float _aInVal[AIN] = {0,0};
//uint8_t aInRange[AIN] = {1,1};  // high range to start

float   ADCtempCalR[AIN];   //  value read
float   ADCtempCalOff[AIN];   // offset value set from Cal screen
int16_t ADCtempCalC[AIN]; // actual values in counts 
int16_t ADCcalVal[AIN] = {ACOUNT_CAL, ACOUNT_CAL}; // Counts for full scale volts.

// DAC
uint16_t aVoltsToCounts(float volts)
{  
  // half step added to change integer conversion from floor() to round()
  return constrain((volts + DACHALFSTEP)* DACcalVal / VMAXOP, 0, DACMAX);
}

// assumes accurate calibration and no DC offset
float DACcountsToVolts(int counts)
{ 
  float tempV;
  tempV = (VMAXOP * counts) / DACcalVal; // force float arithmetic
  return tempV;  
}

void setCal(void)
{
  Serial.printf("******* setCal()\n");
  // calculate the counts for ADC/DAC VMAXOP 
  int i = 0;
  Serial.printf("DAC S = %2.3f, Off = %1.3f, Counts = %i\n", DACtempCalS, DACtempCalOff, DACtempCalC);
  if(abs(DACtempCalOff) > SMALLBIT && DACtempCalS >= DACMINCALV) // only change if non-zero offset and high enough value
  {
    // float map()
    DACcalVal = mapf(VMAXOP, 0, DACtempCalS + DACtempCalOff, 0, DACtempCalC);
    Serial.printf("DAC[%i] to %i\n",  i, DACcalVal);      
  }
  DACcalVal = constrain(DACcalVal, DACMINCAL, DACMAX); // make sure current value is legal, changed or not
  aOut(_aOutVal);  // update the DAC to the value
    
  for(i = 0; i < AIN ; i++)
  {   
    Serial.printf("ADC[%i] R = %2.3f, Off = %1.3f, Counts = %i\n", i, ADCtempCalR[i], ADCtempCalOff[i], ADCtempCalC[i]);     
    if(abs(ADCtempCalOff[i]) > SMALLBIT &&  ADCtempCalR[i] >= ADCMINCALV)
    {
      ADCcalVal[i] = mapf(VMAXOP, 0, ADCtempCalR[i] + ADCtempCalOff[i], 0, ADCtempCalC[i]); 
      Serial.printf("ADC[%i] to %i\n",  i, ADCcalVal[i]);
    }
    ADCcalVal[i] = constrain(ADCcalVal[i], ADCMINCAL, ACOUNT_MAX);
  }
  printCal();
  setProfile(false); // save the values
}

//  *********** OUTPUTS ****************
// DACs: val in volts
void setDAC(float val)
{
    uint8_t dVal;
    val = constrain(val, 0.0, VMAXOP);   
    dVal = aVoltsToCounts(val);  // nearest step 
    _aOutVal = DACcountsToVolts(dVal); // actual value from step
    dacVal = dVal;   
    dacWrite(dacPin, dacVal);    
}

void aOut(float value)
{
  float val = 0;
Serial.printf("aOUT val %fV\n", value);
    val = constrain(value, 0.0, VMAXOP);      
    //_aOutVal = val;
    setDAC(val);        
}

void aOutSetA1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
    Serial.println("SOUR:A1");
   if (parameters.Size() > 0)   
   {
     aOut(String(parameters[0]).toFloat());
     valChanged(SET_A1);
   }
}

void aOutGetA1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.printf("%3.3f\n",_aOutVal);
}

//  *********** INPUTS ****************
// CH1 is -10 to +10V, CH2 is 0 to +10V
float countsToVolts(uint8_t chan)
{
  float volts;
  if (_ainCounts[chan] <= -ACOUNT_OFLOW) return -999;
  if (_ainCounts[chan] >=  ACOUNT_OFLOW) return 999;
    
  volts =  VMAXOP * _ainCounts[chan] / ADCcalVal[chan];
  return volts;
}

// read next value in circular buffer and calculate mean
float readADCx(int chan)
{
     int adc = 0; 
     ads.setGain(2);        // 2x gain   +/- 2.048V  1 bit = 1mV   0.0625mV

  if(chan == 0)
   _ainCountbuf[chan][bufIndx] = ads.readADC_Differential_0_1();
  //  adc = _ainCounts[chan] = ads.readADC_Differential_0_1();
  else
   // adc = _ainCounts[chan] = ads.readADC_Differential_2_3();
    _ainCountbuf[chan][bufIndx] = ads.readADC_Differential_2_3();

  if (chan == 0)  // move t next place in circular buffer every time we sample chan 0
    bufIndx = ++bufIndx % OVERSAMPL;
    for (int i = 0; i < OVERSAMPL; i++)
      adc += _ainCountbuf[chan][i];
   _ainCounts[chan] = (adc + OVERSAMPL/2)/OVERSAMPL; // FLOOR to ROUND
   return _ainVal[chan] = countsToVolts(chan);
}

void aInMeasA1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{  
   interface.printf("%3.3f\n",_ainVal[0]);
}
void aInMeasA2(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{  
   interface.printf("%3.3f\n",_ainVal[1]);
}

void SCPI_analog_setup()
{  
  // DAC
    dacWrite(dacPin, 0); // initialise to 0
    // DAC2 is sine function

  my_instrument.SetCommandTreeBase("SOURce"); 
    my_instrument.RegisterCommand(":A1", &aOutSetA1);
    my_instrument.RegisterCommand(":A1?",&aOutGetA1);

  // ADS ADC
  if (!ads.begin()) 
  {
    errorMsg("ADS1115 not found.\n", false);
    return; // don't register commands
  }
  else 
    Serial.println("ADC found");
  // will read ADC one-shot as needed after setting PGA.   
  my_instrument.SetCommandTreeBase("MEASure"); 
    my_instrument.RegisterCommand(":A1?",&aInMeasA1); 
    my_instrument.RegisterCommand(":A2?",&aInMeasA2); 
}

void printAreadings(bool printBuf)
{
  Serial.print("Ain: ");
  for(int i = 0; i < AIN; i++)
  {
    Serial.printf("[%i] = %2.3f [%i]; ",i, _ainVal[i], _ainCounts[i]);
    // Serial.printf("InChan = %i\n", _aInChan);
    if(printBuf)
    {
      Serial.print(" Buf: ");
      for(int j = 0; j < OVERSAMPL; j++)
          Serial.printf("%i, ", _ainCountbuf[i][j]);
      Serial.println();
    } 
  }
}
void printAsettings(void)
{
    Serial.printf("Aout: set = %2.3fV  [%i], actual = %2.3f; ",_aOutVal, dacVal, DACcountsToVolts(dacVal));
}

void printCal(void)
{
  Serial.print("Cal values:\n");
  Serial.printf("DAC = %i\n",DACcalVal);
  for(int i = 0; i < AIN; i++)
    Serial.printf("ADC[%i] = %i\n",i, ADCcalVal[i]);
}

#endif
