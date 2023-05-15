#pragma once

#include <Arduino.h>
#include <mqtt_ini.h>
#include <driver/adc.h>

struct s_wspeed_val{
  float f_value;
  float f_volt;
  float f_speed;
};

class cl_wspeed{
    public:
        cl_wspeed(adc1_channel_t channel, mqtt_ini* client, String mqtt_subpath, uint32_t interval = 100, boolean debug = true);

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

        s_wspeed_val old_value; //предыдущее значение
        s_wspeed_val last_value; //последнее прочтенное значение

        s_wspeed_val read( ); //чтение

        void msg_interval( const String &message );
        float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
};

cl_wspeed::cl_wspeed(adc1_channel_t channel, mqtt_ini* client, String mqtt_subpath, uint32_t interval, boolean debug){
    _channel = channel;
    _client = client;
    _mqtt_subpath = mqtt_subpath;
    _interval = interval;
    _debug = debug;
    _started = false;
    _initializated = false;
}

void cl_wspeed::set_interval(uint32_t interval){
    _interval = interval;
    if(_interval <= 100) {
        _interval = 100;
    }
}

uint32_t cl_wspeed::get_interval(){
    return _interval;
}

void cl_wspeed::msg_interval(const String &message){
   if(_debug) Serial.println("wdir msg_interval: value = " + message);

  if(message.length() > 0){
    set_interval( message.toInt() );
  }
}

void cl_wspeed::subscribe(){
  _client->Subscribe(_mqtt_subpath + "/interval", [this](const String &message) { this->msg_interval(message); }); 
}

void cl_wspeed::begin(){
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(_channel, ADC_ATTEN_DB_11); //GPIO32
}

void cl_wspeed::loop(){
    if(millis() - _last_readed > _interval) {
        s_wspeed_val val = read( );
        if(!_started) {
            _started = true;
        } else {
            old_value = last_value;
            last_value = val;
            if(_debug){
                Serial.println("speed (wspeed): " + String(last_value.f_speed));
            }

            if(old_value.f_speed != last_value.f_speed){
                _client->Publish(_mqtt_subpath + "/speed", String(last_value.f_speed));
            }
        }
        _last_readed = millis( );
    }
}

s_wspeed_val cl_wspeed::read(){
    s_wspeed_val ret = last_value;

    ret.f_value = adc1_get_raw(_channel);
    if(ret.f_value < 0) ret.f_value = 0;

//  напряжение
    ret.f_volt = mapFloat(ret.f_value,0,4095,0, 3.3);
    
//  скорость ветра
    float koef = 5 / 3.3; //т.к. датчик выдает 5В, а мы уменьшили до 3.3

    ret.f_speed = ret.f_volt * koef * 6; //скорость ветра

    if(ret.f_speed < 0.8){
        ret.f_speed = 0; //Минимальная детектируемая скорость ветра: 0,4–0,8 м/с
    }

    return ret;
}

float cl_wspeed::mapFloat(float x, float in_min, float in_max, float out_min, float out_max){
    return ( x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
