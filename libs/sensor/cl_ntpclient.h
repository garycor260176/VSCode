#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

class cl_ntpclient {
    public:
        cl_ntpclient(const char* ntpServer, long  gmtOffset_sec = 10800, int   daylightOffset_sec = 0);
        void begin( );
        tm get_Time( );

    private:
        const char* _ntpServer;
        long _gmtOffset_sec;
        int _daylightOffset_sec;
};
 

cl_ntpclient::cl_ntpclient(const char* ntpServer, long  gmtOffset_sec, int daylightOffset_sec){
    _ntpServer = ntpServer;
    _gmtOffset_sec = gmtOffset_sec;
    _daylightOffset_sec = daylightOffset_sec;
}

void cl_ntpclient::begin( ){
  configTime(_gmtOffset_sec, _daylightOffset_sec, _ntpServer);
  get_Time( );
}

tm cl_ntpclient::get_Time( ){
    tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return timeinfo;
    }
    timeinfo.tm_year += 1900;
    return timeinfo;
}