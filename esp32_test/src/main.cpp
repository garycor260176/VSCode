#include <mqtt_ini.h>

#define PIN_R01   18

#define def_path "R01"

mqtt_ini client( 
  "ESP32_R01",     // Client name that uniquely identify your device
   def_path);

struct s_pin{
  int pin;
  int state;
};

struct s_state{
  s_pin r01;
};

s_state cur_state;

void report(s_state, int);

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  cur_state.r01.pin = PIN_R01; pinMode(cur_state.r01.pin, OUTPUT); digitalWrite(cur_state.r01.pin, LOW);

  client.begin(true);
}

void onMsgCommand( const String &message ){ 
  if(message == "r01-on") {
    digitalWrite(cur_state.r01.pin, HIGH);
  } else if(message == "r01-off") {
    digitalWrite(cur_state.r01.pin, LOW);
  }
}

s_state Read_state(s_state S){
  s_state ret = S;

  ret.r01.state = digitalRead(ret.r01.pin);

  return ret;
};

void report( s_state sState, int mode){
  if (mode == 0 || ( mode == 1 && sState.r01.state != cur_state.r01.state ) ) {
    client.Publish("r01_state", String(sState.r01.state));
  }

  cur_state = sState;
  client.flag_start = false;
}

void OnCheckState(){
  s_state state = Read_state( cur_state );

  if(client.flag_start){ //первый запуск    
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void onConnection(){
}

void OnLoad(){}

void loop() {
  client.loop();
}
