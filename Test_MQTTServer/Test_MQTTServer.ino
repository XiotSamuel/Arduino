
#include <WiFi.h>

//#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include "DHT.h"


const char* ssid = "Samuel";
const char* password = "Samuel@1986";
const char* mqtt_server = "test.mosquitto.org"; //<-- IP หรือ Domain ของ Server MQTT

long lastMsg = 0;
char msg[100];
int value = 0;
String DataString;

#define LED_PIN 27  //ขาที่ต่อ LED
#define DHTPIN 19   //ขาที่ต่อเซนเซอร์ DHT22
#define DHTTYPE DHT22
#define BUILTIN_LED 27

WiFiClient espClient;
DHT dht(DHTPIN, DHTTYPE);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);
    digitalWrite(LED_PIN, HIGH);

  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    digitalWrite(LED_PIN, LOW);
  }

}


PubSubClient client(mqtt_server, 1883, callback, espClient);


void setup() {


  pinMode(LED_PIN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  dht.begin();

  Serial.begin(115200);

  setup_wifi();

  client.connect("Node1", "joe1", "joe1");
  client.setCallback(callback);
  client.subscribe("command");

}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// Procedure reconnect ใช้กรณีที่บางทีเราหลุดออกจาก Network แล้วมีการต่อเข้าไปใหม่อีกครั้ง

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 


  long now = millis();
  if (now - lastMsg > 5000) //เช็คระยะเวลาล่าสุดที่ส่งข้อความ Publish ออกไป ถ้าเกิน 5 วิก็ส่งออกไปใหม่
  {
    lastMsg = now;
    value++;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    DataString = "Temp: "+String(t)+" Humidity: "+String(h);

    DataString.toCharArray(msg, 100);

    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("env", msg);  //ส่งข้อความ Temp + Humidity ออกไปที่ Topic "env"
  }
}
