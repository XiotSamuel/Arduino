#include "painlessMesh.h"
#include <FastLED.h>
#include "EEPROM.h"
//#include <TaskScheduler.h>
// painless mesh parameter

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

//EEPROM parameter

//int address = 0;
#define EEPROM_SIZE 64

//class for LED light
EEPROMClass  LEDSTATUS("eeprom0", 0x100);
EEPROMClass  R_COLOR("eeprom1", 0x300);
EEPROMClass  G_COLOR("eeprom2", 0x500);
EEPROMClass  B_COLOR("eeprom3", 0x700);
EEPROMClass  BRIGHTNESS("eeprom4", 0x900);

// TaskScheduler parameter
#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST

// to control your personal task
Scheduler userScheduler;
painlessMesh mesh;

int address = 1;
int val = byte(255);
int val_lo = byte(0);
// number of led = 1
// led data pin 27

#define NUM_LEDS 1
#define DATA_PIN 27
CRGB leds[NUM_LEDS];

//push button is at pin 39
#define PushButton 39

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain

// Crear a user task to replace the main loop to avoid the wifi hanged. Currently check every second
//Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

//Comment by Arthur. Nothing is doing for receive testing
// check every second. If the button is pressed then send a green signal to all other node
void sendMessage()
{
  //  int Push_button_state = digitalRead(PushButton);
  //  if ( Push_button_state == HIGH )
  //  {
  //    Serial.printf("\nnot press");
  //    // default is white
  //    leds[0] = CRGB::White;
  //    FastLED.show();
  //    String msg = "Black";
  //    mesh.sendBroadcast( msg );
  //  }
  //  else
  //  {
  //    Serial.printf("\npressed");
  //    // pressed will be red
  //    leds[0] = CRGB::Red;
  //    FastLED.show();
  //    String msg = "Green";
  //    mesh.sendBroadcast( msg );
  //  }
  //
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  
  String msg_receive = msg.c_str();
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg_receive);
  mesh.sendSingle(from, msg);

  int pointer = byte(EEPROM.read(address));
  
  if (msg_receive == "Black")
  {
    Serial.println("No input");
  }
  else
  { 
    Serial.print("the existing value is ");
    Serial.println(pointer);
    
    if (pointer==0) {
      leds[0] = CRGB::Green;
      //for setting the brightness to 6
      FastLED.setBrightness(6);
      FastLED.show();
      //for the EEPROM write
      EEPROM.write(address, val);
      EEPROM.commit();
      
      //put the variable to the EEPROM class
      pointer = byte(EEPROM.read(address));
      LEDSTATUS.put(0, pointer);
      Serial.print("LEDSTATUS in EEPROM class and Flash: ");   
      Serial.println(LEDSTATUS.get(0, pointer));
      
    }else{
      leds[0] = CRGB::Black;
      FastLED.show();
      //for the EEPROM write
      EEPROM.write(address, val_lo);
      EEPROM.commit();

      //put the variable to the EEPROM class
      pointer = byte(EEPROM.read(address));
      LEDSTATUS.put(0, pointer);
      Serial.print("LEDSTATUS in EEPROM class and Flash: ");   
      Serial.println(LEDSTATUS.get(0, pointer));
      
    }
    //Serial.printf("Write to: %s with value %s", address, val);
  }
  

}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup()
{
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
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  //userScheduler.addTask(taskSendMessage);
  //taskSendMessage.enable();

  Serial.println("start...");
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM");
    delay(1000000);
  }
  Serial.println(" bytes read from Flash . Values are:");
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    Serial.print(byte(EEPROM.read(i)));
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("writing random n. in memory");
  
  if (byte(EEPROM.read(address)) > 0)
  {
    leds[0] = CRGB::Green;
    FastLED.show();
  }
  else
  {
    leds[0] = CRGB::Black;
    FastLED.show();
  }

  Serial.println("Testing EEPROMClass\n");
  if (!LEDSTATUS.begin(LEDSTATUS.length())) {
    Serial.println("Failed to initialise NAMES");
    Serial.println("Restarting...");
    delay(1000);
    //ESP.restart();
  }
}

void loop()
{
  mesh.update();
}
