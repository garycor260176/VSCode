#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct s_ip{
  uint8_t ip0;
  uint8_t ip1;
  uint8_t ip2;
  uint8_t ip3;
};

class settings{
    public:
        int d1_temp; boolean f_d1_temp;
        int d2_temp; boolean f_d2_temp;
        int d3_temp; boolean f_d3_temp;
        int d4_temp; boolean f_d4_temp;

        int t_open;
        int t_close;

        settings(Preferences* _pref){
            pref = _pref;
        }

        void begin(const char* _name){
            pref->begin(_name, false);
        }

        int32_t read_attr(const char* key){
            return pref->getInt(key, 0);
        }
        
        void save_attr(const char* key, int32_t value){
            pref->putInt(key, value);
        }

        void read(){
            t_open = pref->getInt("t_open", 25);
            t_close = pref->getInt("t_close", 20);

            if(t_close > t_open) {
                int t = t_open;
                t_open = t_close;
                t_close = t;
            }

            old_t_open = t_open;
            old_t_close = t_close;
        }

        void write(){
            if(t_open != old_t_open)    {
                pref->putInt("t_open", t_open);
            }
            if(t_close != old_t_close)  {
                pref->putInt("t_close", t_close);
            }
            old_t_open = t_open;
            old_t_close = t_close;
        }

    private:
        Preferences* pref;

        int old_t_open;
        int old_t_close;
};