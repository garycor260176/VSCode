#pragma once
#include <Arduino.h>

void sparklesRoutine(s_mode mode) {
  for (byte i = 0; i < mode.scale; i++) {
    byte x = random(0, WIDTH);
    byte y = random(0, HEIGHT);
    if (getPixColorXY(x, y) == 0)
      leds[getPixelNumber(x, y)] = CHSV(random(0, 255), 255, 255);
  }
  fader(70);
}
