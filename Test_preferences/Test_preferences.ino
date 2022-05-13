#include <Preferences.h>

Preferences preferences;

void setup() {
  Serial.begin(115200);
  Serial.println();

  preferences.begin("my-app", false);

  String key ="";
  String items = "Item_no_";
  int count =0;
  unsigned int value =0;

  for(count=0; count < 5; count++) {
    key = items+String(count);
    preferences.putUInt(key, count);
  }

  for(count=0;count < 5; count++) {
    key = items+String(count);
    Serial.print("key : "+key);
    value = preferences.getUInt(key, 0);
    Serial.printf(" value: %u\n", value);
    Serial.println();
  }
  // Close the Preferences
  preferences.end();

  // Wait 10 seconds
  delay(10000);
}

void loop() {}
