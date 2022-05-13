#include <Wire.h> //BH1750 IIC Mode
#include <math.h>
int Sen1 = 0x23; //setting i2c address
int Sen2 = 0x5C;

byte buff[2];

void setup()
{
    Wire.begin();
    Serial.begin(57600);//init Serial baud rate
}

void loop()
{
    uint16_t val1=0, val2=0;
    
    BH1750_Init(Sen1);
    delay(200);
    
    if(2==BH1750_Read(Sen1))
    {
    val1=((buff[0]<<8 )|buff[1])/1.2;
    Serial.print(val1,DEC);
    Serial.println("[lx]");
    }
    delay(200);
    
    BH1750_Init(Sen2);
    delay(200);
    
    if(2==BH1750_Read(Sen2))
    {
    val2=((buff[0]<<8 )|buff[1])/1.2;
    Serial.print(val2,DEC);
    Serial.println("[lx]");
    }
    delay(200);
}

int BH1750_Read(int address) //
{
    int i=0;
    Wire.beginTransmission(address);
    Wire.requestFrom(address, 2);
    while(Wire.available()) //
    {
    buff = Wire.read(); // receive one byte
    i++;
    }
    Wire.endTransmission(); 
    return i;
}
void BH1750_Init(int address)
{
    Wire.beginTransmission(address);
    Wire.write(0x10);//1 lux resolution 120ms*
    Wire.endTransmission();
}
