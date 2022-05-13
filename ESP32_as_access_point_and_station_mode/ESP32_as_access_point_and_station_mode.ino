#include <WiFi.h>
#include <WiFiAP.h>

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* ssid = "Samuel";
const char* password = "Samuel@1986";

const char* assid = "espAccessPoint";
const char* asecret = "hello";

void setup(){
Serial.begin(115200);
WiFi.begin(ssid,password);
WiFi.mode(WIFI_AP_STA);

Serial.println("Creating Accesspoint");
WiFi.softAP(assid,asecret,7,0,5);
Serial.print("IP address:\t");
Serial.println(WiFi.softAPIP());

Serial.print("connecting to...");
Serial.println(ssid);

WiFi.begin(ssid,password);

while(WiFi.status() != WL_CONNECTED){
delay(500);
Serial.print(".");
}

Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());

}

void loop(){

}
