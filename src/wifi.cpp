#include "wifi.h"

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
WiFiServer server(100);
WiFiClient client;
int status=WL_IDLE_STATUS;
void showstring(char* buffer,int countnum);
void WiFiInit(){
    Serial.println("Ready to connect...");
    WiFi.begin(ssid,pass);
    
    while (WiFi.status()!=WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
        WiFi.begin(ssid,pass);
    }
    Serial.println("wifi has connected");
    Serial.print("ssid:");
    Serial.println(WiFi.SSID());
    Serial.print("ip:");
    Serial.println(WiFi.localIP());
    server.begin();
    return;
}

String WiFiReceive(){
    char message;
    String mes = "";
    
    client=server.available();
    while(client.connected()){
        if(client.available()){
            message=client.read();
            
            //*(buffer+countnum)=message;
            if(message=='\n'){
                break;
            }
            mes+=message;
            
            // delayMicroseconds(1);
        }
    }
    return mes;
}