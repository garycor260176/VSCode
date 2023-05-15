#pragma once

#include <Arduino.h>
#include <mqtt_ini.h>
#include "DHT.h"

struct s_dht22_val{
  float t_value;
  String ts_value;

  float h_value;
  String hs_value;
};

class cl_dht22{
    public:
        cl_dht22(DHT* dht22, mqtt_ini* client, String mqtt_subpath, uint32_t interval = 30000, boolean debug = true);

        void begin( );
        void loop( );
        void set_interval(uint32_t interval);
        uint32_t get_interval();
        void subscribe();

    private:
        DHT* _dht22;
        mqtt_ini* _client;
        String _mqtt_subpath;
        uint32_t _interval; //интервал чтения
        uint32_t _last_readed = 0; //время последнего чтения
        boolean _started;   //чтобы пропустить первое чтение. почему-то возвращает кривое значение в первый раз
        boolean _debug;

        s_dht22_val old_value; //предыдущее значение
        s_dht22_val last_value; //последнее прочтенное значение

        s_dht22_val read( ); //чтение

        void msg_interval( const String &message );
};

cl_dht22::cl_dht22(DHT* dht22, mqtt_ini* client, String mqtt_subpath, uint32_t interval, boolean debug){
    _dht22 = dht22;
    _client = client;
    _mqtt_subpath = mqtt_subpath;
    _interval = interval;
    _debug = debug;
    boolean _started = false;
}

void cl_dht22::set_interval(uint32_t interval){
    _interval = interval;
    if(_interval <= 1000) {
        _interval = 1000;
    }
}

uint32_t cl_dht22::get_interval(){
    return _interval;
}

void cl_dht22::msg_interval(const String &message){
   if(_debug) Serial.println("ds18b20 msg_interval: value = " + message);

  if(message.length() > 0){
    set_interval( message.toInt() * 1000 );
  }
}

void cl_dht22::subscribe(){
  _client->Subscribe(_mqtt_subpath + "/interval", [this](const String &message) { this->msg_interval(message); }); 
}

void cl_dht22::begin(){
    _dht22->begin( );
}

void cl_dht22::loop(){
    if(millis() - _last_readed > _interval) {
        s_dht22_val val = read( );
        old_value = last_value;
        last_value = val;
        if(_debug){
            Serial.println("Temperature (dht22): " + last_value.ts_value);
            Serial.println("Humidity (dht22): " + last_value.hs_value);
        }

        if(old_value.ts_value != last_value.ts_value){
            _client->Publish(_mqtt_subpath + "/t_value", last_value.ts_value);
        }

        if(old_value.hs_value != last_value.hs_value){
            _client->Publish(_mqtt_subpath + "/h_value", last_value.hs_value);
        }

        _last_readed = millis( );
    }
}

s_dht22_val cl_dht22::read(){
    s_dht22_val ret;

    ret.t_value = _dht22->readTemperature();
    ret.h_value = _dht22->readHumidity();
    if (isnan(ret.t_value) || isnan(ret.h_value)) { //при сбое вернем предыдущее значение
        ret = last_value;
    } else {
        ret.ts_value = String(ret.t_value);
        ret.hs_value = String(ret.h_value);
    }
        return ret;
}