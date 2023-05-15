#include "mqtt_ini.h"

#define def_path "eurobion"

#define FLAG_SEND_STATE     1

#define PIN_FLOAT           D7

mqtt_ini client( 
  "ESP8266_EUROBION",     // Client name that uniquely identify your device
   def_path);

struct stru_float{
  int pin;
  int state;
};

struct s_state{
  stru_float f;
};

s_state cur_state; //предыдущее отправленное состояние 

uint32_t ContactTime1;

void report( s_state, int, unsigned long);

void setup() {

  Serial.begin(115200);
  Serial.println("Start...");

  cur_state.f.pin = PIN_FLOAT;
  pinMode(cur_state.f.pin, INPUT);  

  client.begin();
}

void onMsgCommand( const String &message ){}

void report( s_state sState1, int mode, unsigned long flagSend ){
  s_state sState = sState1;
  
  unsigned long flagS = flagSend;
  if(flagS == 0) {
    flagS = 
            FLAG_SEND_STATE;
  }

  if( (flagS & FLAG_SEND_STATE) == FLAG_SEND_STATE){
    if(mode == 0 || ( mode == 1 && sState.f.state != cur_state.f.state) ){
      client.Publish("state", String(sState.f.state));
    }
  }

  cur_state = sState;

  client.flag_start = false;
}

s_state Read_state( ){
  s_state ret = cur_state;

  if(digitalRead(ret.f.pin) == HIGH){
    if(millis() - ContactTime1 > 1000) {
      ret.f.state = HIGH;
    }
  } else {
    ret.f.state = LOW;
    ContactTime1 = millis();
  }
  
  return ret;
};

void OnCheckState(){
  s_state state = Read_state( );

  if(client.flag_start){ //первый запуск
    report(state, 0, 0); //отправляем все
  } else {
    report(state, 1, 0); //отправляем только то, что изменилось
  }
}

void onConnection(){}

void OnLoad(){}

void loop() {
  client.loop();
}