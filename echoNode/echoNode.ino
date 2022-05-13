//************************************************************
// this is a simple example that uses the painlessMesh library and echos any
// messages it receives
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Prototypes
void receivedCallback( uint32_t from, String &msg );

painlessMesh  mesh;
SimpleList<uint32_t> nodes;


void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);

  String message;
  nodes = mesh.getNodeList();
  //SimpleList<uint32_t>::iterator node = nodes.begin();
  //while (node != nodes.end()) {
  //  Serial.printf(" %u", *node);
  //  node++;
  //}
  //Serial.println(" ");

}

void loop() {
  mesh.update();
  SimpleList<uint32_t> nodes;

  String message;
  nodes = mesh.getNodeList();

  node = nodes.begin();
  while (node != nodes.end()) {
    message += *node;
    message += " | ";
    node++;
  }
  message += "";
  //        calc_delay = true;
  message += "<br>";
  Serial.println(message);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("echoNode: Received from %u msg=%s\n", from, msg.c_str());
  mesh.sendSingle(from, msg);

  Serial.printf("Connection list:");

  Serial.printf("the node list ");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println(" ");

  String message;
  nodes = mesh.getNodeList();

  node = nodes.begin();
  while (node != nodes.end()) {
    message += *node;
    message += " | ";
    node++;
  }
  message += "";
  //        calc_delay = true;
  message += "<br>";
  Serial.println(message);

}
