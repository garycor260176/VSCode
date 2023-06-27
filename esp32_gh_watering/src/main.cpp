#include <mqtt_ini.h>

#define MQTT_PATH       "GH/CUBE"

#define PIN_PUMP    13
#define PIN_FLOAT_1 14
#define PIN_FLOAT_2 25
#define PIN_FLOAT_3 32
#define PIN_BED_1   18

struct s_pin{
  int pin;
  int state;
};

struct s_state{
  s_pin pump;
  s_pin float_1;
  s_pin float_2;
  s_pin float_3;
  s_pin bed_1;
};

s_state cur_state;

mqtt_ini client( 
  "GH_CUBE",     // Client name that uniquely identify your device
   MQTT_PATH);

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  cur_state.pump.pin = PIN_PUMP; pinMode(cur_state.pump.pin, OUTPUT); digitalWrite(cur_state.pump.pin, LOW);
  cur_state.bed_1.pin = PIN_BED_1; pinMode(cur_state.bed_1.pin, OUTPUT); digitalWrite(cur_state.bed_1.pin, LOW);
  cur_state.float_1.pin = PIN_FLOAT_1; pinMode(cur_state.float_1.pin, INPUT); //attachInterrupt(cur_state.float_1.pin, F1, CHANGE); 
  cur_state.float_2.pin = PIN_FLOAT_2; pinMode(cur_state.float_2.pin, INPUT); //attachInterrupt(cur_state.float_2.pin, F2, CHANGE); 
  cur_state.float_3.pin = PIN_FLOAT_3; pinMode(cur_state.float_3.pin, INPUT); //attachInterrupt(cur_state.float_3.pin, F3, CHANGE); 

  client.begin(true);
}

void onMsgCommand( const String &message ){ 
}

void report( s_state sState, int mode){
  if (mode == 0 || ( mode == 1 && sState.float_1.state != cur_state.float_1.state ) ) {
    client.Publish("float_1_state", String(sState.float_1.state));
  }
  if (mode == 0 || ( mode == 1 && sState.float_2.state != cur_state.float_2.state ) ) {
    client.Publish("float_2_state", String(sState.float_2.state));
  }
  if (mode == 0 || ( mode == 1 && sState.float_3.state != cur_state.float_3.state ) ) {
    client.Publish("float_3_state", String(sState.float_3.state));
  }

  if (mode == 0 || ( mode == 1 && sState.bed_1.state != cur_state.bed_1.state ) ) {
    client.Publish("bed_1_state", String(sState.bed_1.state));
  }

  if (mode == 0 || ( mode == 1 && sState.pump.state != cur_state.pump.state ) ) {
    client.Publish("pump_state", String(sState.pump.state));
  }
  
  cur_state = sState;
  client.flag_start = false;
}

void OnCheckState(){}

void pump_on() {
  int state_pump = digitalRead(cur_state.pump.pin);
  int f3 = digitalRead(cur_state.float_3.pin);
  if(f3 == HIGH) state_pump = LOW;
  if(state_pump == HIGH) return;
  digitalWrite(cur_state.pump.pin, HIGH);
}

void pump_off() {
  int state_pump = digitalRead(cur_state.pump.pin);
  if(state_pump == LOW) return;
  digitalWrite(cur_state.pump.pin, LOW);
}

void poliv_on() {
  int state_poliv = digitalRead(cur_state.bed_1.pin);
  int f1 = digitalRead(cur_state.float_1.pin);
  if(f1 == LOW) state_poliv = LOW;
  if(state_poliv == HIGH) return;
  digitalWrite(cur_state.bed_1.pin, HIGH);
}

void poliv_off() {
  int state_poliv = digitalRead(cur_state.bed_1.pin);
  if(state_poliv == LOW) return;
  digitalWrite(cur_state.bed_1.pin, LOW);
}

void msg_pump_state(const String message){
  cur_state.pump.state = (message.toInt() == 1 ? HIGH : LOW);
  switch(cur_state.pump.state){
    case HIGH: pump_on(); break;
    default: pump_off(); break;
  }  
}

void msg_bed_1_state(const String message){
  cur_state.bed_1.state = (message.toInt() == 1 ? HIGH : LOW);
  switch(cur_state.bed_1.state){
    case HIGH: poliv_on(); break;
    default: poliv_off(); break;
  }  
}

void onConnection(){
  client.Subscribe("pump_state",  msg_pump_state);
  client.Subscribe("bed_1_state",  msg_bed_1_state);
}

void OnLoad(){}

void check(s_state* state){
  state->float_1.state = digitalRead(state->float_1.pin);
  state->float_2.state = digitalRead(state->float_2.pin);
  state->float_3.state = digitalRead(state->float_3.pin);
  state->pump.state = digitalRead(state->pump.pin);
  state->bed_1.state = digitalRead(state->bed_1.pin);

  if(state->float_3.state == HIGH) {
    state->pump.state = LOW;
  }
  if(state->float_1.state == LOW) {
    state->bed_1.state = LOW;
  }

  digitalWrite(state->pump.pin, state->pump.state);
  digitalWrite(state->bed_1.pin, state->bed_1.state);
}

void loop() {
  client.loop();

  s_state sState = cur_state;
  check(&sState);

  if (client.flag_start) { //первый запуск
    report(sState, 0); //отправляем все
  } else {
    report(sState, 1); //отправляем все
  } 
}