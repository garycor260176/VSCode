#include <Arduino.h>

#include <Wire.h>
#include <DallasTemperature.h>

#define DS18B20PIN 15

boolean ds18b20_readed = false;

uint32_t DS18b20_Start = 0;
boolean DS18b20_Started = false;
uint32_t LastRead_DS18B20 = 0; //запоминаем время последней публикации
OneWire oneWire(DS18B20PIN);
DallasTemperature DS18B20(&oneWire);

boolean flag = false;

struct s_ds18b20{
  float f_t;
  String s_t;
};

struct s_state{
  uint32_t interval_DS18B20;
  s_ds18b20 ds18b20;
};

s_state cur_state;

void setup() {
  Serial.begin(115200);

  cur_state.interval_DS18B20 = 5000;

  DS18B20.begin();

  delay(200);
}

s_ds18b20 Read_ds18b20(){
  s_ds18b20 ret;

  DS18B20.requestTemperatures();
  ret.f_t = DS18B20.getTempCByIndex(0);

//в первй раз пропускаем. почему-то всегда выдает 25градусов
  if(DS18b20_Started){
    if (isnan(ret.f_t) || ret.f_t == -127 ) { //при сбое вернем предыдущее значение
      ret.f_t = cur_state.ds18b20.f_t;
      ret.s_t = cur_state.ds18b20.s_t;
    } else {
      ret.s_t = String(ret.f_t);
      ds18b20_readed = true;
    }
  } else {
    DS18b20_Started = true;
  }

  return ret;
}

s_state Read_state( ){
  s_state ret = cur_state;

  if((millis( ) - LastRead_DS18B20 > cur_state.interval_DS18B20 && cur_state.interval_DS18B20 >= 0 ) ) {
    ret.ds18b20 = Read_ds18b20( );
    LastRead_DS18B20 = millis();
  }
  return ret;
};

void report( s_state sState, int mode){
  if(ds18b20_readed && (mode == 0 || ( mode == 1 && sState.ds18b20.s_t != cur_state.ds18b20.s_t )) ){
    Serial.println("ds18b20/t = " +  sState.ds18b20.s_t);
  }
  cur_state = sState;
  flag = true;
}

void OnCheckState(){
  s_state state = Read_state( );
  
  if(!flag) {
    report(state, 0); //отправляем все
  } else {
    report(state, 1);
  }
}

void loop() {
  OnCheckState();
}