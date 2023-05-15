#include <mqtt_ini.h>
#include <WiFi.h>

#define def_path "rb02"

#define PIN_01   13
#define PIN_02   2
#define PIN_03   4
#define PIN_04   16
#define PIN_05   17
#define PIN_06   5
#define PIN_07   18
#define PIN_08   19
#define PIN_09   23
#define PIN_10   32
#define PIN_11   33
#define PIN_12   25
#define PIN_13   26
#define PIN_14   27
#define PIN_15   14
#define PIN_16   12
#define PIN_17   15

mqtt_ini client( 
  "ESP32_RB02",     // Client name that uniquely identify your device
   def_path);

WebServer* web;


struct s_level {
  int pin;
  int state;
};

struct s_relay{
  String rel_p;
  s_level rel;
};

struct s_state{
  s_relay r01;
  s_relay r02; 
  s_relay r03; 
  s_relay r04; 
  s_relay r05; 
  s_relay r06; 
  s_relay r07; 
  s_relay r08; 
  s_relay r09; 
  s_relay r10; 
  s_relay r11; 
  s_relay r12; 
  s_relay r13; 
  s_relay r14; 
  s_relay r15; 
  s_relay r16;
  s_relay r17;
};

s_state cur_state; //предыдущее отправленное состояние 

void report( s_state, int);

s_relay ini_relay(int pin_relay, String rel_p){
  s_relay ret;
  ret.rel.pin = pin_relay; pinMode(ret.rel.pin, OUTPUT);
  ret.rel_p = rel_p;
  return ret;
}

void handle_command() {
  Serial.println("handle_command");

  String command;
  String rXX;
  String msg;

  for (int i = 0; i < web->args(); i++){
    if(web->argName(i) == "r") {
      rXX = web->arg(i);
    } else if(web->argName(i) == "c") {
      command = web->arg(i);
    }
  }  

  s_relay rel;  rel.rel.pin = 0;

  if(rXX == "r01")      rel = cur_state.r01;
  else if(rXX == "r02") rel = cur_state.r02;
  else if(rXX == "r03") rel = cur_state.r03;
  else if(rXX == "r04") rel = cur_state.r04;
  else if(rXX == "r05") rel = cur_state.r05;
  else if(rXX == "r06") rel = cur_state.r06;
  else if(rXX == "r07") rel = cur_state.r07;
  else if(rXX == "r08") rel = cur_state.r08;
  else if(rXX == "r09") rel = cur_state.r09;
  else if(rXX == "r10") rel = cur_state.r10;
  else if(rXX == "r11") rel = cur_state.r11;
  else if(rXX == "r12") rel = cur_state.r12;
  else if(rXX == "r13") rel = cur_state.r13;
  else if(rXX == "r14") rel = cur_state.r14;
  else if(rXX == "r15") rel = cur_state.r15;
  else if(rXX == "r16") rel = cur_state.r16;
  else if(rXX == "r17") rel = cur_state.r17;

  if(rel.rel.pin == 0){
    msg = "Relay not found!";
  } else {
    if(command == "on") {
      if( digitalRead(rel.rel.pin) != HIGH ) digitalWrite(rel.rel.pin, HIGH);
      msg = rXX + " = on";
    } else if(command == "off") {
      if( digitalRead(rel.rel.pin) != LOW ) digitalWrite(rel.rel.pin, LOW);
      msg = rXX + " = off";
    }
  }
  web->send(200, "text/plain", msg);

  OnCheckState();
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));  Serial.println(F("Start!"));

  cur_state.r01 = ini_relay(PIN_01, "r01");
  cur_state.r02 = ini_relay(PIN_02, "r02");
  cur_state.r03 = ini_relay(PIN_03, "r03");
  cur_state.r04 = ini_relay(PIN_04, "r04");
  cur_state.r05 = ini_relay(PIN_05, "r05");
  cur_state.r06 = ini_relay(PIN_06, "r06");
  cur_state.r07 = ini_relay(PIN_07, "r07");
  cur_state.r08 = ini_relay(PIN_08, "r08");
  cur_state.r09 = ini_relay(PIN_09, "r09");
  cur_state.r10 = ini_relay(PIN_10, "r10");
  cur_state.r11 = ini_relay(PIN_11, "r11");
  cur_state.r12 = ini_relay(PIN_12, "r12");
  cur_state.r13 = ini_relay(PIN_13, "r13");
  cur_state.r14 = ini_relay(PIN_14, "r14");
  cur_state.r15 = ini_relay(PIN_15, "r15");
  cur_state.r16 = ini_relay(PIN_16, "r16");
  cur_state.r17 = ini_relay(PIN_17, "r17");

  client.begin(true);
  web = client.get_webserver();

  if(web == NULL) {
    Serial.println("err");
  } else {
    web->on("/command", handle_command);
  }
}

boolean MsgRel(const String &message, s_relay r){
  boolean ret = false;
  String s_on = r.rel_p.substring(1)+ "-on";
  String s_off = r.rel_p.substring(1) + "-off";

  if(message == s_on){
    if(digitalRead(r.rel.pin)!=HIGH) digitalWrite(r.rel.pin, HIGH);
    ret = true;
  } else if(message == s_off){
    if(digitalRead(r.rel.pin)!=LOW) digitalWrite(r.rel.pin, LOW);
    ret = true;
  } 

  return ret;
}

void onMsgCommand( const String &message ){
       if( MsgRel(message, cur_state.r01) ) {  } 
  else if( MsgRel(message, cur_state.r02) ) {  }
  else if( MsgRel(message, cur_state.r03) ) {  }
  else if( MsgRel(message, cur_state.r04) ) {  }
  else if( MsgRel(message, cur_state.r05) ) {  }
  else if( MsgRel(message, cur_state.r06) ) {  }
  else if( MsgRel(message, cur_state.r07) ) {  }
  else if( MsgRel(message, cur_state.r08) ) {  }
  else if( MsgRel(message, cur_state.r09) ) {  }
  else if( MsgRel(message, cur_state.r10) ) {  }
  else if( MsgRel(message, cur_state.r11) ) {  }
  else if( MsgRel(message, cur_state.r12) ) {  }
  else if( MsgRel(message, cur_state.r13) ) {  }
  else if( MsgRel(message, cur_state.r14) ) {  }
  else if( MsgRel(message, cur_state.r15) ) {  }
  else if( MsgRel(message, cur_state.r16) ) {  }
  else if( MsgRel(message, cur_state.r17) ) {  }
}

void report_rel(s_relay rel, s_relay rel_old, int mode){
  if (mode == 0 || ( mode == 1 && rel.rel.state != rel_old.rel.state ) ) {
    client.Publish(rel.rel_p + "/relay_state", String(rel.rel.state));
  }
}

void report( s_state sState, int mode ){
  report_rel(sState.r01, cur_state.r01, mode);
  report_rel(sState.r02, cur_state.r02, mode);
  report_rel(sState.r03, cur_state.r03, mode);
  report_rel(sState.r04, cur_state.r04, mode);
  report_rel(sState.r05, cur_state.r05, mode);
  report_rel(sState.r06, cur_state.r06, mode);
  report_rel(sState.r07, cur_state.r07, mode);
  report_rel(sState.r08, cur_state.r08, mode);
  report_rel(sState.r09, cur_state.r09, mode);
  report_rel(sState.r10, cur_state.r10, mode);
  report_rel(sState.r11, cur_state.r11, mode);
  report_rel(sState.r12, cur_state.r12, mode);
  report_rel(sState.r13, cur_state.r13, mode);
  report_rel(sState.r14, cur_state.r14, mode);
  report_rel(sState.r15, cur_state.r15, mode);
  report_rel(sState.r16, cur_state.r16, mode);
  report_rel(sState.r17, cur_state.r17, mode);

  cur_state = sState;
  client.flag_start = false;
}

s_relay read_state_rel(s_relay rel){
  s_relay ret = rel;
  ret.rel.state = digitalRead(ret.rel.pin);
  return ret;
}

s_state Read_state( ){
  s_state ret = cur_state;

  ret.r01 = read_state_rel(ret.r01);
  ret.r02 = read_state_rel(ret.r02);
  ret.r03 = read_state_rel(ret.r03);
  ret.r04 = read_state_rel(ret.r04);
  ret.r05 = read_state_rel(ret.r05);
  ret.r06 = read_state_rel(ret.r06);
  ret.r07 = read_state_rel(ret.r07);
  ret.r08 = read_state_rel(ret.r08);
  ret.r09 = read_state_rel(ret.r09);
  ret.r10 = read_state_rel(ret.r10);
  ret.r11 = read_state_rel(ret.r11);
  ret.r12 = read_state_rel(ret.r12);
  ret.r13 = read_state_rel(ret.r13);
  ret.r14 = read_state_rel(ret.r14);
  ret.r15 = read_state_rel(ret.r15);
  ret.r16 = read_state_rel(ret.r16);
  ret.r17 = read_state_rel(ret.r17);
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
