#include <mqtt_ini.h>

#define def_path "outside/gh/cube"

#define PIN_RELAY_POLIV    27
#define PIN_BTN_POLIV      12
#define PIN_BTN_PUMP       14

#define PIN_FLOAT_MIN      18
#define PIN_FLOAT_MAX      19

struct s_relay{
  int pin;
  int state;
  boolean changed;

  String name;

  uint32_t TimeSinceLastBtn = 0;
  boolean flagSinceLastBtn = false;
};

struct s_state {
  s_relay pump;
  s_relay poliv;

  s_relay float_min;
  s_relay float_max;

  s_relay btn;
  s_relay btn_pump;
};

s_state cur_state;

mqtt_ini client( 
  "ESP8266_GH_CUBE",     // Client name that uniquely identify your device
   def_path);

void report(int);

void setup() { 
  Serial.begin(115200);

  cur_state.poliv.pin = PIN_RELAY_POLIV; cur_state.poliv.name = "poliv";  pinMode(cur_state.poliv.pin, OUTPUT); digitalWrite(cur_state.poliv.pin, LOW);
  cur_state.btn.pin = PIN_BTN_POLIV; cur_state.btn.name = "btn";  pinMode(cur_state.btn.pin, INPUT);
  cur_state.btn_pump.pin = PIN_BTN_PUMP; cur_state.btn_pump.name = "btn_pump";  pinMode(cur_state.btn_pump.pin, INPUT);

  cur_state.float_min.pin = PIN_FLOAT_MIN; cur_state.float_min.name = "float_min";  pinMode(cur_state.float_min.pin, INPUT);
  cur_state.float_max.pin = PIN_FLOAT_MAX; cur_state.float_max.name = "float_max";  pinMode(cur_state.float_max.pin, INPUT);

  client.begin();
}

void pump_on( ){
  int state = cur_state.pump.state;
  if(state != HIGH) state = HIGH;
  if(cur_state.float_max.state == HIGH) state = LOW;
  if(state != cur_state.pump.state) {
    if(state == HIGH)
      client.Publish("outside/pump_01/command", "valve_01-on", true);
    else
      client.Publish("outside/pump_01/command", "valve_01-off", true);
  } 
}

void pump_off(){
  int state = cur_state.pump.state;
  if(state != LOW) state = LOW;
  if(state != cur_state.pump.state) {
    cur_state.pump.changed = true;
    if(state == HIGH)
      client.Publish("outside/pump_01/command", "valve_01-on", true);
    else
      client.Publish("outside/pump_01/command", "valve_01-off", true);
  } 
}

void poliv_on( ){
  int state = digitalRead(cur_state.poliv.pin);
  if(state != HIGH) state = HIGH;
  if(cur_state.float_min.state == LOW) state = LOW;
  if(state != cur_state.poliv.state) {
    cur_state.poliv.changed = true;
    digitalWrite(cur_state.poliv.pin, state);
  }
}

void poliv_off(){
  int state = digitalRead(cur_state.poliv.pin);
  if(state != LOW) state = LOW;
  if(state != cur_state.poliv.state) {
    cur_state.poliv.changed = true;
    digitalWrite(cur_state.poliv.pin, state);
  }
}

void all_off(){
  pump_off( );
  poliv_off( );
}

void onMsgCommand( const String &message ){
  if(message == "pump-on"){
    pump_on( );
  }
  if(message == "pump-off"){
    pump_off( );
  }
  if(message == "poliv-on"){
    poliv_on( );
  }
  if(message == "poliv-off"){
    poliv_off( );
  }
}

void Msg_valve_01( const String &message ){
  if(!client.flag_Loaded) return;

  if(message.length() > 0){
    cur_state.pump.changed = true;
    cur_state.pump.state = message.toInt();
  }
}

void onConnection()
{
  client.Subscribe("outside/pump_01/01/state", Msg_valve_01, true); 
}

void read_state_rel(s_relay &ret){
  int val = digitalRead(ret.pin);

  if(val != ret.state){
    ret.state = val;
    ret.changed = true;
  }
}

void Read_state( ){
  cur_state.poliv.changed = false;
  cur_state.float_min.changed = false;
  cur_state.float_max.changed = false;

  read_state_rel(cur_state.float_min);
  read_state_rel(cur_state.float_max);

  if(cur_state.float_max.state == HIGH){
    pump_off();
  }

  if(cur_state.float_min.state == LOW){
    poliv_off();
  }

  if(!client.client->isConnected()){
    all_off();
    client.flag_start = false;
  }

  read_state_rel(cur_state.poliv);
}

void report_rel( s_relay &rel, int mode){
  if(mode == 0 || ( mode == 1 && rel.changed ) ){
    client.Publish(rel.name + "/state", String(rel.state));
  }  
}

void report( int mode ){
  report_rel(cur_state.poliv, mode);
  report_rel(cur_state.float_min, mode);
  report_rel(cur_state.float_max, mode);

  client.flag_start = false;
}

void check_btn(s_relay &btn){
  btn.state = digitalRead( btn.pin );
  if(btn.state == HIGH){
    if(!btn.flagSinceLastBtn){
      btn.flagSinceLastBtn = true;
      btn.TimeSinceLastBtn = millis( );
    }
  } else {
    if(btn.flagSinceLastBtn){
      if(millis( ) - btn.TimeSinceLastBtn >= 3000){
        client.Publish(btn.name + "/state", "2");
      } else {
        client.Publish(btn.name + "/state", "1");
      }
      btn.TimeSinceLastBtn = 0;
    }
    btn.flagSinceLastBtn = false;
  }
}

void OnCheckState(){
  Read_state( );

  //check_btn( cur_state.btn );
  //check_btn( cur_state.btn_pump );

  if(client.flag_start){ //первый запуск
    report(0); //отправляем все
  } else {
    report(1); //отправляем только то, что изменилось
  }
}

void OnLoad(){}

void loop() {
  client.loop();
}