
#include <WiFi.h>
#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);

//EEPROM
#include "EEPROM.h"
// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;
#define EEPROM_SIZE 128


// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid_ap = "ESP32";
const char* password_ap = "ESP32";
const char* ssid = "Samuel";
const char* password = "Samuel@1986";
const char* PARAM_1 = "ssid";
const char* PARAM_2 = "password";
const String inputParam1 = "ssid";
const String inputParam2 = "password";

//WiFiServer wifi_server(80);

// HTML web page to handle 2 input fields (input1, input2)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
  <html lang="en">
    <head>
      <meta charset="UTF-8">
      
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      
      <title>ESP32 Form</title>
      
      <style>
      body{ margin: 0;padding: 0;font-family: Arial, Helvetica, sans-serif;background-color: #2c257a;}
      .box{ width: 70%; padding: 40px; position: absolute; top: 50%; left: 50%; transform: translate(-50%,-50%); background-color: #191919; color: white; text-align: center; border-radius: 24px; box-shadow: 0px 1px 32px 0px rgba(0,227,197,0.59);}
      h1{ text-transform: uppercase; font-weight: 500;}
      input{ border: 0; display: block; background: none; margin: 20px auto; text-align: center; border: 2px solid #4834d4; padding: 14px 10px; width: 45%; outline: none; border-radius: 24px; color: white; font-size: smaller; transition: 0.3s;}
      input:focus{ width: 90%; border-color:#22a6b3 ;}
      input[type='submit']{ border: 0; display: block; background: none; margin: 20px auto; text-align: center; border: 2px solid #22a6b3; padding: 14px 10px; width: 140px; outline: none; border-radius: 24px; color: white; transition: 0.3s; cursor: pointer;}
      input[type='submit']:hover{ background-color: #22a6b3;}
      </style>
    
    </head>
  
    <body>
      <form action="/get" class="box" id="my-form">
        <h1>Register</h1>
        <div class="part">
        <input name="ssid" type="text" placeholder="ssid">
        </div>
        <div class="part">
        <input name="password" type="text"  placeholder="password">
        </div>
        <input type="submit" value="Register">
      </form>
    </body>
  </html>

)rawliteral";

void writeString(int add,String data);
String read_String(int add);


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


//String read_String(int add)
//{
  //int i;
  //char data[100]; //Max 100 Bytes
  //int len=0;
  //unsigned char k;
  //k=EEPROM.read(add);
  //while(k != '\0' && len<500)   //Read until null character
  //{    
  //  k=EEPROM.read(add+len);
  //  data[len]=k;
  //  len++;
  //}
  //data[len]='\0';

  //Serial.println(" bytes written on Flash . Values are:");
  //for (int i = 0; i < EEPROM_SIZE; i++)
  //  {
  //    Serial.print(char(EEPROM.read(i))); 
      //Serial.print(" ");
  //  }
  //Serial.println(); Serial.println("----------------------------------");

  
  //Serial.print("the address is ");
  //Serial.println(add);
  
  //return String(data);
//}

void notFound(AsyncWebServerRequest *request) { 
  request->send(404, "text/plain", "Not found");
}

IPAddress local_IP(192,168,10,1);
IPAddress gateway(192,168,10,2);
IPAddress subnet(255,255,255,0);

void setup() {
  Serial.begin(115200);

  //init EEPROM
  EEPROM.begin(64);
  
  //initialize wifi mode in AP
  //WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  Serial.print("Setting AP *");
  //WiFi.mode(WIFI_MODE_AP);
  //WiFi.begin(ssid, password);
  delay(250);
  WiFi.mode(WIFI_AP); //Set mode of the Wifi
  delay(250);
  boolean result = WiFi.softAP("ESP32","12345678");

  if(result == true)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }

  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  
  //if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //  Serial.println("WiFi Failed!");
  //  return;
  //}
  //WiFi.setAutoReconnect(false);
  //Serial.println();
  //Serial.print("IP Address: ");
  //Serial.println(WiFi.localIP());
  
// Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1,inputMessage2;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->getParam(PARAM_1)->value() != "") {
      inputMessage1 = request->getParam(PARAM_1)->value();
    } else {
      inputMessage1 = "none";
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    if (request->getParam(PARAM_2)->value() != "") {
      inputMessage2 = request->getParam(PARAM_2)->value();
    } else {
      inputMessage2 = "none";
    }

    Serial.println(inputParam1 + ": " + inputMessage1);
    Serial.println(inputParam2 + ": " + inputMessage2);
  
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" + inputParam1 + ") with value: " + inputMessage1 + ", (" + inputParam2 + ") with value: " + inputMessage2 + ", (" + "<br><a href=\"/\">Return to Home Page</a>"); 
    //WiFi.mode(WIFI_AP);
    //WiFi.begin(inputMessage1.c_str(), inputMessage2.c_str());

   String login_save;

   login_save += "{\"login_save\":[";
   login_save += "{";
   login_save += "\"ssid\":\"";
   login_save += inputMessage1;
   login_save += "\"},";
   login_save += "{";
   login_save += "\"password\":\"";
   login_save += inputMessage2;   
   login_save += "\"}";
   login_save += "]}";
  
    writeString(addr, login_save);
    //read_String(addr);


    //writeString(addr+inputMessage1.length()+1, inputMessage2);
    
    Serial.println(" bytes written on Flash . Values are:");
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
      Serial.print(char(EEPROM.read(i))); 
      Serial.print(" ");
    }
    Serial.println(); Serial.println("----------------------------------");

    //ESP.restart();
    //WiFi.mode(WIFI_AP);
    //IPAddress default_ip(0, 0, 0, 0);
    //IPAddress default_DNS(10, 0, 1, 2);
    //IPAddress default_GW(255, 255, 255, 0);
    //WiFi.softAPdisconnect (true);   
    WiFi.disconnect();
    //WiFi.config(default_ip, default_ip, default_ip);
    //WiFi.config(default_ip);
    //WiFi.config(default_ip, default_DNS, default_GW);
    //WiFi.config(); need to have ip
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(inputMessage1.c_str(),inputMessage2.c_str());
    delay(300);
    
    //IPAddress ipTemp;
    IPAddress ipTemp;
    ipTemp = WiFi.localIP();
    Serial.print("New IP Address: ");
    Serial.println(ipTemp);
    
    });

  server.onNotFound(notFound);
  server.begin();
}


void loop() {

  //WiFiClient client = server.available();   // listen for incoming clients

      // close the connection:
    //client.stop();
    //Serial.println("Client Disconnected.");
  //Serial.print("inputParam1 ");
  //Serial.println(inputParam1);
  //Serial.print("inputParam2 ");
  //Serial.println(inputParam2);
  }
