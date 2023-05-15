#include <mqtt_ini.h>
#include <AsyncUDP.h>
#include <ArduinoJson.h>

#define def_path "rb01"

#define PIN_01   15
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
#define PIN_17   13

IPAddress local_IP(192, 168, 1, 150);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
AsyncUDP udp;
const uint16_t port = 8888;

mqtt_ini client( 
  "ESP32_REL01",     // Client name that uniquely identify your device
   def_path);

struct s_level {
  int pin;
  int state;
  int state_pin;
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

void parsePacket(AsyncUDPPacket packet);

void setup() {
  Serial.begin(115200);                                         
  Serial.println("");  Serial.println("Start!");

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

// Настраивает статический IP-адрес
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  client.begin(true);

  if(udp.listen(port)) {
      udp.onPacket(parsePacket);
  }
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

void set_State(int pin, int state){
  int _state = state;
  if(_state != HIGH) _state = LOW;
  digitalWrite(pin, _state);
}

void msg_subscr_01(const String &message){
  Serial.println("msg_subscr_01 = " + message);
  set_State(cur_state.r01.rel.pin, message.toInt());
}
void msg_subscr_02(const String &message){
  Serial.println("msg_subscr_02 = " + message);
  set_State(cur_state.r02.rel.pin, message.toInt());
}
void msg_subscr_03(const String &message){
  Serial.println("msg_subscr_03 = " + message);
  set_State(cur_state.r03.rel.pin, message.toInt());
}
void msg_subscr_04(const String &message){
  Serial.println("msg_subscr_04 = " + message);
  set_State(cur_state.r04.rel.pin, message.toInt());
}
void msg_subscr_05(const String &message){
  Serial.println("msg_subscr_05 = " + message);
  set_State(cur_state.r05.rel.pin, message.toInt());
}
void msg_subscr_06(const String &message){
  Serial.println("msg_subscr_06 = " + message);
  set_State(cur_state.r06.rel.pin, message.toInt());
}
void msg_subscr_07(const String &message){
  Serial.println("msg_subscr_07 = " + message);
  set_State(cur_state.r07.rel.pin, message.toInt());
}
void msg_subscr_08(const String &message){
  Serial.println("msg_subscr_08 = " + message);
  set_State(cur_state.r08.rel.pin, message.toInt());
}
void msg_subscr_09(const String &message){
  Serial.println("msg_subscr_09 = " + message);
  set_State(cur_state.r09.rel.pin, message.toInt());
}
void msg_subscr_10(const String &message){
  Serial.println("msg_subscr_10 = " + message);
  set_State(cur_state.r10.rel.pin, message.toInt());
}
void msg_subscr_11(const String &message){
  Serial.println("msg_subscr_11 = " + message);
  set_State(cur_state.r11.rel.pin, message.toInt());
}
void msg_subscr_12(const String &message){
  Serial.println("msg_subscr_12 = " + message);
  set_State(cur_state.r12.rel.pin, message.toInt());
}
void msg_subscr_13(const String &message){
  Serial.println("msg_subscr_13 = " + message);
  set_State(cur_state.r13.rel.pin, message.toInt());
}
void msg_subscr_14(const String &message){
  Serial.println("msg_subscr_14 = " + message);
  set_State(cur_state.r14.rel.pin, message.toInt());
}
void msg_subscr_15(const String &message){
  Serial.println("msg_subscr_15 = " + message);
  set_State(cur_state.r15.rel.pin, message.toInt());
}
void msg_subscr_16(const String &message){
  Serial.println("msg_subscr_16 = " + message);
  set_State(cur_state.r16.rel.pin, message.toInt());
}
void msg_subscr_17(const String &message){
  Serial.println("msg_subscr_17 = " + message);
  set_State(cur_state.r17.rel.pin, message.toInt());
}

void onConnection(){
  client.Subscribe("r01/relay_state",  msg_subscr_01);
  client.Subscribe("r02/relay_state",  msg_subscr_02);
  client.Subscribe("r03/relay_state",  msg_subscr_03);
  client.Subscribe("r04/relay_state",  msg_subscr_04);
  client.Subscribe("r05/relay_state",  msg_subscr_05);
  client.Subscribe("r06/relay_state",  msg_subscr_06);
  client.Subscribe("r07/relay_state",  msg_subscr_07);
  client.Subscribe("r08/relay_state",  msg_subscr_08);
  client.Subscribe("r09/relay_state",  msg_subscr_09);
  client.Subscribe("r10/relay_state",  msg_subscr_10);
  client.Subscribe("r11/relay_state",  msg_subscr_11);
  client.Subscribe("r12/relay_state",  msg_subscr_12);
  client.Subscribe("r13/relay_state",  msg_subscr_13);
  client.Subscribe("r14/relay_state",  msg_subscr_14);
  client.Subscribe("r15/relay_state",  msg_subscr_15);
  client.Subscribe("r16/relay_state",  msg_subscr_16);
  client.Subscribe("r17/relay_state",  msg_subscr_17);
}

void OnLoad(){
}

void loop() {
  client.loop();
}

void parsePacket(AsyncUDPPacket packet)
{
  String msg = packet.readString();
  Serial.println("UDP message: " + msg);
  packet.print("ok!");

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, msg);  
  if( error ) { Serial.printf("Error on deserialization: %s\n", error.c_str() ); return; }

  String r = doc["r"]; //номер реле
  String s = doc["s"]; //состояние
  int state = LOW;
  int pin = 0;
  if(s == "1") state = HIGH;

  if(r == "01")      pin = cur_state.r01.rel.pin;
  else if(r == "02") pin = cur_state.r02.rel.pin;
  else if(r == "03") pin = cur_state.r03.rel.pin;
  else if(r == "04") pin = cur_state.r04.rel.pin;
  else if(r == "05") pin = cur_state.r05.rel.pin;
  else if(r == "06") pin = cur_state.r06.rel.pin;
  else if(r == "07") pin = cur_state.r07.rel.pin;
  else if(r == "08") pin = cur_state.r08.rel.pin;
  else if(r == "09") pin = cur_state.r09.rel.pin;
  else if(r == "10") pin = cur_state.r10.rel.pin;
  else if(r == "11") pin = cur_state.r11.rel.pin;
  else if(r == "12") pin = cur_state.r12.rel.pin;
  else if(r == "13") pin = cur_state.r13.rel.pin;
  else if(r == "14") pin = cur_state.r14.rel.pin;
  else if(r == "15") pin = cur_state.r15.rel.pin;
  else if(r == "16") pin = cur_state.r16.rel.pin;
  else if(r == "17") pin = cur_state.r17.rel.pin;

  if(pin > 0){
    set_State(pin, state);
  }
}

void onMsgCommand( const String &message ){
  int pin = 0;
  int state = LOW;
Serial.println("onMsgCommand: " + message);
  if(message == "01-on")        { pin = cur_state.r01.rel.pin; state = HIGH; }
  else if(message == "01-off")  pin = cur_state.r01.rel.pin; 

  else if(message == "02-on")   { pin = cur_state.r02.rel.pin; state = HIGH; }
  else if(message == "02-off")  pin = cur_state.r02.rel.pin; 

  else if(message == "03-on")   { pin = cur_state.r03.rel.pin; state = HIGH; }
  else if(message == "03-off")  pin = cur_state.r03.rel.pin; 

  else if(message == "04-on")   { pin = cur_state.r04.rel.pin; state = HIGH; }
  else if(message == "04-off")  pin = cur_state.r04.rel.pin; 
  
  else if(message == "05-on")   { pin = cur_state.r05.rel.pin; state = HIGH; }
  else if(message == "05-off")  pin = cur_state.r05.rel.pin; 
  
  else if(message == "06-on")   { pin = cur_state.r06.rel.pin; state = HIGH; }
  else if(message == "06-off")  pin = cur_state.r06.rel.pin; 
  
  else if(message == "07-on")   { pin = cur_state.r07.rel.pin; state = HIGH; }
  else if(message == "07-off")  pin = cur_state.r07.rel.pin; 
  
  else if(message == "08-on")   { pin = cur_state.r08.rel.pin; state = HIGH; }
  else if(message == "08-off")  pin = cur_state.r08.rel.pin; 
  
  else if(message == "09-on")   { pin = cur_state.r09.rel.pin; state = HIGH; }
  else if(message == "09-off")  pin = cur_state.r09.rel.pin; 
  
  else if(message == "10-on")   { pin = cur_state.r10.rel.pin; state = HIGH; }
  else if(message == "10-off")  pin = cur_state.r10.rel.pin; 
  
  else if(message == "11-on")   { pin = cur_state.r11.rel.pin; state = HIGH; }
  else if(message == "11-off")  pin = cur_state.r11.rel.pin; 
  
  else if(message == "12-on")   { pin = cur_state.r12.rel.pin; state = HIGH; }
  else if(message == "12-off")  pin = cur_state.r12.rel.pin; 
  
  else if(message == "13-on")   { pin = cur_state.r13.rel.pin; state = HIGH; }
  else if(message == "13-off")  pin = cur_state.r13.rel.pin; 
  
  else if(message == "14-on")   { pin = cur_state.r14.rel.pin; state = HIGH; }
  else if(message == "14-off")  pin = cur_state.r14.rel.pin; 
  
  else if(message == "15-on")   { pin = cur_state.r15.rel.pin; state = HIGH; }
  else if(message == "15-off")  pin = cur_state.r15.rel.pin; 
  
  else if(message == "16-on")   { pin = cur_state.r16.rel.pin; state = HIGH; }
  else if(message == "16-off")  pin = cur_state.r16.rel.pin; 
  
  else if(message == "17-on")   { pin = cur_state.r17.rel.pin; state = HIGH; }
  else if(message == "17-off")  pin = cur_state.r17.rel.pin; 

  if(pin > 0){
    set_State(pin, state);
  }
}