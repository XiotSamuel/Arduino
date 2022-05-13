//variable to store the brightness
int brightness=0;
int colour=0;
void setup()   
{  
    Serial.begin(115200);  
  //for user input the brightness
  Serial.println("Please enter the brightness (1 to 20):");
  while(Serial.available()==0){}
  brightness = Serial.read()-'0';
  Serial.println(brightness);
 
  //for user input the colour
  Serial.println("Please enter the colour (1 to 20):");
  while(Serial.available()==0){}
  colour = Serial.read()-'0';
  Serial.println(colour);
  while (Serial.available() == 0) {}  
}  
  
void loop()   
{  

} 
