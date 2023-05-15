#include <mqtt_ini.h>

#define def_path "gates01"

#define PIN_LAMP     14
#define PIN_MOVE     12  

mqtt_ini client( 
  "ESP32_GATES01",     // Client name that uniquely identify your device
   def_path);

struct s_pin_state {
  int pin;
  int state;
};

struct s_state{
  s_pin_state lamp;
};

s_state cur_state;

void report(struct s_state* state, int mode);

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("Start!");

  cur_state.lamp.pin = PIN_LAMP; pinMode(cur_state.lamp.pin, OUTPUT);

  client.begin();
}

void Msg_state(const String &message) {
  cur_state.lamp.state = ( message.toInt() == 1 ? 1 : 0 );
}

void onMsgCommand( const String &message ){}
void OnLoad(){}
void OnCheckState(){}
void onConnection(){
  client.Subscribe("lamp/state", Msg_state); 
}
void check_state();

void report(s_state* sState, int mode ){
  if (mode == 0 || ( mode == 1 && sState->lamp.state != digitalRead(cur_state.lamp.pin) ) ) {
    client.Publish("lamp/state", String(sState->lamp.state));
  }
  client.flag_start = false;
}

void check_state() {
  s_state state = cur_state;

  if (client.flag_start) { //первый запуск
    report(&state, 0); //отправляем все
  } else {
    report(&state, 1); //отправляем все
  }  

  cur_state = state;
}

void loop() {
  client.loop();
  check_state();

  if(cur_state.lamp.state != digitalRead(cur_state.lamp.pin)) {
    digitalWrite(cur_state.lamp.pin, cur_state.lamp.state);
  }
}