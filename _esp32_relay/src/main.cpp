#include <mqtt_ini.h>

#define def_path "outside/rl01"

#define PIN_01   32
#define PIN_02   33  
#define PIN_03   25
#define PIN_04   26  

mqtt_ini client( 
  "ESP32_RL01",     // Client name that uniquely identify your device
   def_path);

struct s_level {
  int pin;
  int state;
};

struct s_state{
  s_level r01;
  s_level r02;
  s_level r03;
  s_level r04;
};

s_state cur_state; //предыдущее отправленное состояние 

void report( s_state, int);

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");
  Serial.println("Start!");

  cur_state.r01.pin = PIN_01; pinMode(cur_state.r01.pin, OUTPUT);
  cur_state.r02.pin = PIN_02; pinMode(cur_state.r02.pin, OUTPUT);
  cur_state.r03.pin = PIN_03; pinMode(cur_state.r03.pin, OUTPUT);
  cur_state.r04.pin = PIN_04; pinMode(cur_state.r04.pin, OUTPUT);

  client.begin();
}

void onMsgCommand( const String &message ){
  if(message == "01-on"){
    digitalWrite(cur_state.r01.pin, HIGH);
  } else
  if(message == "01-off"){
    digitalWrite(cur_state.r01.pin, LOW);
  } else

  if(message == "02-on"){
    digitalWrite(cur_state.r02.pin, HIGH);
  } else 
  if(message == "02-off"){
    digitalWrite(cur_state.r02.pin, LOW);
  } else

  if(message == "03-on"){
    digitalWrite(cur_state.r03.pin, HIGH);
  } else
  if(message == "03-off"){
    digitalWrite(cur_state.r03.pin, LOW);
  } else

  if(message == "04-on"){
    digitalWrite(cur_state.r04.pin, HIGH);
  } else
  if(message == "04-off"){
    digitalWrite(cur_state.r04.pin, LOW);
  }
}

void report( s_state sState, int mode ){
  if (mode == 0 || ( mode == 1 && sState.r01.state != cur_state.r01.state ) ) {
    client.Publish("r01/state", String(sState.r01.state));
  }
  if (mode == 0 || ( mode == 1 && sState.r02.state != cur_state.r02.state ) ) {
    client.Publish("r02/state", String(sState.r02.state));
  }
  if (mode == 0 || ( mode == 1 && sState.r03.state != cur_state.r03.state ) ) {
    client.Publish("r03/state", String(sState.r03.state));
  }
  if (mode == 0 || ( mode == 1 && sState.r04.state != cur_state.r04.state ) ) {
    client.Publish("r04/state", String(sState.r04.state));
  }

  cur_state = sState;
  client.flag_start = false;
}

s_state Read_state( ){
  s_state ret = cur_state;

  ret.r01.state = digitalRead(ret.r01.pin);
  ret.r02.state = digitalRead(ret.r02.pin);
  ret.r03.state = digitalRead(ret.r03.pin);
  ret.r04.state = digitalRead(ret.r04.pin);

  return ret;
};

void OnCheckState(){
  s_state state = Read_state( );

  if(client.flag_start){ //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void onConnection(){
}

void OnLoad(){
}

void loop() {
  client.loop();
}
