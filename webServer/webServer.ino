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

#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "EnergysGroup"
#define   STATION_PASSWORD "NewV1s10n"

#define HOSTNAME "HTTP_BRIDGE"

painlessMesh  mesh;//object for the painless mesh

//List for the nodes
SimpleList<uint32_t> nodes;

// Prototype
void receivedCallback( const uint32_t &from, const String &msg );
IPAddress getlocalIP();

AsyncWebServer server(80);
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());

  
  //Async webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    String dropdownList = "</option>";

    SimpleList<uint32_t>::iterator node = nodes.begin();
    nodes = mesh.getNodeList();
    node = nodes.begin();
    while (node != nodes.end()) {
              dropdownList += "<option value=\"";
              dropdownList += *node;
              dropdownList += "\">";
              dropdownList += *node;
              dropdownList += "</option>";
              node++;
            };
    
    //String dropdownList = "<select name=/"nodeID/" id=/"nodeID/"><option value=/"/">--Please choose an option--</option>";
    String configurationPage = 
    "<form>Configuration Page"
    //"<br><input type='number' name='nodeID'><br>"
    "<select name=\"nodeID\" id=\"nodeID\">"
    "<option value=\"\">--Please choose an option--</option>"
    + dropdownList +
    "</select>"
    "<input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'>"
    "</form>";
    
    request->send(200, "text/html", configurationPage);
    if (request->hasArg("BROADCAST")){
      String msg = request->arg("BROADCAST");
      String nodeID=request->arg("nodeID");
      char buf[nodeID.length()+1];
      //change to char array nodeID.toCharArray(buf,nodeID.length()+1);
      nodeID.toCharArray(buf,nodeID.length()+1);
      int nodeIDint=atoi(buf);

      //sscanf(nodeID, "%04d", &nodeIDint);
      Serial.println(msg);
      Serial.println(nodeIDint);
      mesh.sendSingle(nodeIDint,msg);
    }
  });
  server.begin();

}

void loop() {
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      String dropdownList = "</option>";

    SimpleList<uint32_t>::iterator node = nodes.begin();
    nodes = mesh.getNodeList();
    node = nodes.begin();
    while (node != nodes.end()) {
              dropdownList += "<option value=\"";
              dropdownList += *node;
              dropdownList += "\">";
              dropdownList += *node;
              dropdownList += "</option>";
              node++;
            };
    
    //String dropdownList = "<select name=/"nodeID/" id=/"nodeID/"><option value=/"/">--Please choose an option--</option>";
    String configurationPage = 
    "<form>Configuration Page"
    //"<br><input type='number' name='nodeID'><br>"
    "<select name=\"nodeID\" id=\"nodeID\">"
    "<option value=\"\">--Please choose an option--</option>"
    + dropdownList +
    "</select>"
    "<input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'>"
    "</form>";
    
    request->send(200, "text/html", configurationPage);

    });
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
