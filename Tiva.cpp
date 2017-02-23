#include "Tiva.h"

#include <Energia.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <ArduinoHttpServer.h>
#include <string>
#include <AD9854.h>

using namespace std;

#define GET_NUMBER 2
#define POST_NUMBER 3

StaticJsonBuffer<40000> jsonBuffer;

char* prueba;

API::API(DDS* _DDS, IPAddress _ip, byte _mac[])
{
    _DDS_JRO=_DDS;
    IPAddress mydns(10, 10, 10, 1);
    IPAddress gateway(10, 10, 10, 1);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(_mac, _ip, mydns, gateway, subnet);   
}

int API::readcommand( EthernetClient client)
{
	int msg=0;
    
    if (client.connected())
    {
        
        ArduinoHttpServer::StreamHttpRequest<50000> httpRequest(client);
        ArduinoHttpServer::StreamHttpReply httpReply(client, "application/json");
        

        if(_DDS_JRO->verifyconnection())
        {
            if (httpRequest.readRequest())
            {
                char* data = (char*) httpRequest.getBody();
                int data_length = (int) httpRequest.getContentLength();
                ArduinoHttpServer::MethodEnum method( ArduinoHttpServer::MethodInvalid );
                method = httpRequest.getMethod();
                
                if ( method == ArduinoHttpServer::MethodGet)
                {
                    /*******************************************************
                    READ
                    *****************************************************/
                    if (httpRequest.getResource()[0] ==  "read")
                    {
                        if (httpRequest.getResource()[1] ==  "11")
                        {
                            httpReply.send("{\"frequency1\":\"ok\"}");
                        }

                    }
                    /*******************************************************
                    STATUS
                    *****************************************************/
                    else if (httpRequest.getResource()[0] == "status")
                    {
                    	JsonObject&  dds_status = jsonBuffer.createObject();
                        dds_status["Conection"] = "YES";
                        dds_status["Clock"]= _DDS_JRO->getclock() ;
                        dds_status["Frequency1_out"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency1())*_DDS_JRO->getMultiplier();
                        dds_status["Frequency2_out"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency2())*_DDS_JRO->getMultiplier();
                        dds_status["Multiplier"] =String(_DDS_JRO->getMultiplier());
                        dds_status["Frequency1_reg"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency1());
                        dds_status["Frequency2_reg"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency2());
                        dds_status["phaseA"] =_DDS_JRO->rdPhase1();
                        dds_status["phaseB"] =_DDS_JRO->rdPhase2();
                        dds_status["amplitudeI"]=_DDS_JRO->rdAmplitudeI();
                        dds_status["amplitudeQ"]=_DDS_JRO->rdAmplitudeQ();


                        dds_status["Clock"] =_DDS_JRO->getclock();

                        String hola1;
                        dds_status.printTo(hola1);
                        const String& msg_json = hola1;
                        httpReply.send(msg_json);
                        msg=2;
                    }
                    else
                    {
                        msg=3;
                    }
                }
                else if ( method == ArduinoHttpServer::MethodPost)
                {
                    /*******************************************************
                    WRITE
                    *****************************************************/
                    if (httpRequest.getResource()[0] == "write")
                    {
                        JsonObject& jsondata = jsonBuffer.parseObject(data);
                        
                        Serial.println("------------------------------------");
                        double freq_1 = double(jsondata["frequency1"]);
                        Serial.println(String(freq_1));
                        freq_1/=float(_DDS_JRO->getMultiplier());
                        
                        if(freq_1<=_DDS_JRO->getclock()/2)// && freq_1>=_DDS_JRO->getclock()/250)
                        {                   
                            Serial.println(freq_1);
                            char* fb=_DDS_JRO->freq2binary(freq_1);
                            _DDS_JRO->wrFrequency1(fb);
                            double fre_out = _DDS_JRO->binary2freq(_DDS_JRO->rdFrequency1())*_DDS_JRO->getMultiplier();
                            
                            fb=_DDS_JRO->rdFrequency1();
                            Serial.print("Decimal read: ");
                            double decimal = _DDS_JRO->binary2decimal(fb);
                            Serial.println(String(decimal));

                            JsonObject&  dds_status = jsonBuffer.createObject();
                            dds_status["Conection"] = "YES";
                            dds_status["Clock"]= _DDS_JRO->getclock() ;
                            dds_status["Frequency1"] = String(fre_out);// double (_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency1())*_DDS_JRO->getMultiplier());
                            dds_status["Frequency2"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency2())*_DDS_JRO->getMultiplier();
                            dds_status["Multiplier"] =double(_DDS_JRO->getMultiplier());
                            dds_status["Clock"] =_DDS_JRO->getclock();

                            

                            Serial.println(fre_out);
                            String hola1;
                            dds_status.printTo(hola1);
                            const String& msg_json = hola1;
                            httpReply.send(msg_json);
                            msg=1;

                        }
                        
                        else{

                            JsonObject&  dds_error_1 = jsonBuffer.createObject();
                            dds_error_1["Conection"] = "Configuration error";
                            dds_error_1["Msg"]= "Frecuency set is out of range, please change value (Mhz)" ;

                            String hola1;
                            dds_error_1.printTo(hola1);
                            const String& msg_json = hola1;
                            httpReply.send(msg_json);
                            msg=1;
                        }
                        msg=4;
                    }
                    /*******************************************************
                    START
                    *****************************************************/
                    else if (httpRequest.getResource()[0] == "start")
                    {
                        httpReply.send("{\"start\":\"ok\"}");
                        msg=5;
                    }
                    /*******************************************************
                    STOP
                    *****************************************************/
                    else if (httpRequest.getResource()[0] == "stop")
                    {
                        _DDS_JRO->reset();
                        httpReply.send("{\"stop\":\"ok\"}");
                        msg=6;
                    }


                    else
                    {
                        httpReply.send("{\"wrong_post\":\"ok\"}");
                        msg=7;
                    }
                    
                }
                
            }
            else
            {
                
                ArduinoHttpServer::StreamHttpErrorReply httpReply(client, httpRequest.getContentType());
                httpReply.send(httpRequest.getErrorDescrition());
            }
        }

        else
        {
            httpReply.send("{\"wrong\":\"ok\"}");
        }
    
    }
    return msg;
    client.stop();
}

/*

{
  "amplitude_enabled": false, 
  "frequencyB_Mhz": 0, 
  "frequencyA": 29320310074026, 
  "phaseA_degrees": 0.0, 
  "clock": 60.0, 
  "modulation": 4, 
  "frequencyB": 0, 
  "phaseA": 0, 
  "phaseB": 0, 
  "multiplier": 4, 
  "device_id": 1, 
  "amplitudeQ": 0, 
  "phaseB_degrees": 0, 
  "amplitudeI": 0, 
  "frequencyA_Mhz": 24.99999999999943
}
*/