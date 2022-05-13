//************************************************************
//This is the test script for the wifi and ap switching without wifi manager
//add storage function 19July2021
//************************************************************


#include "IPAddress.h"
#include "painlessMesh.h"
#include "Preferences.h"

#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

//painlessMesh parameter
#define   MESH_PREFIX     "NetMesh"
#define   MESH_PASSWORD   "NetMesh2021"
#define   MESH_PORT       5555

//hostname setting
#define HOSTNAME "MeshNode"

//pin setting
#define TRIGGER_PIN 39

//for storage in flash
Preferences preferences;

// Prototype
void receivedCallback( const uint32_t &from, const String &msg );

painlessMesh  mesh;
AsyncWebServer server(80);
IPAddress myIP(0, 0, 0, 0);
IPAddress myAPIP(0, 0, 0, 0);

//for perfernce wifi
const char* strSSID;
const char* strpassword;

//HTML setup web site
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<form action="/index" method=get name="Index">INDEX
<br><input type='radio' name='index' value='1'> SETUP WIFI <br/>
<br><input type='radio' name='index' value='2'> SEND MESSAGE<br/>
<br><input type='submit' value='Submit'><br/>
</form>
</html>
)rawliteral";

//HTML setting WIFI
const char settingWIFI[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<form action="/submit" method=get name="SetupWifi">Enter SSID and Password
<br><input type='text' name='SSID'><br/>
<br><input type='text' name='password'><br/>
<br><input type='radio' name='customfieldid' value='1'> root <br/>
<br><input type='radio' name='customfieldid' value='2'> normal<br/>
<br><input type='submit' value='Submit'><br/>
</form>
</html>
)rawliteral";

//HTML set up ok
const char setting_ok[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<h1>
Form submitted
</h1>
</html>
)rawliteral";

//HTML send message page
const char send_message[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<form action="/sendMessage" method=get name="SendMessage">Enter Message here
<br><input type='text' name='Message'><br/>
<br><input type='submit' value='Submit'><br/>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT);
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);
  mesh.setHostname(HOSTNAME);
  
  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());



  preferences.begin("credentials", false);

  //get the wifi setting in flash
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
   preferences.end();
  //check if have old wifi setting
  if (ssid == "" || password == "")
  {
    Serial.println("no wifi set");
  }
  else
  {
    Serial.print("old wifi setting found:: ssid: ");
    Serial.print(ssid);
    Serial.print("password: ");
    Serial.println(password);
    mesh.stationManual(ssid, password);
    mesh.setHostname(HOSTNAME);
  //myIP = IPAddress(mesh.getStationIP());
  //Serial.println("My station IP is " + myIP.toString());
 
  }
  
  //Web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/index", HTTP_GET, [](AsyncWebServerRequest * request) {
       if(request->arg("index")=="1"){
          request->send(200, "text/html", settingWIFI);
      }else if (request->arg("index")=="2"){
          request->send(200, "text/html", send_message);
      }else {
        Serial.println("index page selection error");
      }
  });

  server.on("/sendMessage", HTTP_GET, [](AsyncWebServerRequest * request){
    if (request->hasArg("Message")) {
      String strMessage = request->arg("Message");
      Serial.print("user input Message: ");
      Serial.println(strMessage);
      mesh.sendBroadcast(strMessage);
      request->send(200, "text/html", send_message);
    }
  });
  
  server.on("/submit", HTTP_GET, [](AsyncWebServerRequest * request) {

    if (request->hasArg("SSID")) {

      strSSID = request->arg("SSID").c_str();
      Serial.print("user input SSID: ");
      Serial.println(strSSID);
    }
    if (request->hasArg("password")) {
      strpassword = request->arg("password").c_str();
      Serial.print("user input password: ");
      Serial.println(strpassword);
    }
    if (request->hasArg("customfieldid")){
      Serial.print("the radio form input: ");
      Serial.println(request->arg("customfieldid"));
      
      if(request->arg("customfieldid")=="1"){
        mesh.setRoot(true);
        mesh.setContainsRoot(true);
      }else if (request->arg("customfieldid")=="2"){
        mesh.setRoot(false);
        mesh.setContainsRoot(false);
      }else {
        Serial.println("radio form customfieldid input error");
      }
    }
    //store wifi cerdentials
    preferences.begin("credentials", false);
    preferences.putString("ssid", strSSID);
    preferences.putString("password", strpassword);
   preferences.end();
    //set the mesh to use the wifi
    mesh.stationManual(strSSID, strpassword);
  //myIP = IPAddress(mesh.getStationIP());
  //Serial.println("My station IP is " + myIP.toString());

    
    //Serial.println("local IP is "+ getlocalIP().toString());
    request->send(200, "text/html", setting_ok);
  });
  server.begin();
  //cannot add this function as the plainless mesh will stop continua connect to the wifi
  // Serial.print("Connecting to WiFi ..");
  //while (WiFi.status() != WL_CONNECTED) {
  //  Serial.print('.');
  //  delay(1000);
  //}
  //Serial.println(WiFi.localIP());  
  
}

void loop() {
  mesh.update();
  
  //if (myIP != getlocalIP()) {
  //  myIP = getlocalIP();
  //  Serial.println("My IP is " + myIP.toString());
  //}
  //for check button if it is long press
  checkButton();
}

//-------------------------------------------------------------------
//additional function
//1. send all message
//2. check button if it is long press
//--------------------------------------------------------------------

//send message to all node----------------------------------------------------------------------------
void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
}



//check button -----------------------------------------------------------------------------------------------
void checkButton() {
  // Check for button press
  if (digitalRead(TRIGGER_PIN) == LOW) {
    // Poor-man's debounce/press-hold, code not ideal for production
    delay(50);
    if (digitalRead(TRIGGER_PIN) == LOW) {
      Serial.println("Button Pressed");
      // Still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000); // Reset delay hold
      if (digitalRead(TRIGGER_PIN) == LOW) {
        Serial.println("Useraction:Button held.  System: Erasing wifi cerdentials and restarting.");
        //need something to reset wifi//wm.resetSettings();
        preferences.begin("credentials", false);
        preferences.putString("ssid", "");
        preferences.putString("password", "");
        //wifiCerdentials.clear();
        preferences.end();
        ESP.restart();
      }

      // Start portal w/delay
      Serial.println("Starting config portal.");

    }
  }
}
