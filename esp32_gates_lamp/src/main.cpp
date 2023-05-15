#include <mqtt_ini.h>
#include <BH1750FVI.h>

#define def_path "outside/gates/dev01"

#define PIN_LAMP01   14
#define PIN_MOVE     12  

mqtt_ini client( 
  "ESP32_GL01",     // Client name that uniquely identify your device
   def_path);

struct s_move {
  int pin;
  int state;
};

struct s_level {
  int pin;
  int state;
};

struct s_state{
  s_level lamp01;
  s_move move;
};

s_state cur_state; //предыдущее отправленное состояние 

void report( s_state, int);

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("Start!");

  cur_state.lamp01.pin = PIN_LAMP01; pinMode(cur_state.lamp01.pin, OUTPUT);
  cur_state.move.pin = PIN_MOVE; pinMode(cur_state.move.pin, INPUT); 

  client.begin();
}

void onMsgCommand( const String &message ){
  if(message == "lamp01-on"){
    digitalWrite(cur_state.lamp01.pin, HIGH);
  }
  if(message == "lamp01-off"){
    digitalWrite(cur_state.lamp01.pin, LOW);
  }
}

void report( s_state sState, int mode ){
  if (mode == 0 || ( mode == 1 && sState.lamp01.state != cur_state.lamp01.state ) ) {
    client.Publish("lamp01/state", String(sState.lamp01.state));
  }

  if (mode == 0 || ( mode == 1 && sState.move.state != cur_state.move.state ) ) {
    client.Publish("move/state", String(sState.move.state));
  }

  cur_state = sState;
  client.flag_start = false;
}

s_state Read_state( ){
  s_state ret = cur_state;

  ret.lamp01.state = digitalRead(ret.lamp01.pin);
  ret.move.state = digitalRead(ret.move.pin);

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
