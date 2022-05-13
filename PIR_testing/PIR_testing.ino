#include <FastLED.h>


#define NUM_LEDS 1
#define DATA_PIN 27
CRGB leds[NUM_LEDS];
/*
 * PIR sensor tester
 */

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause_counter = 5000;

boolean lockLow=true;
boolean takeLowTime;

int ledPin = 27;                // choose the pin for the LED
int inputPin = 19;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status
 
void setup() {
  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input
 
  Serial.begin(115200);
}
 
void loop(){
  val = digitalRead(inputPin);  // read input value

  if(digitalRead(inputPin)== HIGH){
    //Serial.println("input pin == HIGH");

    if(lockLow){
      lockLow=false;
      Serial.print("motion detected at ");
      Serial.print(millis()/1000);
      Serial.println(" sec"); 
      delay(50);
    }
    takeLowTime = true;
  }

  if(digitalRead(inputPin) == LOW){       
       //Serial.println("input pin == LOW"); //the led visualizes the sensors output pin state

       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       //if the sensor is low for more than the given pause, 
       //we assume that no more motion is going to happen
       if(!lockLow && millis() - lowIn > pause_counter){  
           //makes sure this block of code is only executed again after 
           //a new motion sequence has been detected
           lockLow = true;                        
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause_counter)/1000);
           Serial.println(" sec");
           delay(50);
           }
       }
  
  //delay(3000);
}
