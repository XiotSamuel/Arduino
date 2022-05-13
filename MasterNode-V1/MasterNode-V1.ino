#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

#include "IPAddress.h"
#include "painlessMesh.h"

#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#define   MESH_PREFIX     "NZARI-Sensors"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "SPARK-XT6HVY"
#define   STATION_PASSWORD "RYE3HDJ94D"

#define HOSTNAME "NZARI-MasterNode"

String nodeName = "HTTP_BRIDGE"; // Name needs to be unique

// Prototype
void receivedCallback( const uint32_t &from, const String &msg );
IPAddress getlocalIP();

painlessMesh  mesh;
AsyncWebServer server(80);
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);


void setup() {
  Serial.begin(115200);
  
/**********************************/
/***      SETUP THE RTC      ******/
/**********************************/
 
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()){
        if (Rtc.LastError() != 0){
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }else{
            Serial.println("RTC lost confidence in the DateTime!");
            Rtc.SetDateTime(compiled);
        }
    }
    if (!Rtc.GetIsRunning()){
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled){
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }else if (now > compiled){
        Serial.println("RTC is newer than compile time. (this is expected)");
    }else if (now == compiled){
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
    Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low); 

/**********************************/
/***      SETUP THE WiFi     ******/
/**********************************/

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION | DEBUG );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  // mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA);
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());

/***************************************/
/***      SETUP THE Web Server       ***/
/***   really only here for testing  ***/
/***************************************/

  //Async webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
    if (request->hasArg("BROADCAST")){
      String msg = request->arg("BROADCAST");
      mesh.sendBroadcast(msg);
    }
  });
  server.begin();

}//end setup

void loop(){
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }

//  RtcDateTime now = Rtc.GetDateTime();
//  printDateTime(now);
//  Serial.print("EpochTimeStamp: ");
  
}//end loop

/******************************/
/***     other functions    ***/
/******************************/

#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt){
    char datestring[20];
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(dt.Epoch32Time());       
    Serial.println(datestring);
}//end printDateTime

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
}//end receiveCallback

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}// end getLocalIP
