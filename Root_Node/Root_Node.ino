//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and relay messages from a MQTT broker to the nodes of the mesh network.
// To send a message to a mesh node, you can publish it to "painlessMesh/to/12345678" where 12345678 equals the nodeId.
// To broadcast a message to all nodes in the mesh you can publish it to "painlessMesh/to/broadcast".
// When you publish "getNodes" to "painlessMesh/to/gateway" you receive the mesh topology as JSON
// Every message from the mesh which is send to the gateway node will be published to "painlessMesh/from/12345678" where 12345678 
// is the nodeId from which the packet was send.
//************************************************************

#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

//Mesh setting
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

//RootNode IP Setting
#define   STATION_SSID     "Samuel"
#define   STATION_PASSWORD "Samuel@1986"

#define HOSTNAME "MQTT_Bridge"

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

//RootNode IP Setting
IPAddress getlocalIP();

//RootNode IP Setting
IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 1, 1);

//Mesh setting
painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

//Scheduler
Scheduler userScheduler; // to control your personal task

// User stub prototype for the actual function
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void PSenor ();
void Lux ();
void PWM ();


//Task timing configuration
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskPSenor( TASK_SECOND * 1 , TASK_FOREVER, &PSenor );
Task taskLux (TASK_SECOND * 1 , TASK_FOREVER, &Lux);
Task taskPWM (TASK_SECOND * 1 , TASK_FOREVER, &PWM);

//task for the PWM
void PWM(){
  Serial.println("PWM\n");
}

//task for the Lux
void Lux(){
  Serial.println("Lux Task\n");
}

//task for the PSenor
void PSenor(){
  Serial.println("PSenor\n");
}

//task for the send message
void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}



void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  //mesh.onReceive(&receivedCallback);

  //Task list
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  //User Scheduler
  
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  
  userScheduler.addTask( taskPSenor );
  taskPSenor.enable();
  
  userScheduler.addTask( taskLux );
  taskLux.enable();
  
  userScheduler.addTask( taskPWM );
  taskPWM.enable();
}

void loop() {
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

   if (mqttClient.connect("painlessMeshClient")) {
      mqttClient.publish("painlessMesh/from/gateway","Ready!");
      mqttClient.subscribe("painlessMesh/to/#");
    } 
  }
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "painlessMesh/from/" + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length+1);
  memcpy(cleanPayload, payload, length);
  cleanPayload[length] = '\0';
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if(targetStr == "gateway")
  {
    if(msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("painlessMesh/from/gateway", str.c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
    }
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
