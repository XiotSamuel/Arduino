#include "painlessMesh.h"
#include "DHT.h"

#define DHTPIN D4
#define DHTTYPE DHT22

#define   MESH_PREFIX     "HelloMyMesh"
#define   MESH_PASSWORD   "hellomymeshnetwork"
#define   MESH_PORT       5555

DHT dht(DHTPIN, DHTTYPE);

void receivedCallback( uint32_t from, String &msg );

painlessMesh  mesh;

size_t logServerId = 0;

// Send message to the logServer every 5 seconds
Task myLoggingTask(5000, TASK_FOREVER, []() {
  
  float h = dht.readHumidity();

  float t = dht.readTemperature();
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& msg = jsonBuffer.createObject();
  msg["nodename"] = "Wemos-t1";
  msg["NodeID"] = mesh.getNodeId();
  msg["Temp"] = String(t)+"C";
  msg["Humidity"] = String(h)+"%";

  String str;
  msg.printTo(str);
  if (logServerId == 0) // If we don't know the logServer yet
    mesh.sendBroadcast(str);
  else
    mesh.sendSingle(logServerId, str);

  // log to serial
  msg.printTo(Serial);
  Serial.printf("\n");
});

void setup() {

  Serial.begin(115200);
  Serial.println("Begin DHT22 Mesh Network test!");

  dht.begin();

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, AUTH_WPA2_PSK, 6 );
  mesh.onReceive(&receivedCallback);

  // Add the task to the mesh scheduler
  mesh.scheduler.addTask(myLoggingTask);
  myLoggingTask.enable();

}

void loop() {
  // put your main code here, to run repeatedly:

  mesh.update();


}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("logClient: Received from %u msg=%s\n", from, msg.c_str());

  // Saving logServer
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
  if (root.containsKey("topic")) {
      if (String("logServer").equals(root["topic"].as<String>())) {
          // check for on: true or false
          logServerId = root["nodeId"];
          Serial.printf("logServer detected!!!\n");
      }
      Serial.printf("Handled from %u msg=%s\n", from, msg.c_str());
  }
}
