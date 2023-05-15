#pragma once

#include <Arduino.h>
#include <mqtt_ini.h>
#include <Adafruit_Sensor.h>
#include <iarduino_Pressure_BMP.h>

struct s_bme280_val{
  float f_temp_value;
  String s_temp_value;

  float f_humidity_value;
  String s_humidity_value;

  float f_pressure_value;
  String s_pressure_value;

  float f_pressure_value_Pa;
  String s_pressure_value_Pa;

  float f_Altitude;
  String s_Altitude;
};

class cl_bme280{
    public:
        cl_bme280(iarduino_Pressure_BMP* bme280, mqtt_ini* client, String mqtt_subpath, uint32_t interval = 30000, boolean debug = true);

        void begin( );
        void loop( );
        void set_interval(uint32_t interval);
        uint32_t get_interval();
        void subscribe();

    private:
        iarduino_Pressure_BMP* _bme280;
        mqtt_ini* _client;
        String _mqtt_subpath;
        uint32_t _interval; //интервал чтения
        uint32_t _last_readed = 0; //время последнего чтения
        boolean _started;   //чтобы пропустить первое чтение. почему-то возвращает кривое значение в первый раз
        boolean _debug;
        boolean _initializated;

        s_bme280_val old_value; //предыдущее значение
        s_bme280_val last_value; //последнее прочтенное значение

        s_bme280_val read( ); //чтение

        void msg_interval( const String &message );
};

cl_bme280::cl_bme280(iarduino_Pressure_BMP* bme280, mqtt_ini* client, String mqtt_subpath, uint32_t interval, boolean debug){
    _bme280 = bme280;
    _client = client;
    _mqtt_subpath = mqtt_subpath;
    _interval = interval;
    _debug = debug;
    _started = false;
    _initializated = false;
}

void cl_bme280::set_interval(uint32_t interval){
    _interval = interval;
    if(_interval <= 1000) {
        _interval = 1000;
    }
}

uint32_t cl_bme280::get_interval(){
    return _interval;
}

void cl_bme280::msg_interval(const String &message){
   if(_debug) Serial.println("bme280 msg_interval: value = " + message);

  if(message.length() > 0){
    set_interval( message.toInt() * 1000 );
  }
}

void cl_bme280::subscribe(){
  _client->Subscribe(_mqtt_subpath + "/interval", [this](const String &message) { this->msg_interval(message); }); 
}

void cl_bme280::begin(){
  _bme280->begin(117);

/*  if (!_bme280->begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
*/
  _initializated = true;
}

void cl_bme280::loop(){
    if(millis() - _last_readed > _interval) {
        s_bme280_val val = read( );
        if(!_started) {
            _started = true;
        } else {
            old_value = last_value;
            last_value = val;
            if(_debug){
                Serial.println("Temperature (bme280): " + last_value.s_temp_value);
                Serial.println("Humidity (bme280): " + last_value.s_humidity_value);
                Serial.println("Pressure (bme280): " + last_value.s_pressure_value);
            }

            if(old_value.s_temp_value != last_value.s_temp_value){
                _client->Publish(_mqtt_subpath + "/temp_value", last_value.s_temp_value);
            }
            if(old_value.s_humidity_value != last_value.s_humidity_value){
                _client->Publish(_mqtt_subpath + "/humidity_value", last_value.s_humidity_value);
            }
            if(old_value.s_pressure_value != last_value.s_pressure_value){
                _client->Publish(_mqtt_subpath + "/pressure_value", last_value.s_pressure_value);
            }
            if(old_value.s_pressure_value_Pa != last_value.s_pressure_value_Pa){
                _client->Publish(_mqtt_subpath + "/pressure_value_Pa", last_value.s_pressure_value_Pa);
            }
            if(old_value.s_Altitude != last_value.s_Altitude){
                _client->Publish(_mqtt_subpath + "/Altitude", last_value.s_Altitude);
            }
        }
        _last_readed = millis( );
    }
}

s_bme280_val cl_bme280::read(){
    s_bme280_val ret = last_value;

    if(_bme280->read(1)) {
        ret.f_temp_value = _bme280->temperature;
        ret.f_pressure_value = _bme280->pressure;
        ret.f_Altitude = _bme280->altitude;
    }
    if(ret.f_pressure_value <= 0) ret.f_pressure_value = last_value.f_pressure_value;

//    ret.f_temp_value = _bme280->readTemperature( );
//    ret.f_humidity_value = _bme280->readHumidity( );
//    ret.f_pressure_value_Pa = _bme280->readPressure() / 100.0F;
//    ret.f_pressure_value = ret.f_pressure_value_Pa * 0.750063755419211;
//    ret.f_Altitude = _bme280->readAltitude(SEALEVELPRESSURE_HPA);

    ret.s_temp_value = String(ret.f_temp_value);
//    ret.s_pressure_value_Pa = String(ret.f_pressure_value_Pa);
    ret.s_pressure_value = String(ret.f_pressure_value);
    ret.s_Altitude = String( ret.f_Altitude );
//    ret.s_humidity_value = String(ret.f_humidity_value);

    return ret;
}