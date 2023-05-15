#pragma once

#include <Arduino.h>
#include <iarduino_I2C_IO.h> 
#include <pin.h>
#include <mqtt_ini.h>
#include <settings.h>

#define MODE_AUTO   0
#define MODE_OPEN   1
#define MODE_CLOSE  2

#define STATE_UNKNOWN 0 //состояние окна неизвестно
#define STATE_OPENING 1 //открывается
#define STATE_OPENED  2 //открыто
#define STATE_CLOSING 3 //закрывается
#define STATE_CLOSED  4 //закрыто

class wnd{
    public:
        int state = STATE_UNKNOWN;
        int old_state = STATE_UNKNOWN;

        int mode = MODE_AUTO;

        String ini_key;

        wnd(pin_h* _pwr24, settings* _ini, iarduino_I2C_IO* _modul, int _pin_open, int _pin_close, mqtt_ini* _client, String _mqtt_subpath, String _ini_key) {
            pwr24 = _pwr24;
            ini_key = _ini_key;
            ini = _ini;
            modul = _modul;
            client = _client;
            mqtt_subpath = _mqtt_subpath;
            pin_open = new pin_h(_pin_open, OUTPUT, _client, _mqtt_subpath + "/pin_open", _modul);
            pin_close = new pin_h(_pin_close, OUTPUT, _client, _mqtt_subpath + "/pin_close", _modul);
            state = old_state = STATE_UNKNOWN;
        }

        boolean new_state( ){
            if(state != need_state) return true;
            return false;
        }

        boolean start_process(int cur_temperature){
            set_need_state(cur_temperature);
            if(state == need_state) return false;            

            switch(need_state){
                case STATE_OPENED:
                    open();
                    return true;
                break;

                case STATE_CLOSED:
                    close();
                    return true;
                break;
            }

            return false;
        }

        void set_need_state(int cur_temperature){
            need_state = 0;

            if(mode == MODE_OPEN) {
                need_state = STATE_OPENED;
            } else if(mode == MODE_CLOSE) {
                need_state = STATE_CLOSED;
            } else {
                if(ini->t_open <= cur_temperature) {
                    need_state = STATE_OPENED;
                } else if(ini->t_close > cur_temperature) {
                    need_state = STATE_CLOSED;
                }
            }
        }

        void publish(){
            if(state != old_state) {
               client->Publish(mqtt_subpath + "/state", String(state));
            }
            pin_open->loop();
            pin_close->loop();
        }

        void stop(){
            boolean b_delay = false;
            if(pin_open->get() == HIGH) {
                state = STATE_OPENED;
                pin_open->set(LOW);
                b_delay = true;
            }
            if(pin_close->get() == HIGH) {
                state = STATE_CLOSED;
                pin_close->set(LOW);
                b_delay = true;
            }
            if(b_delay) delay(300);
        }

        void read_ini(){
            state = old_state = ini->read_attr(ini_key.c_str( ));
            Serial.println("eeprom readed: " + ini_key + "_state = " + String(state));
        }
        void save_ini(){
            if(state != old_state && ( state == STATE_OPENED || state == STATE_CLOSED)) {
                ini->save_attr(ini_key.c_str( ), state);
                Serial.println("eeprom writed: " + ini_key + "_state = " + String(state));
            }
            old_state = state;
        }

        boolean is_process( ){
            if(pin_open->get() == HIGH || pin_close->get() == HIGH) return true;
            return false;
        }

    private:
        int need_state = 0;

        pin_h* pwr24;
        settings* ini;
        iarduino_I2C_IO* modul;
        mqtt_ini* client;
        String mqtt_subpath;
        pin_h* pin_open;
        pin_h* pin_close;

        boolean open(){
            boolean ret = false;
            if(pin_open->get() != HIGH){
                stop();
                pwr24->set(HIGH); 
                pin_open->set(HIGH); 
                delay(300);
                state = STATE_OPENING;
                ret = true;
            }

            return ret;
        }

        boolean close(){
            boolean ret = false;
            if(pin_close->get() != HIGH){
                stop();
                pwr24->set(HIGH); 
                pin_close->set(HIGH);
                delay(300);
                state = STATE_CLOSING;
                ret = true;
            }
            return ret;
        }
};