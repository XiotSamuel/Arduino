#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
 
const char* ssid = "Samuel";
const char* password =  "Samuel@1986";
 
AsyncWebServer server(80);
 
const char HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h1>wifi setup<h1/>
  <form action="/get">
    ssid: <input type="text" name="ssid">
    password: <input type="text" name="password">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void setup(){
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
 
  server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    String ssid_char;
    String password_char;

    if(request->hasParam("ssid") && request->hasParam("password"))
    {
          ssid_char = request->getParam("ssid")->value();
          password_char = request->getParam("password")->value();
          Serial.println("the condition meet");
          Serial.print("ssid: ");
          Serial.println(ssid_char);
          Serial.print("password: ");
          Serial.println(password_char);
    }

    //Serial.println();
    //Serial.println(request->getParam(ssid)->value());
    request->send(200, "text/html", HTML);
  });

  //server.on("/htmllalala", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("this is the test page");
    //Serial.println();
    //Serial.println(request->getParam(ssid)->value());
    //request->send(200, "text/html", HTML);
  //});
 
  server.begin();
}
 
void loop(){
}
