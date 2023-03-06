#ifndef MYSCPI_COMMS_H
#define MYSCPI_COMMS_H

void setSSID(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  strcpy(ssid, String(parameters[0]).c_str());  
  setProfile(false);
}
void setPass(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  strcpy(pass, String(parameters[0]).c_str()); 
  setProfile(false); 
}
void setHost(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  strcpy(hostname, String(parameters[0]).c_str());  
  setProfile(false);
}

void cgetSSID(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(ssid);
}

void getHost(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(hostname);
}

void wifiRestart(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  wifi_begin(ssid, pass);
}

void SCPI_comms_setup()
{  
  my_instrument.SetCommandTreeBase("SYSTem");
    my_instrument.RegisterCommand(":SSID", &setSSID);
    my_instrument.RegisterCommand(":PASSword",&setPass);
    my_instrument.RegisterCommand(":HOSTname",&setHost);
    my_instrument.RegisterCommand(":RESTart",&wifiRestart);
	
	my_instrument.RegisterCommand(":SSID?", &cgetSSID);
    // my_instrument.RegisterCommand(":PASSword?",&getPass); // not implemented: security
    my_instrument.RegisterCommand(":HOSTname?",&getHost);
}
#endif
