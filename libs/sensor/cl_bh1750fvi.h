#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <BH1750FVI.h>
#include <mqtt_ini.h>

struct s_bh1750fvi_val{
  uint16_t lux;
};

class cl_bh1750fvi{
    public:
        cl_bh1750fvi(BH1750FVI* bh1750fvi, mqtt_ini* client, String mqtt_subpath, uint32_t interval = 30000, boolean debug = true);

        void begin( );
        void loop( );
        void set_interval(uint32_t interval);
        uint32_t get_interval();
        uint16_t get_last_value();
        void subscribe();

    private:
        BH1750FVI* _bh1750fvi;
        mqtt_ini* _client;
        String _mqtt_subpath;
        uint32_t _interval; //интервал чтения
        uint32_t _last_readed = 0; //время последнего чтения
        boolean _started;   //чтобы пропустить первое чтение. почему-то возвращает кривое значение в первый раз
        boolean _debug;

        s_bh1750fvi_val old_value; //предыдущее значение
        s_bh1750fvi_val last_value; //последнее прочтенное значение

        s_bh1750fvi_val read( ); //чтение

        void msg_interval( const String &message );
};

uint16_t cl_bh1750fvi::get_last_value(){
    return last_value.lux;
}

cl_bh1750fvi::cl_bh1750fvi(BH1750FVI* bh1750fvi, mqtt_ini* client, String mqtt_subpath, uint32_t interval, boolean debug){
    _bh1750fvi = bh1750fvi;
    _client = client;
    _mqtt_subpath = mqtt_subpath;
    _interval = interval;
    _debug = debug;
    boolean _started = false;
}

void cl_bh1750fvi::set_interval(uint32_t interval){
    _interval = interval;
    if(_interval <= 1000) {
        _interval = 1000;
    }
}

uint32_t cl_bh1750fvi::get_interval(){
    return _interval;
}

void cl_bh1750fvi::msg_interval(const String &message){
   if(_debug) Serial.println("bh1750fvi msg_interval: value = " + message);

  if(message.length() > 0){
    set_interval( message.toInt() * 1000 );
  }
}

void cl_bh1750fvi::subscribe(){
  _client->Subscribe(_mqtt_subpath + "/interval", [this](const String &message) { this->msg_interval(message); }); 
}

void cl_bh1750fvi::begin(){
    _bh1750fvi->begin( );
}

void cl_bh1750fvi::loop(){
    if(millis() - _last_readed > _interval) {
        s_bh1750fvi_val val = read( );
        if(!_started) {
            _started = true;
        } else {
            old_value = last_value;
            last_value = val;
            if(_debug){
                Serial.println("lux (bh1750fvi): " + String(last_value.lux));
            }

            if(old_value.lux != last_value.lux){
                _client->Publish(_mqtt_subpath + "/value", String(last_value.lux));
            }
        }
        _last_readed = millis( );
    }
}

s_bh1750fvi_val cl_bh1750fvi::read(){
    s_bh1750fvi_val ret;
	
    ret.lux = _bh1750fvi->GetLightIntensity();

    return ret;
}