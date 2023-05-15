#pragma once

#include <Arduino.h>
#include <iarduino_I2C_IO.h> 
#include <mqtt_ini.h>

class pin_h{
    public:
        pin_h(uint8_t _pin, uint8_t _mode, mqtt_ini* _client, String _mqtt_subpath, iarduino_I2C_IO* _modul, int def_STATE = LOW){
            pin = _pin;
            modul = _modul;
            mqtt_subpath = _mqtt_subpath;
            client = _client;
            if(modul) {
                modul->pinMode(pin, _mode);  
                modul->digitalWrite(pin, def_STATE);  
            }else{
                pinMode(pin, _mode);  
                digitalWrite(pin, def_STATE);  
            }
        };

        int get(){
            if(modul) {
                state = modul->digitalRead(pin);
            } else {
                state = digitalRead(pin);
            }
            return state;
        }

        void set(uint8_t _state){
            if(modul) {
                if(modul->digitalRead(pin) != _state) {
                    modul->digitalWrite(pin, _state);
                }
            } else {
                if(digitalRead(pin) != _state) {
Serial.println("Set pin = " + String(pin) + ", value = " + String(_state));
                    digitalWrite(pin, _state);
                }
            }
        }

        void publish(){
            if(!client->client->isMqttConnected()) {
                return;
            }
            client->Publish(mqtt_subpath + "/state", String(state));
        }

        void loop(){
            get();
            if(client->flag_start || old_state != state){
                publish();
            }
            old_state = state;
        }


    private:
        iarduino_I2C_IO* modul;
        mqtt_ini* client;
        String mqtt_subpath;

        int pin;        
        int state;
        int old_state;
};