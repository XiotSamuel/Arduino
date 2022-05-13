/* * EEPROM Clear

*

* Sets all of the bytes of the EEPROM to 0.

* This example code is in the public domain.

*/

#include <EEPROM.h>


int addr = 0;
#define EEPROM_SIZE 64

void setup()

{

// 讓EEPROM的512位元組內容全部清零

for (int i = 0; i < EEPROM_SIZE; i++)

EEPROM.write(i, 0);

// 清零工作完成後，將L燈點亮，提示EEPROM清零完成
Serial.print("clean complete");

  // advance to the next address.  there are 512 bytes in
  // the EEPROM, so go back to 0 when we hit 512.
  // save all changes to the flash.
  addr = addr + 1;
  if (addr == EEPROM_SIZE)
  {
    Serial.println();
    addr = 0;
    EEPROM.commit();
    Serial.print(EEPROM_SIZE);
    Serial.println(" bytes written on Flash . Values are:");
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
      Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
    }
    Serial.println(); Serial.println("----------------------------------");
  }

}

void loop()

{

}
