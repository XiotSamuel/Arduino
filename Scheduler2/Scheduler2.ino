//for plainlessMesh
#include <painlessMesh.h>


//for the task scheduler
//#include <TaskScheduler.h>

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
/*
 * PIR sensor tester
 */

//painlessMesh parameter
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

painlessMesh  mesh;//object for the painless mesh

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause_counter = 5000;

boolean lockLow=true;
boolean takeLowTime;

int ledPin = 27;                // choose the pin for the LED
int inputPin = 19;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status


//gobal variable for the brightness and the lux
int brightness_init=200;
int brightness_mem;
int lux_init=lightMeter.readLightLevel();
int lux_mem;

//gobal variable for colour setting
int led_red=0;
int led_green=0;
int led_blue=0;

//List for the nodes
SimpleList<uint32_t> nodes;

//gobal variable for the JSON doc nodeID
StaticJsonDocument<50> nodeID;

//object for the scheduler
Scheduler ts;

//scheduler protype
void PIRsenor();
void LUXsenor();

//painlessmesh task
void sendMessage(); 

//Task object timing configuration
Task taskPIRsenor(TASK_SECOND *1, TASK_FOREVER, &PIRsenor, &ts);
Task taskLUXsenor(TASK_SECOND *1, TASK_FOREVER, &LUXsenor, &ts);

//Task object painlessmesh
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

//Task sendBroadcastNodeIDMessage
void sendBroadcastNodeIDMessage();
Task tasksendBroadcastNodeIDMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendBroadcastNodeIDMessage );


//Task for sendBroadcastNodeIDMessage
void sendBroadcastNodeIDMessage() {
    //get the node ID at setup
  String msg;
  nodeID["nodeID"] = mesh.getNodeId();
  serializeJson(nodeID, msg);
  Serial.println();
  mesh.sendBroadcast( msg );

  

  //send single message for testing
  mesh.sendSingle(2125304809, "this is a single message for 2125304809");
}

//Task for sendMessage
void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
// New function for the node id message
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("receivedCallback: Received from %u msg=%s\n", from, msg.c_str());

  //string for compare
  //nodeID message {"nodeID":
  String nodeIDmessage ="{\"nodeID\"";
  StaticJsonDocument<50> JSONmessage;

  //if the message contain the nodeid string
  if (strstr(msg.c_str(),nodeIDmessage.c_str())){
    
      Serial.println("I have a nodeIDmessage");

      //check the recevied message
      DeserializationError error = deserializeJson(JSONmessage, msg);
      
      if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

      //nodeID int
      int nodeID=JSONmessage["nodeID"];

      Serial.printf("the node list ");
      SimpleList<uint32_t>::iterator node = nodes.begin();
      while (node != nodes.end()) {
          Serial.printf(" %u", *node);
          node++;
        }
      Serial.println(" ");


      //list out all the existing node
      String message;
      nodes = mesh.getNodeList();
      
      node = nodes.begin();
      while (node != nodes.end()) {
              message += *node;
              message += " | ";
              node++;
            }
              message +="";
      message +="<br>"; 
      Serial.println(message);

     }
    
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


//Task for LUXsenor
void LUXsenor(){
  if (lightMeter.measurementReady()) {
    //for the light meter lux value
    float lux = lightMeter.readLightLevel();
    
    //for the change flag for change
    boolean change;

    //counter
    int counter =5;
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    //change to a meaningful value
    if (lux_mem-lux>0){
      if (counter>0){
        Serial.println("lux decrease");
        //condition brightness mem need to less than 255 to continual
        if (brightness_mem <255){
          brightness_mem = brightness_mem + 10;}
        //for print out brightness level
        Serial.println("brightness set to");
        Serial.println(brightness_mem);
        //setted the fastled brightness_mem
        FastLED.setBrightness(brightness_mem);
        FastLED.show();
        counter=counter-1;
      }
    }else if (lux_mem-lux<0){
      if (counter>0){
        Serial.println("lux increase");
        //conditional brightness mem need to more then 0 to continual
        if (brightness_mem > 0){
          brightness_mem = brightness_mem - 10;}
        //for print out brightness level
        Serial.println("brightness set to");
        Serial.println(brightness_mem);
        FastLED.setBrightness(brightness_mem);
        FastLED.show();
        counter=counter-1;
      }
    }else {
      Serial.println("lux unchange");
    }
    //lux_init=lux;
    //brightness_init=brightness_init-1;
    // Serial.println(brightness_init);
    
    //if (brightness=brightness-lux){
    //    FastLED.setBrightness(brightness);
    //    FastLED.show();}
    lux_mem=lux;
  }
}

//Task for PIRsenor
void PIRsenor(){
  Serial.println("PIRsenor start");

   val = digitalRead(inputPin);  // read input value

  if(digitalRead(inputPin)== HIGH){
    //Serial.println("input pin == HIGH");

    if(lockLow){
      lockLow=false;
      Serial.print("motion detected at ");
      Serial.print(millis()/1000);
      Serial.println(" sec"); 
      delay(50);
      //open the led to red
      leds[0] = CRGB(255,0,0);
      FastLED.setBrightness(brightness_init);
      FastLED.show();
    }
    takeLowTime = true;
  }

  if(digitalRead(inputPin) == LOW){       
       //Serial.println("input pin == LOW"); //the led visualizes the sensors output pin state

       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause, 
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause_counter){  
           //makes sure this block of code is only executed again after 
           //a new motion sequence has been detected
           lockLow = true;                        
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause_counter)/1000);
           Serial.println(" sec");
           delay(50);
           //open the led to red
           leds[0] = CRGB(0,255,0);
           FastLED.setBrightness(brightness_init);
           FastLED.show();
           }
       }
      // brightness_init=brightness_init-1;
   //Serial.println(brightness_init);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //ts.addTask(taskPIRsenor);
  ts.enableAll();

  //pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input PIR senor AM312

  // init Fastled
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(10);
  // Clear the led 
  leds[0] = CRGB::Black;
  FastLED.show();

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin(33,22);
  
  //setup the LUX meter and print out the result
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

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

  //for enable tasksendBroadcastNodeIDMessage
  ts.addTask( tasksendBroadcastNodeIDMessage );
  tasksendBroadcastNodeIDMessage.enable();

  //For checking lux init
  //Serial.print("lux_init is ");
  //Serial.print(lux_init);
  //Serial.println();
  //Serial.begin(115200);


  
}

void loop() {

  ts.execute();
  mesh.update();
}
