#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>


OneWire oneWire(18);
DallasTemperature DS18B20(&oneWire);

void setup() {
  Serial.begin(115200);

  DS18B20.begin();
}

void loop() {
  DS18B20.requestTemperatures();
  Serial.println("DS18B20 tempC=" + String(DS18B20.getTempCByIndex(0)));
  delay(10000);
}