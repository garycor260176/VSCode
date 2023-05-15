#include <mqtt_ini.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "time.h"

#define PIN_PUMP    13
#define PIN_FLOAT_1 14
#define PIN_FLOAT_2 25
#define PIN_FLOAT_3 32
#define PIN_BED_1   18

#define def_path "GH/CUBE"

boolean local_mode = false;
boolean f_ntp = false;

Preferences preferences; //хранение текущего состояния

mqtt_ini client( 
  "ESP32_GH_CUBE",     // Client name that uniquely identify your device
   def_path);

struct s_pin{
  int pin;
  int state;
};

struct s_d{
  int on = 0;
  int s_hour = 20;
  int e_hour = 21;
};

struct s_state{
  s_pin pump;
  s_pin float_1;
  s_pin float_2;
  s_pin float_3;
  s_pin bed_1;

  s_d d1;
  s_d d2;
  s_d d3;
  s_d d4;
  s_d d5;
  s_d d6;
  s_d d7;

  int poliv_mode = 0;
  int cube_mode = 0;
};

s_state cur_state;
s_state eeprom;

void report(s_state, int);
bool get_LocalTime(struct tm * info);

boolean Flag_F1 = false;
boolean Flag_F2 = false;
boolean Flag_F3 = false;

const char* ntpServer = "time.google.com";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 10800;
uint64_t last_t;

s_d read_eeprom_d(String d_name){
  s_d ret;

  String dn = d_name + "_on";
  ret.on = preferences.getInt(dn.c_str( ), 0);
  Serial.println("eeprom readed: " + dn + " = " + String(ret.on));
   if(ret.on < 0 || ret.on > 1) ret.on = 0;

  dn = d_name + "_sh";
  ret.s_hour = preferences.getInt(dn.c_str( ), 0); 
  Serial.println("eeprom readed: " + dn + " = " + String(ret.s_hour));
  if(ret.s_hour < 0 || ret.s_hour > 23) ret.s_hour = 0;

  dn = d_name + "_eh";
  ret.e_hour = preferences.getInt(dn.c_str( ), 0); 
  Serial.println("eeprom readed: " + dn + " = " + String(ret.e_hour));
  if(ret.e_hour < 0 || ret.e_hour > 23) ret.e_hour = 0;

  if(ret.s_hour > ret.e_hour) ret.e_hour = ret.s_hour;

  return ret;
}

void read_eeprom(){
  cur_state.d1 = read_eeprom_d("d1");
  cur_state.d2 = read_eeprom_d("d2");
  cur_state.d3 = read_eeprom_d("d3");
  cur_state.d4 = read_eeprom_d("d4");
  cur_state.d5 = read_eeprom_d("d5");
  cur_state.d6 = read_eeprom_d("d6");
  cur_state.d7 = read_eeprom_d("d7");

  eeprom = cur_state;
}

void write_eeprom_d(String d_name, s_d c_d, s_d e_d){
  String dn = d_name + "_on";
  if(c_d.on != e_d.on) {
    preferences.putInt(dn.c_str( ), c_d.on);
    Serial.println("eeprom writed: " + dn + " = " + String(c_d.on));
  }

  dn = d_name + "_sh";
  if(c_d.s_hour != e_d.s_hour) {
    preferences.putInt(dn.c_str( ), c_d.s_hour);
    Serial.println("eeprom writed: " + dn + " = " + String(c_d.s_hour));
  }

  dn = d_name + "_eh";
  if(c_d.e_hour != e_d.e_hour) {
    preferences.putInt(dn.c_str( ), c_d.e_hour);
    Serial.println("eeprom writed: " + dn + " = " + String(c_d.e_hour));
  }
}

void write_eeprom(){
  write_eeprom_d("d1", cur_state.d1, eeprom.d1);
  write_eeprom_d("d2", cur_state.d2, eeprom.d2);
  write_eeprom_d("d3", cur_state.d3, eeprom.d3);
  write_eeprom_d("d4", cur_state.d4, eeprom.d4);
  write_eeprom_d("d5", cur_state.d5, eeprom.d5);
  write_eeprom_d("d6", cur_state.d6, eeprom.d6);
  write_eeprom_d("d7", cur_state.d7, eeprom.d7);

  eeprom = cur_state;  
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  preferences.begin("settings", false);

  cur_state.pump.pin = PIN_PUMP; pinMode(cur_state.pump.pin, OUTPUT); digitalWrite(cur_state.pump.pin, LOW);

  cur_state.float_1.pin = PIN_FLOAT_1; pinMode(cur_state.float_1.pin, INPUT); //attachInterrupt(cur_state.float_1.pin, F1, CHANGE); 
  cur_state.float_2.pin = PIN_FLOAT_2; pinMode(cur_state.float_2.pin, INPUT); //attachInterrupt(cur_state.float_2.pin, F2, CHANGE); 
  cur_state.float_3.pin = PIN_FLOAT_3; pinMode(cur_state.float_3.pin, INPUT); //attachInterrupt(cur_state.float_3.pin, F3, CHANGE); 

  cur_state.bed_1.pin = PIN_BED_1; pinMode(cur_state.bed_1.pin, OUTPUT); digitalWrite(cur_state.bed_1.pin, LOW);

  read_eeprom();

  client.begin(true);
}

void onMsgCommand( const String &message ){ 
  if(message == "pump-on") {
    if(cur_state.float_3.state != HIGH) {
      digitalWrite(cur_state.pump.pin, HIGH);
    }
  } else if(message == "pump-off") {
    digitalWrite(cur_state.pump.pin, LOW);
  } else if(message == "bed_1-on") {
    if(cur_state.float_1.state == HIGH) {
      digitalWrite(cur_state.bed_1.pin, HIGH);
    }
  } else if(message == "bed_1-off") {
    digitalWrite(cur_state.bed_1.pin, LOW);
  }
}

s_state Read_state(s_state S){
  s_state ret = S;

  Flag_F1 = Flag_F2 = Flag_F3 = true;
  if(Flag_F1) { Flag_F1 = false; ret.float_1.state = digitalRead(ret.float_1.pin); }
  if(Flag_F2) { Flag_F2 = false; ret.float_2.state = digitalRead(ret.float_2.pin); }
  if(Flag_F3) { Flag_F3 = false; ret.float_3.state = digitalRead(ret.float_3.pin); }
  Flag_F1 = Flag_F2 = Flag_F3 = true;

// если взвелся верхний поплавок, то вырубить насос в любом случае
  if(ret.float_3.state == HIGH) {
    digitalWrite(ret.pump.pin, LOW);
  }
// если сбросился нижний поплавок, то вырубить полив в любом случае
  if(ret.float_1.state == LOW) {
    digitalWrite(ret.bed_1.pin, LOW);
  }

  ret.bed_1.state = digitalRead(ret.bed_1.pin);

  ret.pump.state = digitalRead(ret.pump.pin);

  return ret;
};

String get_d( s_d d ){
  String ret = "";
  DynamicJsonDocument doc(1024);
  doc["on"] = d.on;
  doc["s_h"] = d.s_hour;
  doc["e_h"] = d.e_hour;
  serializeJson(doc, ret);
  return ret;
}

void rep_d(int mode, String topic, s_d c_d, s_d s_d){
  String s_c_d = get_d(c_d);
  String s_s_d = get_d(s_d);
  if (mode == 0 || ( mode == 1 && s_c_d != s_s_d ) ) {
    client.Publish("settings/" + topic, s_s_d);
  }
}

void report( s_state sState, int mode){
  if(!local_mode) {
    if (mode == 0 || ( mode == 1 && sState.pump.state != cur_state.pump.state ) ) {
      client.Publish("pump_state", String(sState.pump.state));
    }
    if (mode == 0 || ( mode == 1 && sState.cube_mode != cur_state.cube_mode ) ) {
      client.Publish("cube_mode", String(sState.cube_mode));
    }

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
    if (mode == 0 || ( mode == 1 && sState.poliv_mode != cur_state.poliv_mode ) ) {
      client.Publish("poliv_mode", String(sState.poliv_mode));
    }

    rep_d(mode, "d1", cur_state.d1, sState.d1);
    rep_d(mode, "d2", cur_state.d2, sState.d2);
    rep_d(mode, "d3", cur_state.d3, sState.d3);
    rep_d(mode, "d4", cur_state.d4, sState.d4);
    rep_d(mode, "d5", cur_state.d5, sState.d5);
    rep_d(mode, "d6", cur_state.d6, sState.d6);
    rep_d(mode, "d7", cur_state.d7, sState.d7);
  }

  cur_state = sState;
  client.flag_start = false;

  write_eeprom( );
}

void check_cube(s_state state, boolean fon = false){
  switch(state.cube_mode) {
    case 1:
      if(state.float_3.state != HIGH) {
        digitalWrite(state.pump.pin, HIGH);
      }
    break;

    case 2:
      digitalWrite(state.pump.pin, LOW);
    break;

    default:
      if(state.float_1.state == LOW || ( fon && state.float_3.state != HIGH )) {
        digitalWrite(state.pump.pin, HIGH);
      }
      if(state.float_3.state == HIGH) {
        digitalWrite(state.pump.pin, LOW);
      }
      
    break;
  }
}

void check_d(struct tm info, s_state state, s_d d){
  if(d.on != 1) {
    if(state.float_2.state == LOW) { //включить набор бочки если воды меньше половины
      check_cube(state, true);
    }
    return;
  }

  if(info.tm_hour >= d.s_hour && info.tm_hour <= d.e_hour) {
    if(state.float_1.state == HIGH) {
      digitalWrite(state.bed_1.pin, HIGH);
    }
  } else {
    digitalWrite(state.bed_1.pin, LOW);
    if(info.tm_hour > d.e_hour && state.float_2.state == LOW) { //включить набор бочки после полива если воды меньше половины
      check_cube(state, true);
    }
  }
}

void check_poliv(s_state state){
  struct tm info;
  if(!get_LocalTime(&info)){
    return;
  } 

  switch(state.poliv_mode){
    case 1:
      if(state.float_1.state == HIGH) {
        digitalWrite(state.bed_1.pin, HIGH);
      }
    break;

    case 2:
      digitalWrite(state.bed_1.pin, LOW);
    break;
    
    default:
      switch(info.tm_wday){
        case 1: check_d(info, state, state.d1);
        case 2: check_d(info, state, state.d2);
        case 3: check_d(info, state, state.d3);
        case 4: check_d(info, state, state.d4);
        case 5: check_d(info, state, state.d5);
        case 6: check_d(info, state, state.d6);
        case 7: check_d(info, state, state.d7);
      }

    break;
  }
}

void OnCheckState(){
  s_state state = Read_state( cur_state );

  check_cube(state);
  check_poliv(state);

  state = Read_state(state);

  if(client.flag_start){ //первый запуск    
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

s_d read_d(String msg){
  s_d ret;

  if(msg.length( ) == 0) return ret;

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg);  

  ret.on = doc["on"];
  ret.s_hour = doc["s_h"];
  ret.e_hour = doc["e_h"];

  return ret;
}
void msg_settings_d1( const String &message ){
  cur_state.d1 = read_d(message);
}
void msg_settings_d2( const String &message ){
  cur_state.d2 = read_d(message);
}
void msg_settings_d3( const String &message ){
  cur_state.d3 = read_d(message);
}
void msg_settings_d4( const String &message ){
  cur_state.d4 = read_d(message);
}
void msg_settings_d5( const String &message ){
  cur_state.d5 = read_d(message);
}
void msg_settings_d6( const String &message ){
  cur_state.d6 = read_d(message);
}
void msg_settings_d7( const String &message ){
  cur_state.d7 = read_d(message);
}
void msg_settings_cube_mode( const String &message ){
  cur_state.cube_mode = message.toInt( );
  if(cur_state.cube_mode < 0 || cur_state.cube_mode > 2) cur_state.cube_mode = 0;
}
void msg_settings_poliv_mode( const String &message ){
  cur_state.poliv_mode = message.toInt( );
  if(cur_state.poliv_mode < 0 || cur_state.poliv_mode > 2) cur_state.poliv_mode = 0;
}

void onConnection(){
  local_mode = false;
  client.Subscribe("settings/d1",  msg_settings_d1);
  client.Subscribe("settings/d2",  msg_settings_d2);
  client.Subscribe("settings/d3",  msg_settings_d3);
  client.Subscribe("settings/d4",  msg_settings_d4);
  client.Subscribe("settings/d5",  msg_settings_d5);
  client.Subscribe("settings/d6",  msg_settings_d6);
  client.Subscribe("settings/d7",  msg_settings_d7);
  client.Subscribe("settings/d7",  msg_settings_d7);

  client.Subscribe("cube_mode",  msg_settings_cube_mode);
  client.Subscribe("poliv_mode",  msg_settings_poliv_mode);
}

void OnLoad(){}

void loop() {
  if(local_mode) {
    client.client->reset_count_failed_connect_attempt( );
  }

  client.loop();

  //если соединение с MQTT есть
  if(client.client->isMqttConnected()) {
    local_mode = false;
  } else {
    if(client.client->get_count_failed_connect_attempt( ) >= 7 && !local_mode) {
      local_mode = true;
      Serial.println("Set local mode");
      cur_state.poliv_mode = 0;
      cur_state.cube_mode = 0;
    }
  }

  if(local_mode) { //переходим в авторежим
    OnCheckState( );
  }
}

bool get_LocalTime(struct tm * info){
  bool ret = false;

  if(!f_ntp) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    f_ntp = true;
    delay(200);
  }

  ret  = getLocalTime(info);
  if(!ret){
    Serial.println("Failed to obtain time");
  }

  return ret;
}