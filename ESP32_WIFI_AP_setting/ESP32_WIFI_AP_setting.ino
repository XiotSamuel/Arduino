#include <WiFi.h>

IPAddress local_IP(192,168,4,22); 
IPAddress gateway(192,168,4,22); 
IPAddress subnet(255,255,255,0); 
const char *ssid = "ESP32_AP_TEST"; 
const char *password = "12345678";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  Serial.println(); 
  WiFi.mode(WIFI_AP); //設置工作在AP模式 
  WiFi.softAPConfig(local_IP, gateway, subnet); //設置AP地址 
  while(!WiFi.softAP(ssid, password)){}; //啟動AP 
  Serial.println("AP啟動成功"); 
  Serial.print("IP address: "); 
  Serial.println(WiFi.softAPIP()); // 列印IP位址 
  WiFi.softAPsetHostname("myHostName"); //設置主機名 
  Serial.print("HostName: "); 
  Serial.println(WiFi.softAPgetHostname()); //列印主機名 
  Serial.print("mac Address: "); 
  Serial.println(WiFi.softAPmacAddress()); //列印mac地址

}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(1000); 
  //Serial.println(WiFi.softAPgetStationNum()); //列印客戶端連接數

}
