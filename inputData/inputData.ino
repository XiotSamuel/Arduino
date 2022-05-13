

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
//include JSON message library
#include <ArduinoJson.h>

//for the ESP WiFi.h
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

//for the EEPROM
#include "EEPROM.h"
#define EEPROM_SIZE 64
//for the EEPROM address
int addr = 0;
int addr_ssid;
int addr_pw;

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";

// HTML web page to handle 3 input fields (input1, input2)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    wifi ssid: <input type="text" name="ssid">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    password: <input type="text" name="password">
  </form>
</body></html>)rawliteral";

//gobal variable for the JSON doc wifi ssid and pw
//StaticJsonDocument<50> wifi_ssid_pw;

//protol type for the function
void writeString(int add,String data);
String read_String(int add);

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void checkROM(){
   Serial.print("new address in rom ");
   Serial.println(addr);
  
   int a=0;
   Serial.print("the value check ");
   while (a<64){
        Serial.print(char (EEPROM.read(a)));
        Serial.print(" ");
        a++;
      }
   Serial.println();
}

void writeString(int add,String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
  EEPROM.commit();
}


String read_String(int add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)   //Read until null character
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';

  checkROM();
  return String(data);
}

void setup() {
  Serial.begin(115200);
  //WiFi.mode(WIFI_AP);
  IPAddress IP(192, 168, 10, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(IP, IP, NMask);
  WiFi.softAP(ssid, password);
  //if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //  Serial.println("WiFi Failed!");
  //  return;
  //}
  Serial.println();
  //Serial.print("IP Address: ");
  //Serial.println(WiFi.localIP());
  //check for wifi ap ip
  //IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  //intiaize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    //for the ssid message
    String inputMessage1;
    String inputParam1;
    //for the pw message
    String inputMessage2;
    String inputParam2;

    //create a string for storage the value
    String wifi_ssid_pw;
    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      //input ssid message
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputParam1 = PARAM_INPUT_1;
      //input pw message
      //inputMessage2 = " ";
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;

      //create a JSON message for the ssid and pw
      wifi_ssid_pw += "{\"wifi_ssid_pw\":[";
      wifi_ssid_pw += inputMessage1;
      wifi_ssid_pw += ",";
      wifi_ssid_pw += inputMessage2;
      wifi_ssid_pw += "]}";
      //wifi_ssid_pw["wifi_ssid_pw"];
      //wifi_ssid_pw.add[inputMessage1];
      //wifi_ssid_pw.add[inputMessage2];
      
      // write to ROM
      writeString(addr, wifi_ssid_pw);


      Serial.print("the message write to EEPROM is ");
      Serial.println(wifi_ssid_pw);

      //read_String(addr);
      //for change the inputMessage to json
      

      Serial.print("the old addr is ");
      Serial.println(addr);

      
      
      //update the add
      addr=addr+wifi_ssid_pw.length();


      Serial.print("the new addr is ");
      Serial.println(addr);

      
      /*
      int a=0;
      char charBuff[inputMessage.length()+1];
      strcpy(charBuff, inputMessage.c_str());
      while(a<64){
      EEPROM.write(addr, charBuff[a]);
      EEPROM.commit();
      a++;
      }
      addr=addr+inputMessage.length();*/
      
      //checkROM();
      
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
    }
    
    //else {
    //  inputMessage1 = "No message sent";
    //  inputParam1 = "none";
    //}
    //Serial.print("The input message is ");
    //Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam1 + ") with value: " + wifi_ssid_pw +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
    // put your main code here, to run repeatedly:
  //String recivedData;
  //recivedData = read_String(10);
  //Serial.print("Read Data:");
  //Serial.println(recivedData);
  //delay(1000);
  
}
