

/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include <painlessMesh.h>



// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";


const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input2: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

//for the web server
#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

//AsyncWebServer server(80);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
//String output26State = "off";
//String output27State = "off";

// Assign output variables to GPIO pins
//const int output26 = 26;
//const int output27 = 27;

//object for the scheduler
//Scheduler ts;


//scheduler protype
//void webServer();

//Task object for the web server
//Task taskWebServer(TASK_SECOND *1, TASK_FOREVER, &webServer, &ts);

//void webServer(){
//  Serial.printf("task web server\n");
//}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  //pinMode(output26, OUTPUT);
  //pinMode(output27, OUTPUT);
  // Set outputs to LOW
  //digitalWrite(output26, LOW);
  //digitalWrite(output27, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress Ip(192, 168, 10, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
                  + inputParam + ") with value: " + inputMessage +
                  "<br><a href=\"/\">Return to Home Page</a>");
  });

  server.onNotFound(notFound);
  server.begin();


  //for the task of web server
  //ts.addTask(taskWebServer);
  //taskWebServer.enable();

}

void loop() {
  /*
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            /*
              // turns the GPIOs on and off
              if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
              } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
              } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
              } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
              }
            */
            /*
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Please enter the home wifi name and password</h1>");

            //form for the input
            client.println("<form>");
            client.println("<label for=\"username\">User Name:</label><br>");
            client.println("<input type=\"text\" id=\"username\"><br>");
            client.println("<label for=\"password\">Password:</label><br>");
            client.println("<input type=\"text\" id=\"password\"><br>");
            client.println("<input type=\"submit\" value=\"Submit\"><br>");
            client.println("</form>");

            // Display current state, and ON/OFF buttons for GPIO 26
            //client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button
            /*
              if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
              } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
            */
            // Display current state, and ON/OFF buttons for GPIO 27
            //client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button
            /*if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
              } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
              }*/
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();

            /*
              if(request->hasArg("SUBMIT")){
              Serial.println("submit button click");
              }*/
/*
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");

  }*/
}
