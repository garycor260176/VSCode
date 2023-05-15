#pragma once

#include <Arduino.h>

class button {
  public:
    button (byte pin) {
      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
    }
    bool click() {
        int ret = false;

        bool btnState = digitalRead(_pin);
        if(btnState == HIGH) {
            _flag = true;
            _tmr = millis();
        } else {
            if(_flag) {
                ret = true;
            }
            _flag = false;
            _tmr = 0;
        }

        return ret;
    }
  private:
    byte _pin;
    uint32_t _tmr;
    bool _flag;
};