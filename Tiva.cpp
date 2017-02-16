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
                    JsonObject&  dds_status = jsonBuffer.createObject();
                    dds_status["Conection"] = "YES";
                    dds_status["Clock"]= _DDS_JRO->getclock() ;
                    dds_status["Frequency1"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency1())*_DDS_JRO->getMultiplier();
                    dds_status["Frequency2"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency2())*_DDS_JRO->getMultiplier();
                    dds_status["Multiplier"] =double(_DDS_JRO->getMultiplier());
                    dds_status["Clock"] =_DDS_JRO->getclock();

                    String hola1;
                    dds_status.printTo(hola1);
                    const String& msg_json = hola1;
                    httpReply.send(msg_json);
                    msg=1;

                }
                /*******************************************************
                STATUS
                *****************************************************/
                else if (httpRequest.getResource()[0] == "status")
                {
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
                    double freq_1 =double(jsondata["frequency1"]);
                    freq_1*=1000000; 
                    freq_1/=_DDS_JRO->getMultiplier();
                    
                    if(freq_1<=_DDS_JRO->getclock()/2)// && freq_1>=_DDS_JRO->getclock()/250)
                    {                   
                        Serial.println(freq_1);
                        char* x=(_DDS_JRO->freq2binary(freq_1));
                        _DDS_JRO->wrFrequency1(x);
                        

                        JsonObject&  dds_status = jsonBuffer.createObject();
                        dds_status["Conection"] = "YES";
                        dds_status["Clock"]= _DDS_JRO->getclock() ;
                        dds_status["Frequency1"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency1())*_DDS_JRO->getMultiplier();
                        dds_status["Frequency2"] =_DDS_JRO->binary2freq(_DDS_JRO->rdFrequency2())*_DDS_JRO->getMultiplier();
                        dds_status["Multiplier"] =double(_DDS_JRO->getMultiplier());
                        dds_status["Clock"] =_DDS_JRO->getclock();

                    String hola1;
                    dds_status.printTo(hola1);
                    const String& msg_json = hola1;
                    httpReply.send(msg_json);
                    msg=1;

                    }
                    
                    else{

                        httpReply.send("{\"frequency1\":\"out of range\"}");
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
                    httpReply.send("{\"stop\":\"ok\"}");
                    msg=6;
                }
                else
                {
                    httpReply.send("{\"wrong\":\"ok\"}");
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
    return msg;
    client.stop();
}