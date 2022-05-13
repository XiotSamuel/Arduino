//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and broadcast message from a webpage to the edges of the mesh network.
// This sketch can be extended further using all the abilities of the AsyncWebserver library (WS, events, ...)
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
// for more details about my version
// https://gitlab.com/Assassynv__V/painlessMesh
// and for more details about the AsyncWebserver library
// https://github.com/me-no-dev/ESPAsyncWebServer
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

#define HOSTNAME "HTTP_BRIDGE"

#define TRIGGER_PIN 39

//for storage in flash
Preferences wifiCerdentials;

// Prototype
void receivedCallback( const uint32_t &from, const String &msg );

painlessMesh  mesh;
AsyncWebServer server(80);
IPAddress myIP(0, 0, 0, 0);
IPAddress myAPIP(192, 168, 10, 1);

String strSSID;
String strpassword;

//HTML setup web site
const char index_html[] PROGMEM = R"rawliteral(
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

const char setting_ok[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<h1>
Form submitted
</h1>
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





  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());



  wifiCerdentials.begin("credentials", false);

  String ssid = wifiCerdentials.getString("ssid", "");
  String password = wifiCerdentials.getString("password", "");

  //Async webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/submit", HTTP_GET, [](AsyncWebServerRequest * request) {

    if (request->hasArg("SSID")) {

      strSSID = request->arg("SSID");
      Serial.print("user input SSID: ");
      Serial.println(strSSID);
    }
    if (request->hasArg("password")) {
      strpassword = request->arg("password");
      Serial.print("user input password: ");
      Serial.println(strpassword);
    }
    if (request->hasArg("customfieldid")){
      Serial.print("the radio form input: ");
      Serial.println(request->arg("customfieldid"));
      
      if(request->arg("customfieldid")=="1"){
        // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
        mesh.setRoot(true);
        // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
        mesh.setContainsRoot(true);
      }else if (request->arg("customfieldid")=="2"){
        // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
        mesh.setRoot(false);
        // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
        mesh.setContainsRoot(false);
      }else {
        Serial.println("radio form customfieldid input error");
      }
    }
    //store wifi cerdentials
    wifiCerdentials.putString("SSID", strSSID);
    wifiCerdentials.putString("password", strpassword);
    //set the mesh to use the wifi
    mesh.stationManual(strSSID, strpassword);
    mesh.setHostname(HOSTNAME);
    //Serial.println("local IP is "+ getlocalIP().toString());
    request->send(200, "text/html", setting_ok);
  });
  
  server.begin();

  if (ssid == "" || password == "")
  {
    Serial.println("no wifi set");
  }
  else
  {
    Serial.print("old wifi setting found ssid: ");
    Serial.print(ssid);
    Serial.print("password: ");
    Serial.println(password);
    mesh.stationManual(ssid, password);
    mesh.setHostname(HOSTNAME);
  }
}

void loop() {
  mesh.update();
  //if (myIP != getlocalIP()) {
  //  myIP = getlocalIP();
  //  Serial.println("My IP is " + myIP.toString());
  //}

  checkButton();
}

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
        wifiCerdentials.clear();
        ESP.restart();
      }

      // Start portal w/delay
      Serial.println("Starting config portal.");

    }
  }
}
