#include <Arduino.h>

int old_v;

void setup() {
  Serial.begin(115200); // Start the serial port
  // put your setup code here, to run once:
  pinMode(18, INPUT);
  old_v = digitalRead(18);
    Serial.println(String(old_v));
}

void loop() {
  // put your main code here, to run repeatedly:
  int v = digitalRead(18);
  if(old_v != v) {
    Serial.println(String(v));
  }
  old_v = v;
}