//************************************************************
// this is a simple example that uses the painlessMesh library and echos any
// messages it receives
//
//************************************************************
#include "painlessMesh.h"
#include <ArduinoJson.h>

//for the FastLED
#include <FastLED.h>
#define NUM_LEDS 1
#define DATA_PIN 27
CRGB leds[NUM_LEDS];

//for the mesh
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Prototypes
void receivedCallback( uint32_t from, String &msg );

painlessMesh  mesh;

void setup() {
  Serial.begin(115200);
    
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);

            // init Fastled and set to black
 FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
           leds[0] = CRGB::Black;
          FastLED.show();
}

void loop() {
  mesh.update();
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("echoNode: Received from %u msg=%s\n", from, msg.c_str());
  mesh.sendSingle(from, msg);
  StaticJsonDocument<50> JSONmessage;

  String controlmessage="{\"controlmsg\"";
  //if the message is {"controlmsg":"LEDon"}
  if (strstr(msg.c_str(),controlmessage.c_str())){
          Serial.println("I have a controlmessage");

          //check the recevied message
          DeserializationError error = deserializeJson(JSONmessage, msg);
          
          if (error) {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
              return;
            }


            

          //FastLED.setBrightness(10);
          //open led to red 
          leds[0] = CRGB::Red;
          FastLED.show();
      
    }
  
}
