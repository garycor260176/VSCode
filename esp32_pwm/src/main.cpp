#include <Arduino.h>

const int ledPin = 19;

const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

int v;
boolean dir;

void setup(){
  Serial.begin(115200);

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel); 
}

void loop(){
  if(!dir) { 
    v++;
    if(v >= 255) {
      dir  = true;
    }
  } else {
    v--;
    if(v <= 0) {
      dir  = false;
    }
  }

  ledcWrite(ledChannel, v);
  Serial.println(v);

  delay(15);
}