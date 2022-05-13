//#include <ESP8266WiFi.h>
#include <WiFi.h> //ESP32

#include <ESPForm.h>

//Your WiFi SSID and Password
#define WIFI_SSID "Samuel"
#define WIFI_PASSWORD "Samuel@1986"

//Raw content of index.html in plain text
static const char *index_html = R"--espform--(
<!DOCTYPE html>
<html lang="en">

<head>
  <title> Simple Textbox </title>
  <style>
body {
    padding: 20px;
  }
  
  label {
    font-size: 17px;
    font-family: sans-serif;
  }
  
  input {
    display: block;
    width: 300px;
    height: 40px;
    padding: 4px 10px;
    margin: 10 0 10 0;
    border: 1px solid #03A9F4;
    background: #cce6ff;
    color: #1c87c9;
    font-size: 17px;
  }
}
  </style>
</head>

<body>
  <form>
    <label for="text1">Value to device</label>
    <input type="text" id="text1" name="text1" value="Change me..."/>
    <label for="text2">Value from device</label>
    <input type="text" id="text2" name="text2" /> </form>
</body>

</html>
)--espform--";

unsigned long prevMillis = 0;
unsigned long serverTimeout = 2 * 60 * 1000;

void formElementEventCallback(HTMLElementItem element)
{
  Serial.println();
  Serial.println("***********************************");
  Serial.println("id: " + element.id);
  Serial.println("value: " + element.value);
  Serial.println("type: " + element.type);
  Serial.println("event: " + ESPForm.getElementEventString(element.event));
  Serial.println("***********************************");
  Serial.println();
}

void serverTimeoutCallback()
{

  //If server timeout (no client connected within specific time)
  Serial.println("***********************************");
  Serial.println("Server Timeout");
  Serial.println("***********************************");
  Serial.println();
}

void setup()
{

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  //For STA only or AP + STA mode
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Add the html contents (in html.h) for the web page rendering
  ESPForm.addFileData(index_html, "index.html");

  //Add html element event listener, id "text1" for onchange event
  ESPForm.addElementEventListener("text1", ESPFormClass::EVENT_ON_CHANGE);

  //Start ESPForm's Webserver
  ESPForm.begin(formElementEventCallback, serverTimeoutCallback, serverTimeout, true);

  ESPForm.startServer();

  Serial.println();
  Serial.println("=================================================");
  Serial.println("Use web browser and navigate to " + WiFi.localIP().toString());
  Serial.println("=================================================");
  Serial.println();
}

void loop()
{
  //If a client existed
  if (ESPForm.getClientCount() > 0)
  {

    if (millis() - prevMillis > 1000)
    {
      prevMillis = millis();
      //The event listener for text2 is not set because we don't want to listen to its value changes
      ESPForm.setElementContent("text2", String(millis()));
    }
  }
}
