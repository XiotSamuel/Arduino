#include "painlessMesh.h"
#include <FastLED.h>

// painless mesh parameter

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// to control your personal task
Scheduler userScheduler; 
painlessMesh  mesh;


// number of led = 1
// led data pin 27

#define NUM_LEDS 1
#define DATA_PIN 27
CRGB leds[NUM_LEDS];

//push button is at pin 39
#define PushButton 39

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

// Crear a user task to replace the main loop to avoid the wifi hanged. Currently check every second
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

//variable to store the brightness
String brightness;

// check every second. If the button is pressed then send a green signal to all other node
void sendMessage() {
  int Push_button_state = digitalRead(PushButton);
  if ( Push_button_state == HIGH )
  { 
    Serial.printf("\nnot press");
    // default is white
    leds[0] = CRGB::White;
    FastLED.show();
    String msg = "Black";
    mesh.sendBroadcast( msg );
  }
  else 
  {
    Serial.printf("\npressed");
    // pressed will be red
    leds[0] = CRGB::Red;
    FastLED.show();
    String msg = "Green";
    //mesh.sendBroadcast( msg );
    String msg1 = brightness;
    mesh.sendBroadcast(msg+";"+msg1);
  }
  
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
//  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
// Do Nothing 
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
  // setup push button pin
  pinMode(PushButton, INPUT);

  // init Fastled
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);
  // Clear the led 
  leds[0] = CRGB::Black;
  FastLED.show();

  //painless mesh
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  //for user input the brightness
  Serial.println("Please enter the brightness (1 to 20):");
  while(Serial.available()==0){}
  brightness = Serial.readString();
  Serial.println(brightness);
  
}

void loop() {
  mesh.update();
}
