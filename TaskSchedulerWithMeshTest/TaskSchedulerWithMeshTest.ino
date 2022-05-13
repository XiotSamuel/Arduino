//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

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

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

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
  // it will run the user scheduler as well
  mesh.update();
}
