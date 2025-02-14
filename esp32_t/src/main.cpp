#include <arduino.h>

void setup() { 
  Serial.begin(115200);
  Serial.println("");  Serial.println("Start!");
}

void loop() { 
  Serial.println("Start!");
  delay(1000);
}
