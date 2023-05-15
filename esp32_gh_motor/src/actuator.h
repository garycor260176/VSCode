#pragma once

#include <mqtt_ini.h>

enum STATES{
    STATE_UNKNOWN, //состояние окна неизвестно
    STATE_OPENING, //открывается
    STATE_OPENED , //открыто
    STATE_CLOSING, //закрывается
    STATE_CLOSED   //закрыто
};

enum NEW_STATES{
    STOP,
    OPEN,
    CLOSE
};

boolean canOpen(String name);
void pwrState(int state);

class actuator {
    public: 
        actuator(int pinOpen, int pinClose, mqtt_ini* client, String name){
            _pinOpen = pinOpen;
            _pinClose = pinClose;

            pinMode(_pinOpen, OUTPUT); digitalWrite(_pinOpen, LOW);
            pinMode(_pinClose, OUTPUT); digitalWrite(_pinClose, LOW);
            _pinOpenOldState = _pinCloseOldState = LOW;

            _state = STATE_UNKNOWN;
            _client = client;
            _name = name;
            _newState = STOP;
        };

        void onSubscribe(const String &topic, const String &message){
            String myTopic = "GH/MOTOR/" + _name + "/new_state";
            if(myTopic == topic) {
                int newState = message.toInt();
                switch(newState) {
                    case STOP:  _newState = STOP;  break;
                    case OPEN:  _newState = OPEN;  break;
                    case CLOSE: _newState = CLOSE; break;
                }
            }
        };

        void loop() {
            switch(_newState) {
                case STOP:  stop(); break;
                case OPEN:  open(); break;
                case CLOSE: close(); break;
            }

            if(_start_process) {
                if(millis() - _start_process_millis > TIME_CHANGE_STATE) {
                    _start_process = false;
                    stop();
                    switch(_state) {
                        case STATE_OPENING:
                            _state = STATE_OPENED;
                            break;
                        case STATE_CLOSING:
                            _state = STATE_CLOSED;
                            break;
                    }
                }
            }
        }

        void publish(int mode){
            if(mode == 0 || (mode == 1 && _old_state != _state)) {
              _client->Publish(_name + "/state", String(_state));
            }
            int value = digitalRead(_pinOpen);
            if(mode == 0 || (mode == 1 && _pinOpenOldState != value)) {
              _client->Publish(_name + "/pin_open/state", String(value));
            }
            value = digitalRead(_pinClose);
            if(mode == 0 || (mode == 1 && _pinCloseOldState != value)) {
              _client->Publish(_name + "/pin_close/state", String(value));
            }

            _old_state = _state;
            _pinOpenOldState = digitalRead(_pinOpen);
            _pinCloseOldState = digitalRead(_pinClose);
        }

        boolean stop(){
            if(!inProcess()) return false;
            _old_state = _state;
            _start_process = false;
            _pinOpenOldState  = digitalRead(_pinOpen);           digitalWrite(_pinOpen, LOW);
            _pinCloseOldState = digitalRead(_pinClose);          digitalWrite(_pinClose, LOW);
            delay(100);
            return true;
        }

        void open() {
            if(!canOpen(_name)) return;

            _old_state = _state;
            if(_state == STATE_OPENED) return;
            int val = digitalRead(_pinOpen);
            if(val != HIGH) {
                pwrState(HIGH);
                stop();
                _pinOpenOldState = val;       digitalWrite(_pinOpen, HIGH);
                _start_process = true;
                _start_process_millis = millis( );
            }
            _state = STATE_OPENING;
        }

        void close() {
            if(!canOpen(_name)) return;

            _old_state = _state;
            if(_state == STATE_CLOSED) return;
            int val = digitalRead(_pinClose);
            if(val != HIGH) {
                pwrState(HIGH);
                stop();
                _pinCloseOldState  = val;     digitalWrite(_pinClose, HIGH);
                _start_process = true;
                _start_process_millis = millis( );
            }
            _state = STATE_CLOSING;
        }

        String getName(){
            return _name;
        }

        boolean inProcess(){
            return (digitalRead(_pinOpen) == HIGH || digitalRead(_pinClose) == HIGH);
        }

    private:
        int _pinOpen;
        int _pinClose;
        STATES _state;
        NEW_STATES _newState;
        boolean _start_process;
        uint64_t _start_process_millis;

        int _pinOpenOldState;
        int _pinCloseOldState;
        STATES _old_state;

        mqtt_ini* _client;
        String _name;
};