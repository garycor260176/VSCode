#pragma once

#include <Arduino.h>
#include <mqtt_ini.h>

void IRAM_ATTR rgisr();

class cl_rg11;

cl_rg11* inst;

class cl_rg11{
    public:
        cl_rg11(int pin, mqtt_ini* client, String mqtt_subpath, float Bucket_Size = 0.01, boolean debug = true);

        void begin( );

        void handler();
        void loop();
        void command(const String &message);

    private:
        int _pin;
        float _Bucket_Size;
        mqtt_ini* _client;
        String _mqtt_subpath;
        uint32_t _interval; //интервал чтения
        boolean _debug;

        volatile unsigned long tipCount;// bucket tip counter used in interrupt routine
        volatile unsigned long ContactTime;// Timer to manage any contact bounce in interrupt routine

        long lastcount;// Timer to manage any contact bounce in interrupt routine
        float totalRainfall;

        void print();
};

void cl_rg11::command(const String &message){
    if(message == "rg11-clear") {
        lastcount = 0;
        tipCount = 0;
        totalRainfall = 0;

        print();
    }
}

cl_rg11::cl_rg11(int pin, mqtt_ini* client, String mqtt_subpath, float Bucket_Size, boolean debug){
    _pin = pin;
    _Bucket_Size = Bucket_Size;
    _client = client;
    _mqtt_subpath = mqtt_subpath;
    _debug = debug;

    lastcount = 0;
    tipCount = 0;
    totalRainfall = 0;
}

void cl_rg11::begin(){
    pinMode(_pin, INPUT);

    inst = this;

    attachInterrupt(_pin, rgisr, FALLING);
}

void cl_rg11::print(){
    if(_debug) {
        Serial.println("--------------");
        Serial.print("Tip Count: "); Serial.print(tipCount);
        Serial.print("\tTotal Rainfall: "); Serial.print(totalRainfall);
        Serial.println("--------------");
    }

    _client->Publish(_mqtt_subpath + "/Total_Rainfall", String(totalRainfall));
}

void cl_rg11::loop(){
    cli();

    if(tipCount != lastcount) {
        lastcount = tipCount;
        totalRainfall = tipCount * _Bucket_Size;

        print( );
    }    

    sei();
}

void cl_rg11::handler(){
    if((millis() - ContactTime) > 70 ) { // debounce of sensor signal
        tipCount++;
        ContactTime = millis();
    }
}

void IRAM_ATTR rgisr() { 
    inst->handler( );
} 
