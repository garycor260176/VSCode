#include <Arduino.h>
#include <driver/adc.h>

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); //GPIO32 - brightness
}

int BRIGHTNESSMARK;
int BRIGHTNESSMARK_old;

#define BRIGHTNESSMIN       5
#define BRIGHTNESSMAX       255

void loop() {
  float raw = adc1_get_raw(ADC1_CHANNEL_4);
  raw = 4095 - raw;
  if(raw < 0) raw = 0;
  BRIGHTNESSMARK = map(raw,0,4095,BRIGHTNESSMIN, BRIGHTNESSMAX);
  if(BRIGHTNESSMARK != BRIGHTNESSMARK_old && abs(BRIGHTNESSMARK - BRIGHTNESSMARK_old) > 5) {
    Serial.println("BRIGHTNESSMARK: " + String(BRIGHTNESSMARK));
    BRIGHTNESSMARK_old = BRIGHTNESSMARK;
  }
}
