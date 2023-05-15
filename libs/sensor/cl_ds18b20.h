#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <mqtt_ini.h>

struct s_ds18b20_val{
  float f_value;
  String s_value;

  boolean readed;
};

class cl_ds18b20{
    public:
        cl_ds18b20(DallasTemperature* ds18b20, mqtt_ini* client, String mqtt_subpath, uint32_t interval = 30000, boolean debug = true);

        void begin( );
        void loop( );
        void set_interval(uint32_t interval);
        uint32_t get_interval();
        void subscribe();
        s_ds18b20_val get_value( );
        s_ds18b20_val get_old_value( );

    private:
        DallasTemperature* _ds18b20;
        mqtt_ini* _client;
        String _mqtt_subpath;
        uint32_t _interval; //интервал чтения
        uint32_t _last_readed = 0; //время последнего чтения
        boolean _started;   //чтобы пропустить первое чтение. почему-то возвращает кривое значение в первый раз
        boolean _debug;

        s_ds18b20_val old_value; //предыдущее значение
        s_ds18b20_val last_value; //последнее прочтенное значение

        s_ds18b20_val read( ); //чтение

        void msg_interval( const String &message );
};

s_ds18b20_val cl_ds18b20::get_value( ){
    return last_value;
}

s_ds18b20_val cl_ds18b20::get_old_value( ){
    return old_value;
}

cl_ds18b20::cl_ds18b20(DallasTemperature* ds18b20, mqtt_ini* client, String mqtt_subpath, uint32_t interval, boolean debug){
    _ds18b20 = ds18b20;
    _client = client;
    _mqtt_subpath = mqtt_subpath;
    _interval = interval;
    _debug = debug;
    boolean _started = false;
}

void cl_ds18b20::set_interval(uint32_t interval){
    _interval = interval;
    if(_interval <= 1000) {
        _interval = 1000;
    }
}

uint32_t cl_ds18b20::get_interval(){
    return _interval;
}

void cl_ds18b20::msg_interval(const String &message){
   if(_debug) Serial.println("ds18b20 msg_interval: value = " + message);

  if(message.length() > 0){
    set_interval( message.toInt() * 1000 );
  }
}

void cl_ds18b20::subscribe(){
  _client->Subscribe(_mqtt_subpath + "/interval", [this](const String &message) { this->msg_interval(message); }); 
}

void cl_ds18b20::begin(){
    _ds18b20->begin( );
}

void cl_ds18b20::loop(){
    if(millis() - _last_readed > _interval) {
        s_ds18b20_val val = read( );
        if(!_started) {
            _started = true;
        } else {
            old_value = last_value;
            last_value = val;
            if(_debug){
                Serial.println("Temperature (ds18b20): " + last_value.s_value);
            }

            if(old_value.s_value != last_value.s_value){
                _client->Publish(_mqtt_subpath + "/value", last_value.s_value);
            }
        }
        _last_readed = millis( );
    }
}

s_ds18b20_val cl_ds18b20::read(){
    s_ds18b20_val ret;
    _ds18b20->requestTemperatures();
    ret.f_value = _ds18b20->getTempCByIndex(0);
    ret.readed = true;
    if (isnan(ret.f_value) || ret.f_value == -127 ) { //при сбое вернем предыдущее значение
        ret = last_value;
    } else {
        ret.s_value = String(ret.f_value);
    }
    return ret;
}