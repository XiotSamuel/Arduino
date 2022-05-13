/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-save-data-permanently-preferences/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <Preferences.h>

Preferences preferences;

const char* ssid = "Samuel";
const char* password = "Samuel@1986";

const char* ssid1 = "Samuel12345";
const char* password1 = "Samuel@12345";

void setup() {
  Serial.begin(115200);
  Serial.println();

  preferences.begin("credentials", false);
  preferences.putString("ssid", ssid); 
  preferences.putString("password", password);

  Serial.println("Network Credentials Saved using Preferences");

  preferences.end();

  preferences.begin("credentials", false);
  preferences.putString("ssid", ssid1); 
  preferences.putString("password", password1);

  Serial.println("Network Credentials Saved using Preferences");

  preferences.end();
}

void loop() {

}
