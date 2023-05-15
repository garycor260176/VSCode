#pragma once

#include <Arduino.h>

class button {
  public:
    button (byte pin) {
      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
    }
    int click() {
        int ret = 0;

        bool btnState = digitalRead(_pin);
        if(btnState == HIGH) {
          if(!_flag) _tmr = millis();
            _flag = true;            
        } else {
            if(_flag) {
              if(millis() - _tmr > 3000) {
                ret = 2;
              } else
                ret = 1;
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