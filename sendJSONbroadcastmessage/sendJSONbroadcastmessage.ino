// ArduinoJson - https://arduinojson.org
// Copyright Benoit Blanchon 2014-2021
// MIT License
//
// This example shows how to generate a JSON document with ArduinoJson.
//
// https://arduinojson.org/v6/example/generator/

//#include <ArduinoJson.h>
#include <painlessMesh.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

//nodeID gobal variable
StaticJsonDocument<50> nodeID;


// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );



void sendBroadcastNodeIDMessage();
Task tasksendBroadcastNodeIDMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendBroadcastNodeIDMessage );

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

//added for send broadcast message of the node id
void sendBroadcastNodeIDMessage() {
    //get the node ID at setup
  String msg;
  nodeID["nodeID"] = mesh.getNodeId();
  serializeJson(nodeID, msg);
  Serial.println();
  mesh.sendBroadcast( msg );
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
  // Initialize Serial port
  Serial.begin(115200);
  while (!Serial) continue;

  // Allocate the JSON document
  //
  // Inside the brackets, 200 is the RAM allocated to this document.
  // Don't forget to change this value to match your requirement.
  // Use https://arduinojson.org/v6/assistant to compute the capacity.


  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();



  userScheduler.addTask( tasksendBroadcastNodeIDMessage );
  tasksendBroadcastNodeIDMessage.enable();
  
}

void loop() {
  // not used in this example
  mesh.update();
}

// See also
// --------
//
// https://arduinojson.org/ contains the documentation for all the functions
// used above. It also includes an FAQ that will help you solve any
// serialization problem.
//
// The book "Mastering ArduinoJson" contains a tutorial on serialization.
// It begins with a simple example, like the one above, and then adds more
// features like serializing directly to a file or an HTTP request.
// Learn more at https://arduinojson.org/book/
// Use the coupon code TWENTY for a 20% discount ❤❤❤❤❤
