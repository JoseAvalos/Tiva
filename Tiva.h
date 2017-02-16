#ifndef Tiva_h
#define Tiva_h

#include "Energia.h"
#include <ArduinoHttpServer.h>
#include <Ethernet.h>
#include <AD9854.h>

class API 
{
  public:
  	API(DDS*,IPAddress, byte[]);
  	int readcommand(EthernetClient);
  private:  
  	DDS* _DDS_JRO;
};

#endif
