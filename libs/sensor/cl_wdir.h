#pragma once

#include <Arduino.h>
#include <mqtt_ini.h>
#include <driver/adc.h>

struct s_wdir_val{
  float f_value;
  float f_volt;
  String dir;
};

s_wdir_val val2dir[8] = { 
    2.85, 3.3,   "N",
    0, 0.32,     "N/E",
    0.33, 0.76,  "E",
    0.77, 1.18,  "S/E",
    1.19, 1.56,  "S",
    1.57, 1.94,  "S/W",
    1.95, 2.37,  "W",
    2.38, 2.84,  "N/W"
};


class cl_wdir{
    public:
        cl_wdir(adc1_channel_t channel, mqtt_ini* client, String mqtt_subpath, uint32_t interval = 100, boolean debug = true);

        void begin( );
        void loop( );
        void set_interval(uint32_t interval);
        uint32_t get_interval();
        void subscribe();

    private:
        adc1_channel_t _channel;
        mqtt_ini* _client;
        String _mqtt_subpath;
        uint32_t _interval; //интервал чтения
        uint32_t _last_readed = 0; //время последнего чтения
        boolean _started;   //чтобы пропустить первое чтение. почему-то возвращает кривое значение в первый раз
        boolean _debug;
        boolean _initializated;

        s_wdir_val old_value; //предыдущее значение
        s_wdir_val last_value; //последнее прочтенное значение

        s_wdir_val read( ); //чтение

        void msg_interval( const String &message );
        float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
};

cl_wdir::cl_wdir(adc1_channel_t channel, mqtt_ini* client, String mqtt_subpath, uint32_t interval, boolean debug){
    _channel = channel;
    _client = client;
    _mqtt_subpath = mqtt_subpath;
    _interval = interval;
    _debug = debug;
    _started = false;
    _initializated = false;
}

void cl_wdir::set_interval(uint32_t interval){
    _interval = interval;
    if(_interval <= 100) {
        _interval = 100;
    }
}

uint32_t cl_wdir::get_interval(){
    return _interval;
}

void cl_wdir::msg_interval(const String &message){
   if(_debug) Serial.println("wdir msg_interval: value = " + message);

  if(message.length() > 0){
    set_interval( message.toInt() );
  }
}

void cl_wdir::subscribe(){
  _client->Subscribe(_mqtt_subpath + "/interval", [this](const String &message) { this->msg_interval(message); }); 
}

void cl_wdir::begin(){
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(_channel, ADC_ATTEN_DB_11); //GPIO32
}

void cl_wdir::loop(){
    if(millis() - _last_readed > _interval) {
        s_wdir_val val = read( );
        if(!_started) {
            _started = true;
        } else {
            old_value = last_value;
            last_value = val;
            if(_debug){
                Serial.println("dir (wdir): " + last_value.dir);
            }

            if(old_value.dir != last_value.dir){
                _client->Publish(_mqtt_subpath + "/dir", last_value.dir);
            }
        }
        _last_readed = millis( );
    }
}

s_wdir_val cl_wdir::read(){
    s_wdir_val ret = last_value;

    ret.f_value = adc1_get_raw(_channel);
    if(ret.f_value < 0) ret.f_value = 0;

    ret.f_volt = mapFloat(ret.f_value,0,4095,0, 3.3);

    int n = sizeof(val2dir)/sizeof(val2dir[0]);
    for(int i = 0; i < n; i++) {
        if(ret.f_volt >= val2dir[i].f_value && ret.f_volt <= val2dir[i].f_volt ) {
            ret.dir = val2dir[i].dir;
            break;
        }
    }

    return ret;
}

float cl_wdir::mapFloat(float x, float in_min, float in_max, float out_min, float out_max){
    return ( x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
