
//for plainlessMesh
#include <painlessMesh.h>

//for the task ArduinoJson.h
#include <ArduinoJson.h>

//for the FastLED
#include <FastLED.h>

// for the BH1750 light senor
#include <BH1750.h>
#include <Wire.h>

//for web server
#include <WebServer.h>
#include <ESPmDNS.h>

#define NUM_LEDS 1
#define DATA_PIN 27
CRGB leds[NUM_LEDS];

//for the web server configuration
WebServer server(80);

/* Style */
String style =
    "<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
    "input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
    "#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
    "#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
    "form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
    ".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex = 
    "<form name=loginForm>"
    "<h1>ESP32 Login</h1>"
    "<input name=userid placeholder='User ID'> "
    "<input name=pwd placeholder=Password type=Password> "
    "<input type=submit onclick=check(this.form) class=btn value=Login></form>"
    "<script>"
    "function check(form) {"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{window.open('/serverIndex')}"
    "else"
    "{alert('Error Password or Username')}"
    "}"
    "</script>" + style;
 
/* Server Index Page */
String serverIndex = 
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
    "<label id='file-input' for='file'>   Choose file...</label>"
    "<input type='submit' class=btn value='Update'>"
    "<br><br>"
    "<div id='prg'></div>"
    "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
    "<script>"
    "function sub(obj){"
    "var fileName = obj.value.split('\\\\');"
    "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
    "};"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    "$.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "$('#bar').css('width',Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!') "
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>" + style;

//painlessMesh parameter
//#define   MESH_PREFIX     "whateverYouLike"
//#define   MESH_PASSWORD   "somethingSneaky"
//#define   MESH_PORT       5555

#define   MESH_PREFIX     "Samuel"
#define   MESH_PASSWORD   "Samuel@1986"
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

    Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  ts.execute();  
  mesh.update();

  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);
}
