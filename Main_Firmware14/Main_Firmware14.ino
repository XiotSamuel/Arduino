//************************************************************
//This is the test script for the wifi and ap switching without wifi manager
//add storage function 19July2021
//total fail: cannot get the PIR or light senor data with task scheduler because the plainlessmesh task that block other task
//add time fuction
//29 July 2021 combinated to one firmware, but need to check the memory issue
//30 July 2021 add the NodeList function for list out the end node
//4 Aug 2021 set out the light power
//5 Aug 2021 setup web site for the demo
//8 Aug 2021 separted the web site to html file
//10 Aug 2021 add the time function to root node
//************************************************************

#include <IPAddress.h>
#include <painlessMesh.h>
#include <Preferences.h>
#include <time.h>

//self defination header file-----------------------------------
//for the global variable
#include "parameter.h"

#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

//for time------------------------------------
#include <time.h>
#include <TimeLib.h>

// NTP server to request epoch time
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.window.com";
const char *ntpServer3 = "time.google.com";
const uint16_t utcOffset = 28800;
const uint8_t daylightoffset = 0;

// Variable to save current epoch time
unsigned long epochTime;

//for mqtt -----------------------------------
#include <SPI.h>
#include <SPIFFS.h>
//#include <WiFi.h>
#include <PubSubClient.h>

#include <ArduinoJson.h>

//for the FastLED-----------------------------
#include <FastLED.h>

// for the BH1750 light senor------------------------
#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter(0x23);

#define NUM_LEDS 1
#define DATA_PIN 27
CRGB leds[NUM_LEDS];

//-------------------------------------------------------------
//painlessMesh
//-------------------------------------------------------------

//painlessMesh parameter disabled default setting
//#define   MESH_PREFIX     "NetMesh"
//#define   MESH_PASSWORD   "NetMesh2021"
//#define   MESH_PORT       5555

//pin setting
#define TRIGGER_PIN 39

//for the fastled
int ledPin = 27;    // choose the pin for the LED
int inputPin = 19;  // choose the input pin (for PIR sensor)
int pirState = LOW; // we start, assuming no motion detected
int val = 0;        // variable for reading the pin status

//for storage in flash
Preferences preferences;

// Prototype
void receivedCallback(const uint32_t &from, const String &msg);
void subscribeReceive(char *topic, byte *payload, unsigned int length);
void receivedCallback_endnode(const uint32_t &from, const String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
const char *returnTopic();
void checkButton();
void sendSensorLog(String _msg);
void sendMessageToSingleNode(String _msg);
void sendBroadcastMessage(String msg);

//String getNodeList();

//MQTT client
IPAddress mqtt_server(0, 0, 0, 0);
WiFiClient espClient;
const char *mqttserver = "staging.energysmeter.com";
PubSubClient mqttClient(mqtt_server, 1883, subscribeReceive, espClient);

//------------------------------------------------------------
//Task Scheduler
//------------------------------------------------------------
//object for the scheduler
Scheduler ts;

//Task sendBroadcastNodeIDMessage
//void sendBroadcastNodeIDMessage();
//Task tasksendBroadcastNodeIDMessage(TASK_SECOND * 10, TASK_FOREVER, &sendBroadcastNodeIDMessage);

//Task sendBroadcastNodeIDMessage
//void sendBroadcastRootIDMessage();
//Task tasksendBroadcastRootIDMessage(TASK_SECOND * 10, TASK_FOREVER, &sendBroadcastRootIDMessage);

//void sendBroadcastDeviceInformationMessage();
//Task tasksendBroadcastDeviceInformationMessage(TASK_SECOND * 30, TASK_FOREVER, &sendBroadcastDeviceInformationMessage);

//Task object timing configuration
//void PIRsenor();
//Task taskPIRsenor(3000, TASK_FOREVER, &PIRsenor, &ts);

//void PIRsenorReport();
//Task taskPIRsenorReport(TASK_SECOND * 60, TASK_FOREVER, &PIRsenorReport, &ts);

//Task object LUX meter
//void LUXsenor();
//Task taskLUXsenor(TASK_SECOND * 10, TASK_FOREVER, &LUXsenor, &ts);

//Task object internal temp sensor
//void InternalTempsensor();
//Task taskInternalTempsensor(TASK_SECOND * 60, TASK_FOREVER, &InternalTempsensor, &ts);

//Task node status report
//void nodeStatusReport();
//Task tasknodeStatusReport(TASK_SECOND * 60, TASK_FOREVER, &nodeStatusReport, &ts);

//Task RGBbrightness
//void RGBbrightness();
//Task taskRGBbrightness(TASK_SECOND * 60, TASK_FOREVER, &RGBbrightness, &ts);

//Task for config time server
//void configTimeServer();
//Task taskconfigTimeServer(TASK_SECOND * 10, TASK_FOREVER, &configTimeServer, &ts);

//-------------------------------------------------------------
//painlessMesh
//-------------------------------------------------------------
painlessMesh mesh;
AsyncWebServer server(80);
IPAddress myIP(0, 0, 0, 0);
IPAddress myAPIP(0, 0, 0, 0);

//for perfernce wifi
const char *strSSID;
const char *strpassword;
//for perfernce mesh
const char *strMeshSSID;
const char *strMeshpassword;

//Sensor setting---------------------------------------------------

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause_counter = 15000;

//For the PIR senor lock low counter
boolean lockLow = true;
boolean takeLowTime;

//gobal variable for the brightness and the lux
int brightness_init = 200;
int brightness_mem;
int lux_init = lightMeter.readLightLevel();
int lux_mem;

//gobal variable for colour setting
int led_red = 0;
int led_green = 0;
int led_blue = 0;

//gobal variable for root ID
int root_ID;

uint8_t temprature_sens_read();

//List for the nodes
SimpleList<uint32_t> nodes;

//gobal variable for the JSON doc nodeID
StaticJsonDocument<50> nodeID;

// PWM led--------------------------------------------------------------------
// the number of the LED pin
const int ledPin21 = 21; // 16 corresponds to GPIO21
const int ledPin22 = 22; // 17 corresponds to GPIO22
const int ledPin23 = 23; // 5 corresponds to GPIO23

// setting PWM properties
const int freq = 5000;
const int ledChannel_R = 0;
const int ledChannel_B = 1;
const int ledChannel_G = 2;

const int resolution = 8;

//RBG Brightness setting function

float RGBbrightness_old_power = 0;

int RGBbrightness_R;
int RGBbrightness_G;
int RGBbrightness_B;
float RGBbrightness_power;

//deviceID array
int deviceID[100];

void setup()
{

  Serial.begin(115200);
// initize the SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  uint32_t ChipId = ESP.getEfuseMac() & 0xFFFFFFFF;
  pinMode(TRIGGER_PIN, INPUT);

  Serial.println("Setting Plainlessmesh");
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION|COMMUNICATION); // set before init() so that you can see startup messages
  String MESH_PREFIX = "Energys_" + String(ChipId);
  String MESH_PASSWORD = "Mesh2021";
  uint16_t MESH_PORT = 5555;
  //Serial.println(apiChipId);
  //MESH_PREFIX += String(apiChipId);
  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)

  // init Fastled
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(10);
  // Clear the led
  leds[0] = CRGB::Black;
  NODESTATUS.colour[0] = 0;
  NODESTATUS.colour[1] = 0;
  NODESTATUS.colour[2] = 0;
  NODESTATUS.lightONOFF = false;
  FastLED.show();

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin(33, 22);

  //setup the LUX meter and print out the result
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
  {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else
  {
    Serial.println(F("Error initialising BH1750"));
  }

  //Setup the LED PWM---------------------------------------------
  // configure LED PWM functionalitites
  ledcSetup(ledChannel_R, freq, resolution);
  ledcSetup(ledChannel_B, freq, resolution);
  ledcSetup(ledChannel_G, freq, resolution);

  // attach the channel to the GPIO to be controlled
  //Mapping the Pin as a light channel
  ledcAttachPin(ledPin21, ledChannel_R);
  ledcAttachPin(ledPin22, ledChannel_G);
  ledcAttachPin(ledPin23, ledChannel_B);

  //Setup the storage information---------------------------
  Serial.println("Start storage");
  preferences.begin("credentials", false);

  //get the wifi setting in flash------------------------------------
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  String meshSSID = preferences.getString("mesh_ssid", "");
  String meshpassword = preferences.getString("mesh_password", "");
  bool isRoot = preferences.getBool("Root", "");
  preferences.end();

  //check if have old mesh setting
  if (meshSSID == "" || meshpassword == "")
  {
    Serial.println("no mesh set");

    mesh.init(MESH_PREFIX, MESH_PASSWORD, &ts, MESH_PORT, WIFI_AP_STA, 6);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    mesh.setHostname("FactoryNode");

    myAPIP = IPAddress(mesh.getAPIP());
    Serial.println("My AP IP is " + myAPIP.toString());
    Serial.println("Setting Plainlessmesh done");
  }
  else
  {
    //check if have old wifi setting
    if (ssid == "" || password == "")
    {
      Serial.println("no wifi set (for end node only)");

      Serial.print("old mesh setting found:: ssid: ");
      Serial.print(meshSSID);
      Serial.print(" password: ");
      Serial.println(meshpassword);

      mesh.init(meshSSID, meshpassword, &ts, MESH_PORT, WIFI_STA, 6);
      mesh.onReceive(&receivedCallback_endnode);
      mesh.onNewConnection(&newConnectionCallback);
      mesh.onChangedConnections(&changedConnectionCallback);
      mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
      mesh.setHostname("Mesh");
      myAPIP = IPAddress(mesh.getAPIP());
      Serial.println("My AP IP is " + myAPIP.toString());
      Serial.println("Setting Plainlessmesh done");

      //for enable LUX sensor
      //ts.addTask(taskLUXsenor);
      //taskLUXsenor.enable();
      //taskLUXsenor.disable();

      //for enable PIR senor
      //ts.addTask(taskPIRsenorReport);
      //taskPIRsenorReport.enable();
      //taskPIRsenorReport.disable();

      //for enable tasknodeStatusReport
      //ts.addTask(tasknodeStatusReport);
      //tasknodeStatusReport.enable();

      //ts.addTask(taskInternalTempsensor);
      //taskInternalTempsensor.enable();
    }
    else
    {

      Serial.println("wifi set (for root node only)");

      Serial.print("old wifi setting found:: ssid: ");
      Serial.print(ssid);
      Serial.print(" password: ");
      Serial.println(password);
      Serial.print("old mesh setting found:: ssid: ");
      Serial.print(meshSSID);
      Serial.print(" password: ");
      Serial.println(meshpassword);

      mesh.init(meshSSID, meshpassword, &ts, MESH_PORT, WIFI_AP_STA, 6);
      mesh.onReceive(&receivedCallback);
      mesh.onNewConnection(&newConnectionCallback);
      mesh.onChangedConnections(&changedConnectionCallback);
      mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
      mesh.setHostname("Mesh");
      myAPIP = IPAddress(mesh.getAPIP());
      Serial.println("My AP IP is " + myAPIP.toString());
      Serial.println("Setting Plainlessmesh done");
      mesh.stationManual(ssid, password);
      //mesh.setHostname(HOSTNAME);
    }
  }
  
  //check if have old mesh setting
  if (isRoot)
  {
    Serial.println("old root setting found");
    mesh.setRoot(true);
    mesh.setContainsRoot(true);

    //for enable tasksendBroadcastRootIDMessage
    //ts.addTask(tasksendBroadcastRootIDMessage);
    //tasksendBroadcastRootIDMessage.enable();
    //for enable taskconfigTimeServer
    //ts.addTask(taskconfigTimeServer);
    //taskconfigTimeServer.enable();
  }
  else
  {
    Serial.print("Started as end node");
    mesh.setRoot(false);
    mesh.setContainsRoot(false);

    // ts.addTask(tasksendBroadcastNodeIDMessage);
    // tasksendBroadcastNodeIDMessage.enable();
  }

  //Web page-------------------------------------------------------------------------------------------------
  //while(WiFi.status() != WL_CONNECTED){
  //  Serial.print(".");
  //}


  //for index page handler
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  //-for demo-------------------------------------------------------
  //get the node id string

         //

        //for index page handler
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->arg("index") == "1")
              {
                request->send(SPIFFS, "/settingWIFI.html", "text/html");
              }
              else if (request->arg("index") == "2")
              {
                request->send(SPIFFS, "/settingDEVICEINFO.html", "text/html");
              }
              else if (request->arg("index") == "3")
              {
                request->send(SPIFFS, "/send_message.html", "text/html");
              }
              else if (request->arg("index") == "4")
              {
                request->send(SPIFFS, "/submit_ledsetting.html", "text/html");
              }
              else if (request->arg("index") == "5")
              {
                request->send(SPIFFS, "/settingENDNODEled.html", "text/html" );
              }
              else
              {
                Serial.println("index page selection error");
              }
            });

  //for send message page handler
  server.on("/sendMessage", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasArg("Message"))
              {
                String strMessage = request->arg("Message");
                Serial.print("user input Message: ");
                Serial.println(strMessage);
                mesh.sendBroadcast(strMessage);
                request->send(SPIFFS, "/send_message.html", "text/html");
              }
            });

  //for wifi page setting handler
  server.on("/submit_settingwifi", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              bool isRoot=false;
              if (request->hasArg("SSID"))
              {

                strSSID = request->arg("SSID").c_str();
                Serial.print("user input SSID: ");
                Serial.println(strSSID);
              }
              if (request->hasArg("password"))
              {
                strpassword = request->arg("password").c_str();
                Serial.print("user input password: ");
                Serial.println(strpassword);
              }

              if (request->hasArg("meshSSID"))
              {

                strMeshSSID = request->arg("meshSSID").c_str();
                Serial.print("user input SSID: ");
                Serial.println(strMeshSSID);
              }
              if (request->hasArg("meshpassword"))
              {
                strMeshpassword = request->arg("meshpassword").c_str();
                Serial.print("user input password: ");
                Serial.println(strMeshpassword);
              }

              if (request->hasArg("customfieldid"))
              {
                Serial.print("the radio form input: ");
                Serial.println(request->arg("customfieldid"));

                if (request->arg("customfieldid") == "1")
                {
                  mesh.setRoot(true);
                  mesh.setContainsRoot(true);
                  //for enable tasksendBroadcastRootIDMessage
                  //ts.addTask(tasksendBroadcastRootIDMessage);
                  //tasksendBroadcastRootIDMessage.enable();
                  isRoot = true;
                }
                else if (request->arg("customfieldid") == "2")
                {
                  mesh.setRoot(false);
                  mesh.setContainsRoot(false);
                  isRoot = false;
                }
                else
                {
                  Serial.println("radio form customfieldid input error");
                }
              }
              //store wifi cerdentials
              preferences.begin("credentials", false);
              preferences.putString("ssid", strSSID);
              preferences.putString("password", strpassword);
              preferences.putString("mesh_ssid", strMeshSSID);
              preferences.putString("mesh_password", strMeshpassword);
              preferences.putBool("Root", isRoot);
              preferences.end();
              //set the mesh to use the wifi

              //mesh.stationManual(strSSID, strpassword);

              request->send(SPIFFS, "/setting_ok.html", "text/html");
              ESP.restart();
            });

  //for device information page button handler
  server.on("/submit_deviceinfo", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasArg("companyName"))
              {

                DEVICEINFORMATION.companyName = request->arg("companyName").c_str();
                Serial.print("user input companyName: ");
                Serial.println(DEVICEINFORMATION.companyName);
              }
              if (request->hasArg("siteName"))
              {
                DEVICEINFORMATION.siteName = request->arg("siteName").c_str();
                Serial.print("user input siteName: ");
                Serial.println(DEVICEINFORMATION.siteName);
              }
              if (request->hasArg("group"))
              {
                DEVICEINFORMATION.group = request->arg("group").c_str();
                Serial.print("user input group: ");
                Serial.println(DEVICEINFORMATION.group);
              }

              DEVICEINFORMATION.deviceID = mesh.getNodeId();
              Serial.print("user input nodeID: ");
              Serial.println(DEVICEINFORMATION.deviceID);

              if (request->hasArg("deviceName"))
              {
                DEVICEINFORMATION.deviceName = request->arg("deviceName").c_str();
                Serial.print("user input deviceName: ");
                Serial.println(DEVICEINFORMATION.deviceName);
              }

              DEVICEINFORMATION.bootTime = mesh.getNodeId();
              Serial.print("user input bootTime: ");
              Serial.println(DEVICEINFORMATION.bootTime);

              //MQTT---------------------------------------------------------------------
              Serial.println("Setting MQTT Client");
              // Set the MQTT server to the server stated above ^
              mqttClient.setServer(mqttserver, 1883);

              // Attempt to connect to the server with the ID "myClientID"
              if (mqttClient.connect("myClientID", "user2", "Qazw123$"))
              {
                Serial.println("Connection has been established, well done");

                // Establish the subscribe event
                mqttClient.setCallback(subscribeReceive);
                //subscribe the mqtt client for the normal topic
                //if the mqtt server send cmd with topic Company1/Site1/Group1/2125307893/DeviceA
                mqttClient.subscribe(returnTopic());
              }
              else
              {
                Serial.println("Looks like the server connection failed...");
                mqttClient.disconnect();
              }

              Serial.println("Setting MQTT Client done");

              mqttClient.publish(returnTopic(), "Hello");
              Serial.print("Topic form as: ");
              Serial.println(String(returnTopic()));

              //ts.addTask(tasksendBroadcastDeviceInformationMessage);
              //tasksendBroadcastDeviceInformationMessage.enable();

              request->send(SPIFFS, "/setting_ok.html", "text/html");
            });

  //for ledPower handler
  server.on("/submitLedSetting", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasArg("ledPower") && request->hasArg("ledR") && request->hasArg("ledG") && request->hasArg("ledB"))
              {

                //ts.addTask(taskRGBbrightness);

                float ledPower = request->arg("ledPower").toFloat();
                int R = request->arg("ledR").toInt();
                int G = request->arg("ledG").toInt();
                int B = request->arg("ledB").toInt();
                Serial.print("User input ledPower: ");
                Serial.println(ledPower);
                RGBbrightness_R = R;
                RGBbrightness_G = G;
                RGBbrightness_B = B;
                RGBbrightness_power = ledPower / 100;
                //taskRGBbrightness.enable();
                //RGBbrightness();
                //taskRGBbrightness.disable();
              }
              request->send(SPIFFS, "/setting_ok.html", "text/html");
            });



  //for for setting end node led on broad led
  server.on("/settingENDNODEled", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              //List all parameters
              int params = request->params();
              for (int i = 0; i < params; i++)
              {
                AsyncWebParameter *p = request->getParam(i);
                if (p->isFile())
                { //p->isPost() is also true
                  Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
                }
                else if (p->isPost())
                {
                  Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
                else
                {
                  Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
              }

              float ledPower = request->arg("ledPower").toFloat();
              auto nodeID = request->arg("nodeID").toInt();
              int R = request->arg("ledR").toInt();
              int G = request->arg("ledG").toInt();
              int B = request->arg("ledB").toInt();
             

              
              Serial.println(nodeID);
              DynamicJsonDocument JSONmessage(ESP.getMaxAllocHeap());
              //mqtt server need to send {"DeviceCMD":{"group":"TRUE","command":"led_on"}} to trigger
 
              //JSONmessage["DeviceCMD"]["deviceID"] = nodeID;
              //JSONmessage["DeviceCMD"]["command"] = "setup_led";
              //JSONmessage["DeviceCMD"]["lightLevel"] = ledPower / 5;
              //JSONmessage["DeviceCMD"]["R"] = R;
              //JSONmessage["DeviceCMD"]["B"] = B;
              //JSONmessage["DeviceCMD"]["G"] = G;
              if (request->arg("ledPower").toInt() == 0){
              JSONmessage["DeviceCMD"]["group"] = "TRUE";
              JSONmessage["DeviceCMD"]["command"] = "led_off";
              }else{
              JSONmessage["DeviceCMD"]["group"] = "TRUE";
              JSONmessage["DeviceCMD"]["command"] = "led_on";
              }
              String msg;
              serializeJson(JSONmessage, msg);
              Serial.print("setup(server.on(\" / settingENDNODEled \"): to sendMessageToSingleNode(msg)");
              Serial.println(msg);
              //send to the target node
              //sendMessageToSingleNode(msg);
              sendBroadcastMessage(msg);
              request->send(SPIFFS, "/settingENDNODEled.html", "text/html");
            });
  //for updating the node list
  server.on("/settingENDNODEled_updateList", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              nodes = mesh.getNodeList(true);
              //Serial.print("mesh getNodeList size: ");
              //Serial.println(mesh.getNodeList().size());
              //Serial.print("mesh as node tree");
              //Serial.println(mesh.asNodeTree().toString());
              //Serial.print("mesh as subConnection");
              String str = "";
              //Serial.println("Nodes");
              //int i =0;
              //while(i<100)
              //{
              //  str += deviceID[i];
              //  str += (":");
                //Serial.println(String(id));
                //deviceID[i];
              //  i++;
              //}

              //for (auto &&id : nodes)
              //{
              //  str += String(id);
              //  str += (":");
              //  Serial.println(String(id));
              //}
              //Serial.print("Setting() server.on getNodeList: ");
              //Serial.println(str);
              //nodes = mesh.getNodeList();
              
              Serial.printf("節點數: %d\n", nodes.size());
              Serial.printf("連線列表: ");
              SimpleList<uint32_t>::iterator node = nodes.begin();
              while (node != nodes.end())
              {
                Serial.printf(" %u", *node);
                str += String(*node);
                str += (":");
                node++;
              }
              Serial.println();
              Serial.print("Setting() server.on getNodeList: ");
              Serial.println(str);
              request->send(200, "text/html", str);
              
            });

/*server.on("/settingENDNODEled_updateList", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              uint8_t *data;
               size_t len;
               size_t index; 
               size_t total;
               AwsTemplateProcessor callback;
              auto nodes = mesh.getNodeList(true);
               int i = 0;
               for (auto &&id : nodes)
               {

                 data[i] = id;
                 i++;
              }
              request->send(200, data, len, callback);
            }); */

  server.begin();
}

//String getNodeList(){
//  String str;
//  for (auto &&id : nodes)
//  {
 //   str = String(id);

  //  str += String(" : ");
 // }
 // return str;
//}

void loop()
{
  //loop for mqtt client basic function
  mqttClient.loop();
  //loop for mesh basic function
  mesh.update();
  //loop for task scheduler basic function
  ts.execute();

  checkButton();

}

//-------------------------------------------------------------------
//additional function
//1. send all message
//2. check button if it is long press
//3. Function that gets current epoch time
//4. Function get the current time in human format
//5. receivedCallback for the root node received call message
//6. receivedCallback_endnode for the end node reived call message
//7. checkButton for checking the button is long press
//8. returnTopic for create string for the topic
//9. subscribeReceive for Subscribe MQTT message and print out message string
//--------------------------------------------------------------------


int counter=0;
// receivedCallback: function for the root node
// Message path: End node to root node
// Finial: will relay the message to the MQTT server
// received message root node----------------------------------------------------------------------------
void receivedCallback(const uint32_t &from, const String &msg)
{
  //Start the mqtt connection
  if (mqttClient.connect("myClientID", "user2", "Qazw123$"))
  {
    Serial.println("MQTT connection has been reestablished, well done");

    // Establish the subscribe event
    mqttClient.setCallback(subscribeReceive);
    //subscribe for the return topic
    mqttClient.subscribe(returnTopic());
  }
  else
  {
    Serial.println("Looks like the server connection failed...");
  }

  //for print out the received message
  Serial.printf("receivedCallback: Received from %u msg=%s\n", from, msg.c_str());
  //get the max heap for buffer the message
  DynamicJsonDocument JSONmessage(1024);
  DeserializationError error = deserializeJson(JSONmessage, msg);
  if (error)
  {
    Serial.print(F("receivedCallback deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  //initize full path
  std::string fullPath = returnTopic();

  //json message filitering
  if (JSONmessage["LUXmeter"])
  {
    std::string nodeID = JSONmessage["LUXmeter"]["NodeID"];
    fullPath += "/";
    fullPath += nodeID;
    fullPath += "/LUX";
    const char *foo = fullPath.c_str();
    mqttClient.publish(foo, JSONmessage["LUXmeter"]["Lux"].as<const char *>());

    Serial.print("Publish to MQTT server: ");
    Serial.println(JSONmessage["LUXmeter"]["Lux"].as<const char *>());
  }
  if (JSONmessage["PIRsensor"])
  {
    std::string nodeID = JSONmessage["PIRsensor"]["NodeID"];
    fullPath += "/";
    fullPath += nodeID;
    fullPath += "/PIRcounter";
    const char *foo = fullPath.c_str();
    mqttClient.publish(foo, JSONmessage["PIRsensor"]["PIRcounter"]);

    Serial.print("Publish to MQTT server: ");
    Serial.println(JSONmessage["PIRsensor"]["PIRcounter"].as<const char *>());
  }

  if (JSONmessage["PIRsensorReport"])
  {
    std::string nodeID = JSONmessage["PIRsensorReport"]["NodeID"];
    fullPath += "/";
    fullPath += nodeID;
    fullPath += "/timeStamp";
    const char *foo = fullPath.c_str();

    std::string mesg = JSONmessage["PIRsensorReporttimeStamp"];
    const char *poo = mesg.c_str();
    mqttClient.publish(foo, poo);

    Serial.print("Publish to MQTT server: ");
    Serial.println(poo);
  }

  //for publich the node status to server
  if (JSONmessage["NodeStatus"])
  {
    std::string nodeID = JSONmessage["NodeStatus"]["NodeID"];
    fullPath += "/";
    fullPath += nodeID;
    fullPath += "/NodeStatus";
    const char *foo = fullPath.c_str();
    //for send out normal node status
    //mqttClient.publish(foo, JSONmessage["NodeStatus"].as<char*>());
    std::string mesg = JSONmessage["NodeStatus"];
    const char *poo = mesg.c_str();
    mqttClient.publish(foo, poo);

    Serial.print("Publish to MQTT server: ");
    Serial.println(poo);
  }
  //TODO: need to rethink the logic too slow, and cannot compare the value.
  // if (JSONmessage["DeviceInfo"])
  // {

  //     int deviceID_new=JSONmessage["DeviceInfo"]["deviceID"].as<int>();
  //     Serial.print("deviceInfo recevied: ");
  //     Serial.println(deviceID_new);
  //     bool duplicated_flag=false;
  //     int i =0;

  //     SimpleList<uint32_t>::iterator node = nodes.begin();
  //     Serial.println();
  //     while (node != nodes.end())
  //     {
  //       Serial.print("1");
  //       //Serial.printf(" %u", *node);
  //       if (deviceID_new == *node){
  //         duplicated_flag = true;
  //         Serial.println("duplicated found");
  //         break;
  //       }

  //       node++;
  //     }

  //     if (!duplicated_flag){
  //       nodes.push_back(deviceID_new);
  //       Serial.println("receivedCallback(const uint32_t &from, const String &msg): new device added");
  //       counter++;}
  // }
}

// receivedCallback_endnode: function for the end node
// Message path: Root node to End node
// Finial: Output and interact with the device

String arrayCMD[12];
void receivedCallback_endnode(const uint32_t &from, const String &msg)
{
  Serial.printf("receivedCallback_endnode: Received from %u msg=%s\n", from, msg.c_str());

  DynamicJsonDocument JSONmessage(1024);
  DeserializationError error = deserializeJson(JSONmessage, msg);

  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  if (JSONmessage["RootID"])
  {
    Serial.println("Update: root node status updated");
    Serial.println(int(JSONmessage["RootID"]));
    root_ID = int(JSONmessage["RootID"]);
    //Reply to root node my ID
    //DynamicJsonDocument JSONmessage(128);
    //JSONmessage["DeviceInfo"]["deviceID"] = mesh.getNodeId();
    //String msg;
    //serializeJson(JSONmessage, msg);
    //sendBroadcastMessage(msg);
  }

  if (JSONmessage["deviceInformation"])
  {
    DEVICEINFORMATION.companyName = JSONmessage["deviceInformation"]["companyName"].as<String>();
    DEVICEINFORMATION.siteName = JSONmessage["deviceInformation"]["siteName"].as<String>();
    DEVICEINFORMATION.group = JSONmessage["deviceInformation"]["group"].as<String>();
    DEVICEINFORMATION.deviceName = JSONmessage["deviceInformation"]["deviceName"].as<String>();
    Serial.println("JSONmessage: DeviceInformation updated");
    Serial.println(DEVICEINFORMATION.companyName);
    Serial.println(DEVICEINFORMATION.siteName);
    Serial.println(DEVICEINFORMATION.group);
    Serial.println(DEVICEINFORMATION.deviceName);
  }

  if (JSONmessage["DeviceCMD"])
  {
    if (JSONmessage["DeviceCMD"]["group"] == "TRUE")
    {
      //mqtt server need to send {"DeviceCMD":{"group":"TRUE","command":"led_on"}} to trigger
      if (JSONmessage["DeviceCMD"]["command"] == "led_on")
      {
        Serial.println("MQTT group CMD: led_on");
        //open the led to white
        leds[0] = CRGB(255, 255, 255);
        NODESTATUS.colour[0] = 255;
        NODESTATUS.colour[1] = 255;
        NODESTATUS.colour[2] = 255;
        NODESTATUS.lightONOFF = true;
        FastLED.show();
      }

      //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"led_off"}} to trigger
      if (JSONmessage["DeviceCMD"]["command"] == "led_off")
      {
        Serial.println("MQTT group CMD: led_off");
        //open the led to black
        leds[0] = CRGB(0, 0, 0);
        NODESTATUS.colour[0] = 0;
        NODESTATUS.colour[1] = 0;
        NODESTATUS.colour[2] = 0;
        NODESTATUS.lightONOFF = false;
        FastLED.show();
      }
    }

      // if (JSONmessage["DeviceCMD"]["deviceID"] == mesh.getNodeId())
      // {
      //   //mqtt server need to send
      //   //topic--> Company1/Site1/Group1/2125307893/DeviceA
      //   //msg-->{"DeviceCMD":{"deviceID":2125191133,"command":"restart"}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "restart")
      //   {
      //     Serial.println("MQTT CMD: restart");
      //     ESP.restart();
      //   }

      //   //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"status"}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "status")
      //   {
      //     Serial.println("MQTT CMD: status");
      //     //nodeStatusReport();
      //   }

      //   //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"led_on"}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "led_on")
      //   {
      //     Serial.println("MQTT CMD: led_on");
      //     //open the led to white
      //     leds[0] = CRGB(255, 255, 255);
      //     NODESTATUS.colour[0] = 255;
      //     NODESTATUS.colour[1] = 255;
      //     NODESTATUS.colour[2] = 255;
      //     NODESTATUS.lightONOFF = true;
      //     FastLED.show();
      //   }

      //   //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"led_off"}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "led_off")
      //   {
      //     Serial.println("MQTT CMD: led_off");
      //     //open the led to black
      //     leds[0] = CRGB(0, 0, 0);
      //     NODESTATUS.colour[0] = 0;
      //     NODESTATUS.colour[1] = 0;
      //     NODESTATUS.colour[2] = 0;
      //     NODESTATUS.lightONOFF = false;
      //     FastLED.show();
      //   }

      //   //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"change_colour", "colour_0":255,  "colour_1":255, "colour_2":255}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "change_colour")
      //   {
      //     Serial.println("MQTT CMD: change_colour");
      //     int R = JSONmessage["DeviceCMD"]["colour_0"].as<int>();
      //     int B = JSONmessage["DeviceCMD"]["colour_1"].as<int>();
      //     int G = JSONmessage["DeviceCMD"]["colour_2"].as<int>();
      //     //open the led to black
      //     leds[0] = CRGB(R, B, G);

      //     NODESTATUS.colour[0] = R;
      //     NODESTATUS.colour[1] = B;
      //     NODESTATUS.colour[2] = G;
      //     NODESTATUS.lightONOFF = true;

      //     FastLED.show();
      //   }

      //   //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"led_off"}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "change_brightness")
      //   {
      //     Serial.println("MQTT CMD: change_brightness");
      //     int lightLevel = JSONmessage["DeviceCMD"]["lightLevel"].as<int>();
      //     //leds[0] = CRGB(R, B, G);
      //     NODESTATUS.lightLevel = lightLevel;
      //     FastLED.setBrightness(lightLevel);
      //     FastLED.show();
      //   }

      //   //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"setup_led","lightLevel":100,"R":102,"B":255,"G":178}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "setup_led")
      //   {
      //     Serial.println("MQTT CMD: setup_light");
      //     int lightLevel = JSONmessage["DeviceCMD"]["lightLevel"].as<int>();
      //     int R = JSONmessage["DeviceCMD"]["R"].as<int>();
      //     int B = JSONmessage["DeviceCMD"]["B"].as<int>();
      //     int G = JSONmessage["DeviceCMD"]["G"].as<int>();

      //     leds[0] = CRGB(R, B, G);
      //     FastLED.setBrightness(lightLevel);
      //     FastLED.show();

      //     //ts.addTask(taskRGBbrightness);
      //     //taskRGBbrightness.enable();
      //   }

      //   //mqtt server need to send {"DeviceCMD":{"deviceID":2125191133,"command":"setup_light","lightLevel":100,"R":102,"B":255,"G":178}} to trigger
      //   if (JSONmessage["DeviceCMD"]["command"] == "setup_light")
      //   {
      //     Serial.println("MQTT CMD: setup_light");
      //     int lightLevel = JSONmessage["DeviceCMD"]["lightLevel"].as<int>();
      //     int R = JSONmessage["DeviceCMD"]["R"].as<int>();
      //     int B = JSONmessage["DeviceCMD"]["B"].as<int>();
      //     int G = JSONmessage["DeviceCMD"]["G"].as<int>();

      //     RGBbrightness_R = R;
      //     RGBbrightness_G = G;
      //     RGBbrightness_B = B;
      //     RGBbrightness_power = lightLevel / 100;

      //     ts.addTask(taskRGBbrightness);
      //     taskRGBbrightness.enable();
      //   }

      //   //store the message to array
      //   //Serial.println ("CMD JSON message to array: ");
      //   //arrayCMD[0] = JSONmessage["DeviceCMD"]["deviceID"].as<String>();
      //   //arrayCMD[1] = JSONmessage["DeviceCMD"]["command"].as<String>();
      // }
  }

  // if (JSONmessage["TimeSync"])
  // {

  //   jsonBuffer["TimeSync"]["sec"] = now.tm_sec;
  //   jsonBuffer["TimeSync"]["min"] = now.tm_min;
  //   jsonBuffer["TimeSync"]["hour"] = now.tm_hour;
  //   jsonBuffer["TimeSync"]["day"] = now.tm_mday;
  //   jsonBuffer["TimeSync"]["mon"] = now.tm_mon;
  //   jsonBuffer["TimeSync"]["year"] = now.tm_year;
  //   jsonBuffer["TimeSync"]["dst"] = now.tm_isdst;

    // struct timeval tv =
    // {

    // jsonBuffer["TimeSync"]["sec"] = now.tm_sec;
    // jsonBuffer["TimeSync"]["min"] = now.tm_min;
    // jsonBuffer["TimeSync"]["hour"] = now.tm_hour;
    // jsonBuffer["TimeSync"]["day"] = now.tm_mday;
    // jsonBuffer["TimeSync"]["mon"] = now.tm_mon;
    // jsonBuffer["TimeSync"]["year"] = now.tm_year;
    // jsonBuffer["TimeSync"]["dst"] = now.tm_isdst;
    // };
    // settimeofday();
  // }
}

//check button -----------------------------------------------------------------------------------------------
void checkButton()
{
  // Check for button press
  if (digitalRead(TRIGGER_PIN) == LOW)
  {
    // Poor-man's debounce/press-hold, code not ideal for production
    delay(50);
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
      Serial.println("Button Pressed");
      // Still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000); // Reset delay hold
      if (digitalRead(TRIGGER_PIN) == LOW)
      {
        Serial.println("Useraction:Button held.  System: Erasing wifi cerdentials and restarting.");
        //need something to reset wifi//wm.resetSettings();
        preferences.begin("credentials", false);
        preferences.putString("ssid", "");
        preferences.putString("password", "");
        preferences.clear();
        preferences.end();
        ESP.restart();
      }

      // Start portal w/delay
      Serial.println("Starting config portal.");
    }
  }
}

// Function that gets current epoch time
unsigned long getTime()
{
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    //Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

//Function that gets current time in human format
String getHumanTime()
{
  //String buff;
  unsigned long t = getTime();
  char buff[32];
  sprintf(buff, "%02d.%02d.%02d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t));

  return buff;
}

//Function to get the topic
const char *returnTopic()
{
  String topic = DEVICEINFORMATION.companyName + "/" + DEVICEINFORMATION.siteName + "/" +
                 DEVICEINFORMATION.group + "/" + DEVICEINFORMATION.deviceID + "/" +
                 DEVICEINFORMATION.deviceName;

  //for debug use: print out the report topic
  //Serial.print("MQTT report Topic: ");
  //Serial.print(topic);
  //Serial.println();

  const char *msg_const_char = topic.c_str();
  //free(topic);
  return msg_const_char;
}

//MQTT server will send message to here
//message type
//1. DeviceCMD for issue command to the device
//2. NodeList for return the current nodelist, the limit is less than 10
// function for the subscribereceive---------------------------------------------------

void subscribeReceive(char *topic, byte *payload, unsigned int length)
{
  String msg;
  // Print the topic
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print(" ");
  // Print the message
  for (int i = 0; i < length; i++)
  {
    Serial.print(char(payload[i]));
    msg += char(payload[i]);
  }

  // Print a newline
  Serial.print("Message: ");
  Serial.println(msg);

  //when MQTT broker recevied a command will send to device
  DynamicJsonDocument JSONmessage(ESP.getMaxAllocHeap());
  DeserializationError error = deserializeJson(JSONmessage, msg);
  if (error)
  {
    Serial.print(F("subscribeReceive deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  //for issue the device cmd
  if (JSONmessage["DeviceCMD"])
  {
    mesh.sendSingle(JSONmessage["DeviceCMD"]["deviceID"], msg);
    Serial.print("Device CMD recevied form MQTT target device ");
    Serial.print(JSONmessage["DeviceCMD"]["deviceID"].as<String>());
    Serial.print(" message ");
    Serial.println(msg);
  }

  //for getting the nodelist
  if (JSONmessage["NodeList"])
  {
    //initize full path
    std::string fullPath = returnTopic();
    //get the current node list
    std::list<uint32_t> l = mesh.getNodeList();

    //create the topic
    fullPath += "/NodeList";
    const char *foo = fullPath.c_str();

    //init the counter, page number, and get the length of the JSON message
    int counter = 0;
    int listLimit = JSONmessage["NodeList"]["listLimit"].as<int>();
    int page = 1;
    String msg;

    //for print out all mesh node
    for (int n : l)
    {

      if (counter < listLimit)
      {
        //add counter and stay in same page
        counter++;
      }
      else if (counter >= listLimit)
      {
        //print the page number at the end
        msg += ":page";
        msg += page;
        //print to next line and add a new page
        page++;
        counter = 0;
        //for debug print out the seperate msg
        Serial.print(msg);
        Serial.println();

        //publish to the mqtt server and print out result
        mqttClient.publish(foo, msg.c_str());
        Serial.print("Publish to MQTT server: ");
        Serial.println(msg.c_str());

        msg = "";
      }

      msg += n;
      msg += ":";
    }

    //print the page number at the end
    msg += ":page";
    msg += page;
    //for debug print out the seperate msg
    Serial.print(msg);
    Serial.println();

    //publish to the mqtt server and print out result
    mqttClient.publish(foo, msg.c_str());
    Serial.print("Publish to MQTT server: ");
    Serial.println(msg.c_str());
    //for print out all mesh node
  }
}

//-------------------------------------------------------------------------
//Task for the senor
//1. LUX senor
//2. PIR senor
//3. PIRsenorReport
//4. sendSensorLog
//5. InternalTempsensor
//6. nodeStatusReport
//7. newConnectionCallback (for plainlessmesh library)
//8. changedConnectionCallback (for plainlessmesh library)
//9. nodeTimeAdjustedCallback (for plainlessmesh library)
//10. sendBroadcastRootIDMessage
//11. sendBroadcastDeviceInformationMessage
//-------------------------------------------------------------------------

//Task for LUXsenor
void LUXsenor()
{
  if (lightMeter.measurementReady())
  {
    //for the light meter lux value
    float lux = lightMeter.readLightLevel();

    //for the change flag for change
    //boolean change;

    //counter
    //int counter = 5;
    Serial.print("LUXmeter start Light: ");
    Serial.print(lux);
    Serial.println(" lx");

    //send out lux report
    DynamicJsonDocument jsonBuffer(258);
    //JsonObject& msg = jsonBuffer. createObject () ;
    jsonBuffer["LUXmeter"]["NodeID"] = mesh.getNodeId();
    jsonBuffer["LUXmeter"]["Lux"] = String(lux);
    String msg;
    serializeJson(jsonBuffer, msg);
    sendSensorLog(msg);
    //for normal lux output and output as the brightness of the led
    //FastLED.setBrightness(lux);
    //FastLED.show();

    //store the old value
    lux_mem = lux;

    //store in the NODESTATUS
    NODESTATUS.lux_meter = lux;
  }
}

//gobal variable for the PIRsenor counter
int PIRsenor_counter;
int PIRsenor_oldQuotatient;
int PIRsenor_newQuotatient;
int PIRsenor_reportCounter;

long unsigned int PIRsenor_timeStamp[60];

//Task for PIRsenor
void PIRsenor()
{
  Serial.println("PIRsenor start");

  val = digitalRead(inputPin); // read input value

  if (digitalRead(inputPin) == HIGH)
  {
    //Serial.println("input pin == HIGH");

    if (lockLow)
    {
      lockLow = false;
      Serial.print("motion detected at ");
      Serial.print(millis() / 1000);
      Serial.println(" sec");
      delay(50);
      //open the led to red
      leds[0] = CRGB(255, 0, 0);
      NODESTATUS.colour[0] = 255;
      NODESTATUS.colour[1] = 0;
      NODESTATUS.colour[2] = 0;
      NODESTATUS.lightONOFF = true;
      FastLED.setBrightness(brightness_init);
      FastLED.show();

      //this is used to detect the PIR senor trigger in one min
      //set counter to renew if the quotient is increase

      PIRsenor_oldQuotatient = PIRsenor_newQuotatient;
      PIRsenor_newQuotatient = int(millis() / 1000 / 60);
      Serial.print("old quotatient: ");
      Serial.println(PIRsenor_oldQuotatient);
      Serial.print("new quotatient: ");
      Serial.println(PIRsenor_newQuotatient);
      Serial.print("last session counter: ");
      Serial.println(PIRsenor_counter);

      if ((PIRsenor_newQuotatient - PIRsenor_oldQuotatient) >= 1)
      {
        //reset counter
        PIRsenor_counter = 1;
        //reset PIRsenor_reportCounter
        PIRsenor_reportCounter = 1;
        DynamicJsonDocument PIRsenorTimeStampDoc(1024);
        JsonObject root = PIRsenorTimeStampDoc.to<JsonObject>();
        root["PIRsensorReport"]["NodeID"] = mesh.getNodeId();
        //JsonArray nodeID = root.createNestedArray("NodeID");
        //nodeID.add(mesh.getNodeId());
        JsonArray timeStamp = root.createNestedArray("PIRsensorReporttimeStamp");
        int i = 0;
        while (i < 60)
        {
          timeStamp.add(PIRsenor_timeStamp[i]);
          i++;
        }
        String msg;
        serializeJson(PIRsenorTimeStampDoc, msg);
        sendSensorLog(msg);
      }
      else
      {
        PIRsenor_counter++;

        if (PIRsenor_reportCounter < 60)
        {
          PIRsenor_timeStamp[PIRsenor_reportCounter] = millis();
          PIRsenor_reportCounter++;
        }
      }
    }
    takeLowTime = true;
  }

  if (digitalRead(inputPin) == LOW)
  {
    //Serial.println("input pin == LOW"); //the led visualizes the sensors output pin state

    if (takeLowTime)
    {
      lowIn = millis();    //save the time of the transition from high to LOW
      takeLowTime = false; //make sure this is only done at the start of a LOW phase
    }
    //if the sensor is low for more than the given pause,
    //we assume that no more motion is going to happen
    if (!lockLow && millis() - lowIn > pause_counter)
    {
      //makes sure this block of code is only executed again after
      //a new motion sequence has been detected
      lockLow = true;
      Serial.print("motion ended at "); //output
      Serial.print((millis() - pause_counter) / 1000);
      Serial.println(" sec");
      delay(50);
      //open the led to red
      leds[0] = CRGB(0, 255, 0);
      NODESTATUS.colour[0] = 0;
      NODESTATUS.colour[1] = 255;
      NODESTATUS.colour[2] = 0;
      NODESTATUS.lightONOFF = true;
      FastLED.setBrightness(brightness_init);
      FastLED.show();
    }
  }
}

void PIRsenorReport()
{
  //send out JSON report
  DynamicJsonDocument jsonBuffer(1024);
  //JsonObject& msg = jsonBuffer. createObject () ;
  jsonBuffer["PIRsensor"]["NodeID"] = mesh.getNodeId();
  jsonBuffer["PIRsensor"]["PIRcounter"] = String(PIRsenor_counter) + " times/min";
  jsonBuffer["PIRsensor"]["Seconds"] = int(millis() / 1000);
  String msg;
  serializeJson(jsonBuffer, msg);
  sendSensorLog(msg);
  //store the value of pir senor
  NODESTATUS.pir_counter = PIRsenor_counter;

  PIRsenor_counter = 0;
}

void sendSensorLog(String _msg)
{
  //decide send to root node or not
  if (!root_ID)
  {
    mesh.sendBroadcast(_msg);
    Serial.println("Broadcast log: " + _msg);
  }
  else
  {
    mesh.sendSingle(root_ID, _msg);
    Serial.print("send to ");
    Serial.print(root_ID);
    Serial.print(" : ");
    Serial.println(_msg);
  }
}

void sendMessageToSingleNode(String _msg)
{
  DynamicJsonDocument JSONmessage(1024);
  DeserializationError error = deserializeJson(JSONmessage, _msg);
  if (error)
  {
    Serial.print(F("subscribeReceive deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  Serial.print("sendMessageToSingleNode(String _msg): Node: ");
  Serial.println(JSONmessage["DeviceCMD"]["deviceID"].as<int>());
  Serial.print(" : ");
  Serial.println(_msg);
  mesh.sendSingle(JSONmessage["DeviceCMD"]["deviceID"].as<int>(), _msg);
  //mesh.sendBroadcast(_msg);
  //Serial.println("Broadcast log: " + _msg);
}

//for getting the internal temp sensor value
void InternalTempsensor()
{
  int internalTemp;
  Serial.print("Temperature: ");
  Serial.print((temprature_sens_read() - 32) / 1.8);
  Serial.println(" C");

  internalTemp = (temprature_sens_read() - 32) / 1.8;
  NODESTATUS.internalTemp = internalTemp;
}

//Task for sending node status
void nodeStatusReport()
{
  const int capacity = 10 * JSON_OBJECT_SIZE(1);

  //send out lux report
  DynamicJsonDocument jsonDoc_nodeStatus(capacity);
  //JsonObject& msg = jsonBuffer. createObject () ;
  jsonDoc_nodeStatus["NodeStatus"]["NodeID"] = mesh.getNodeId();
  jsonDoc_nodeStatus["NodeStatus"]["Luxmeter"] = int(NODESTATUS.lux_meter);
  jsonDoc_nodeStatus["NodeStatus"]["PIRcounter"] = int(NODESTATUS.pir_counter);
  jsonDoc_nodeStatus["NodeStatus"]["lightONOFF"] = bool(NODESTATUS.lightONOFF);
  jsonDoc_nodeStatus["NodeStatus"]["lightLevel"] = int(NODESTATUS.lightLevel);
  jsonDoc_nodeStatus["NodeStatus"]["colour0"] = int(NODESTATUS.colour[0]);
  jsonDoc_nodeStatus["NodeStatus"]["colour1"] = int(NODESTATUS.colour[1]);
  jsonDoc_nodeStatus["NodeStatus"]["colour2"] = int(NODESTATUS.colour[2]);
  jsonDoc_nodeStatus["NodeStatus"]["internalTemp"] = int(NODESTATUS.internalTemp);
  String msg;
  serializeJson(jsonDoc_nodeStatus, msg);
  sendSensorLog(msg);
}

// Needed for painless library
void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

// Needed for painless library
void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}
// Needed for painless library
void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

//Task for send broadcast message
void sendBroadcastMessage(String msg)
{
  mesh.sendBroadcast(msg);
  Serial.println("sendBroadcastMessage(String msg): " + msg);
}

//Task for send root node id
void sendBroadcastRootIDMessage()
{
  String msg;
  DynamicJsonDocument jsonBuffer(128);

  jsonBuffer["RootID"] = mesh.getNodeId(); //{"RootID":2125252129}
  serializeJson(jsonBuffer, msg);
  mesh.sendBroadcast(msg);
  Serial.println("sendBroadcastRootIDMessage() Broadcast log (Root ID): " + msg);
}

void sendBroadcastNodeIDMessage()
{
  String msg;
  DynamicJsonDocument jsonBuffer(128);

  jsonBuffer["NodeID"] = mesh.getNodeId(); //{"RootID":2125252129}
  serializeJson(jsonBuffer, msg);
  mesh.sendBroadcast(msg);
  Serial.println("sendBroadcastNodeIDMessage() Broadcast log (Node ID): " + msg);
}

    //Task for send device information
    void
    sendBroadcastDeviceInformationMessage()
{
  String msg;
  DynamicJsonDocument jsonDeviceInformation(1024);

  jsonDeviceInformation["deviceInformation"]["companyName"] = DEVICEINFORMATION.companyName;
  jsonDeviceInformation["deviceInformation"]["siteName"] = DEVICEINFORMATION.siteName;
  jsonDeviceInformation["deviceInformation"]["group"] = DEVICEINFORMATION.group;
  jsonDeviceInformation["deviceInformation"]["deviceName"] = DEVICEINFORMATION.deviceName;

  serializeJson(jsonDeviceInformation, msg);
  mesh.sendBroadcast(msg);
  Serial.println("Broadcast log: " + msg);
}

//RBG Brightness setting function

//float old_power = 0;

//int RGBbrightness_R;
//int RGBbrightness_G;
//int RGBbrightness_B;
//float power;

void RGBbrightness()
{
  if (RGBbrightness_power > RGBbrightness_old_power)
  {
    Serial.println("RGBbrightness(): Stepping up power");
    Serial.print("RGBbrightness(): Power: ");
    Serial.print(RGBbrightness_old_power);
    Serial.print("RGBbrightness(): ");
    Serial.print(RGBbrightness_R);
    Serial.print(" ");
    Serial.print(RGBbrightness_G);
    Serial.print(" ");
    Serial.println(RGBbrightness_B);
    // brightness level need to increase
    while (RGBbrightness_old_power < 1 && RGBbrightness_old_power < RGBbrightness_power)
    {
      ledcWrite(ledChannel_R, 255 - (RGBbrightness_R * RGBbrightness_old_power));
      ledcWrite(ledChannel_G, 255 - (RGBbrightness_G * RGBbrightness_old_power));
      ledcWrite(ledChannel_B, 255 - (RGBbrightness_B * RGBbrightness_old_power));
      RGBbrightness_old_power += 0.01;
      Serial.println(RGBbrightness_old_power);
      delay(50);
    }
  }
  else if (RGBbrightness_power < RGBbrightness_old_power)
  {
    Serial.println("RGBbrightness(): Stepping down power");
    Serial.print("RGBbrightness(): Power: ");
    Serial.print(RGBbrightness_old_power);
    Serial.print("RGBbrightness(): ");
    Serial.print(RGBbrightness_R);
    Serial.print(" ");
    Serial.print(RGBbrightness_G);
    Serial.print(" ");
    Serial.println(RGBbrightness_B);

    // brightness level need to decrease
    while (RGBbrightness_old_power > 0 && RGBbrightness_old_power > RGBbrightness_power)
    {
      ledcWrite(ledChannel_R, 255 - (RGBbrightness_R * RGBbrightness_old_power));
      ledcWrite(ledChannel_G, 255 - (RGBbrightness_G * RGBbrightness_old_power));
      ledcWrite(ledChannel_B, 255 - (RGBbrightness_B * RGBbrightness_old_power));
      RGBbrightness_old_power -= 0.01;
      Serial.println(RGBbrightness_old_power);
      delay(50);
    }
  }
  else
  {
    Serial.println("RGBbrightness(): Same power");
    Serial.print("RGBbrightness(): Power: ");
    Serial.print(RGBbrightness_old_power);
    Serial.print("RGBbrightness(): ");
    Serial.print(RGBbrightness_R);
    Serial.print(" ");
    Serial.print(RGBbrightness_G);
    Serial.print(" ");
    Serial.println(RGBbrightness_B);

    //if the brightness level unchange check if need colour change
    ledcWrite(ledChannel_R, 255 - (RGBbrightness_R * RGBbrightness_old_power));
    ledcWrite(ledChannel_G, 255 - (RGBbrightness_G * RGBbrightness_old_power));
    ledcWrite(ledChannel_B, 255 - (RGBbrightness_B * RGBbrightness_old_power));
  }

  //RGBbrightness_old_power = RGBbrightness_power;
  //disable this extra task for release memory
  //taskRGBbrightness.disable();
}


// void configTimeServer(){
//   //Time server configuration
//   configTime(utcOffset, daylightoffset, ntpServer1, ntpServer2, ntpServer3);
//   struct tm now;
//   if(!getLocalTime(&now)){
//     Serial.println("cannot get time~");
//     return;
//   } else {

//     String msg;
//     DynamicJsonDocument jsonBuffer(512);

//     jsonBuffer["TimeSync"]["sec"] = now.tm_sec;
//     jsonBuffer["TimeSync"]["min"] = now.tm_min;
//     jsonBuffer["TimeSync"]["hour"] = now.tm_hour;
//     jsonBuffer["TimeSync"]["day"] = now.tm_mday;
//     jsonBuffer["TimeSync"]["mon"] = now.tm_mon;
//     jsonBuffer["TimeSync"]["year"] = now.tm_year;
//     jsonBuffer["TimeSync"]["dst"] = now.tm_isdst;
//     serializeJson(jsonBuffer, msg);

//     sendBroadcastMessage(msg);
//   }

//   Serial.println(&now, "%Y/%m/%d %H:%M:%S");

// }
