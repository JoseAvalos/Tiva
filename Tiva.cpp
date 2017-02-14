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

StaticJsonBuffer<200> jsonBuffer;

API::API(DDS* _DDS)
{
    _DDS_JRO=_DDS;
    byte mac[] = {0x00, 0x1A, 0xB6, 0x02, 0xEC, 0x3E };
    IPAddress ip(10, 10, 50, 159);
    IPAddress mydns(10, 10, 10, 1);
    IPAddress gateway(10, 10, 10, 1);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(mac, ip, mydns, gateway, subnet);
    
    
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
                    msg=1;
                }
                /*******************************************************
                STATUS
                *****************************************************/
                else if (httpRequest.getResource()[0] == "status")
                {
					JsonObject&  dds_status = jsonBuffer.createObject();
					int state= _DDS_JRO->init();
					dds_status["Coneccion"] = String(state);
					dds_status["Frecuencia1"] = 1351824120;
								
                    String hola1 = "";
                    dds_status.printTo(hola1);
                    const String& msg_json = hola1;
                    httpReply.send(msg_json);              	
                	msg=2;
                }
                else
                {
                    msg=3;
                    //httpReply.send("{"msg":"wrong command"}");
                }
            }
            else if ( method == ArduinoHttpServer::MethodPost)
            {
                /*******************************************************
                WRITE
                *****************************************************/
                if (httpRequest.getResource()[0] == "write")
                {
                    msg=4;
                    // httpReply.send("{"write":"ok"}n");
                    // StaticJsonBuffer<400> jsonBuffer;
                    // JsonObject& jsondata = jsonBuffer.parseObject(data);
                    // const char* username = jsondata["username"];
                    // Serial.println(username);
                }
                /*******************************************************
                START
                *****************************************************/
                else if (httpRequest.getResource()[0] == "start")
                {
                    msg=5;
                    //httpReply.send("{"start":"ok"}");
                }
                /*******************************************************
                STOP
                *****************************************************/
                else if (httpRequest.getResource()[0] == "stop")
                {
                    msg=6;
                    //httpReply.send("{"stop":"ok"}");
                }
                else
                {
                    msg=7;
                    //ArduinoHttpServer::StreamHttpErrorReply httpReply(client, httpRequest.getContentType());
                    //httpReply.send(httpRequest.getErrorDescrition());
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