#ifndef MYSCPI_SINE_H
#define MYSCPI_SINE_H
/* DAC Sine Generator   */

void printSsettings();
 
#include "driver/dac.h"
#include <driver/dac_common.h>
#include "esp_err.h"

#define SIN_CHAN DAC_CHANNEL_2            // using DAC2 = 1; for production
#define DACPINX 26                       // Pin 26 for production
// these defaults should be left alone
#define SIN_OFFSET 1     // leave this alone - there's a problem with zero
#define SIN_SCALE 2   // default is 1/2 scale
#define SIN_FREQ_DEF 500
#define SIN_PHASE DAC_CW_PHASE_0 
#define FMIN 130
#define FMAX 55000

#define NLEVELS 4
#define SCALEMIN 1
#define SCALEMAX 8
// index is the user scale [0..3]
struct cwScale {
  uint8_t scaleDiv;  // 1,2,4,8
  dac_cw_scale_t scaleSet;  // 0..3
  int8_t offset;
}
cw_scales[NLEVELS] = {{8,(dac_cw_scale_t)0,1},  // problematic - offset of zero is broken
                      {4,(dac_cw_scale_t)1,1},
                      {2,(dac_cw_scale_t)2,1},
                      {1,(dac_cw_scale_t)3,1}};
// only freqency and scale should be changed be code

uint8_t cw_scale = SIN_SCALE;
dac_cw_config_t sineConf = {(dac_channel_t)SIN_CHAN, (dac_cw_scale_t)SIN_SCALE, (dac_cw_phase_t)SIN_PHASE, SIN_FREQ_DEF, (int8_t)SIN_OFFSET};

// user levels are divide by 1,2,4,8
void setSineScale(uint8_t uScale)
{  
  int scaleNum = 0 ; // default to lowest output  
  for (int i = 0; i < NLEVELS; i++)  
    if(cw_scales[i].scaleDiv == uScale)
    {    
      scaleNum = i;
      break;
    }
  Serial.printf("SS scale in %i, set %i\n", uScale, scaleNum);
  cw_scale = scaleNum; // save for queries
  sineConf.scale = cw_scales[cw_scale].scaleSet;
  sineConf.offset = cw_scales[cw_scale].offset;
  dac_cw_generator_config(&sineConf);  
}

void setSineFreq(int freq)
{ int tempI;
float tempF;
  esp_err_t succ;
  freq = constrain(freq, FMIN, FMAX);
  //sineConf.freq = freq;
  // Discrete frequencies are multiples of 133.33 Hx
  // HAL software thinks they are 130 Hz steps.
  tempI = freq / 130;
  tempF = 133.33333 * tempI;
  sineConf.freq = tempF;
  succ = dac_cw_generator_config(&sineConf); 
  valChanged(SET_SF1); // as actual value is not the same as the setting, send update even when Web changes value.   
}

void setSF1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val;
  
   if (parameters.Size() > 0)   
   {
     val = String(parameters[0]).toInt();
     val = constrain(val, FMIN, FMAX);      
     setSineFreq(val); 
   }    
   Serial.printf("SOUR:SF1 %i\n", val);
}

void setSS1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val;
  Serial.printf("SOUR:SS1 %i params, ", parameters.Size());
   if (parameters.Size() > 0)   
   {
     val = String(parameters[0]).toInt();
     Serial.printf("val = %i \n", val);
     val = constrain(val, 1, 8);      
     setSineScale(val);  
     valChanged(SET_SS1);       
   }    
}

void getSF1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(sineConf.freq);
}

void getSS1(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(cw_scales[sineConf.scale].scaleDiv);
}

void SCPI_sine_setup()
{
   dac_cw_generator_config(&sineConf);
   dac_cw_generator_enable();
   dac_output_enable(SIN_CHAN);
   setSineFreq(SIN_FREQ_DEF); // cause the value to be sent to web
#ifdef PRINT_DIAGNOSTICS
   printSsettings();
#endif
    
   my_instrument.SetCommandTreeBase("SOURce"); 
     my_instrument.RegisterCommand(":SF", &setSF1);
     my_instrument.RegisterCommand(":SF?",&getSF1);
     my_instrument.RegisterCommand(":SS", &setSS1);
     my_instrument.RegisterCommand(":SS?",&getSS1);
}

void printSsettings()
{
  Serial.printf("Sine: Chan =: %i, ", sineConf.en_ch);
  Serial.printf("Scale = %i, ", sineConf.scale);
  Serial.printf("Freq = %i, ",sineConf.freq);
  Serial.printf("Offset = %i\n",sineConf.offset);  
}
#endif
