//for plainlessMesh
#include <painlessMesh.h>

//for the task ArduinoJson.h
#include <ArduinoJson.h>

//for the FastLED
#include <FastLED.h>

// for the BH1750 light senor
#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter(0x23);

#define NUM_LEDS 1
#define DATA_PIN 27
CRGB leds[NUM_LEDS];


//painlessMesh parameter
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555


painlessMesh  mesh;//object for the painless mesh

//object for the scheduler
Scheduler ts;

//painlessmesh task
void sendMessage(); 

//Task object painlessmesh
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

//Task for sendMessage
void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}
// Needed for painless library
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}
// Needed for painless library
void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}
//Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("receivedCallback: Received from %u msg=%s\n", from, msg.c_str());
}

void setup() {
  // put your setup code here, to run once:
    // put your setup code here, to run once:
  Serial.begin(115200);
    
  //ts.addTask(taskPIRsenor);
  ts.enableAll();

  
  // init Fastled
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(10);
  // Clear the led 
  leds[0] = CRGB::Black;
  FastLED.show();

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &ts, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  
  //for enable taskSendMessage
  ts.addTask( taskSendMessage );
  taskSendMessage.enable();

  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  ts.execute();  
  mesh.update();
}
