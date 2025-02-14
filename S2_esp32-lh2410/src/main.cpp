#include <arduino.h>

void setup(void)
{
 Serial.begin(115200); //Feedback over Serial Monitor
 Serial.println("\nStart...");
 pinMode(23, INPUT);
}

void loop()
{
 static int old_value;
 int new_value = digitalRead(23);
 if(new_value != old_value){
  if(new_value == HIGH) {
    Serial.println(String(millis()) + ": 23 - " + String(new_value));
  } else {
    Serial.println(String(millis()) + ": 23 - LOW");
  }
  old_value = new_value;
 }
}