//************************************************************
// this is a simple example that uses the painlessMesh library and echos any
// messages it receives
// 1. add M5 stick LCD display function
//************************************************************
#include "painlessMesh.h"

//#include <M5StickC.h>

//#define TFT_GREY 0x5AEB // New colour

#define   MESH_PREFIX     "netmesh"
#define   MESH_PASSWORD   "netmesh1234"
#define   MESH_PORT       5555

// Prototypes
void receivedCallback( uint32_t from, String &msg );

painlessMesh  mesh;

void setup() {
  Serial.begin(115200);
    
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);

  //for the M5 stick dispaly
  //M5.begin();
  //M5.Lcd.setRotation(3);
}

void loop() {
  mesh.update();

  //M5.Lcd.fillScreen(TFT_GREY);
  //M5.Lcd.setCursor(0, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  //M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);  
  //M5.Lcd.setTextSize(1);
  
  auto nodes = mesh.getNodeList(true);
  String str;
  for (auto &&id: nodes){
    str += String(id);
    str += String (" ");
    //M5.Lcd.println(str.c_str());
    
  }
  Serial.println(str);
  delay(3000);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("echoNode: Received from %u msg=%s\n", from, msg.c_str());
  mesh.sendSingle(from, msg);
}
