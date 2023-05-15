#include <mqtt_ini.h>

#define def_path "outside/pump_01"

#define PIN_RELAY_PUMP     13
#define PIN_RELAY_VALVE_01 19
#define PIN_RELAY_VALVE_02 18
#define PIN_RELAY_VALVE_03 5
#define PIN_RELAY_VALVE_04 17
#define PIN_RELAY_VALVE_05 16
#define PIN_RELAY_VALVE_06 4
#define PIN_RELAY_VALVE_07 2
#define PIN_RELAY_VALVE_08 32
#define PIN_RELAY_VALVE_09 33
#define PIN_RELAY_VALVE_10 25
#define PIN_RELAY_VALVE_11 26
#define PIN_RELAY_VALVE_12 27
#define PIN_RELAY_VALVE_13 14
#define PIN_RELAY_VALVE_14 12

struct s_relay{
  int pin;
  int state;
  boolean changed;
  String name;
};

struct s_state {
  s_relay pump;
  s_relay valve_01;
  s_relay valve_02;
  s_relay valve_03;
  s_relay valve_04;
  s_relay valve_05;
  s_relay valve_06;
  s_relay valve_07;
  s_relay valve_08;
  s_relay valve_09;
  s_relay valve_10;
  s_relay valve_11;
  s_relay valve_12;
  s_relay valve_13;
  s_relay valve_14;

  int cnt;
};

s_state cur_state;

mqtt_ini client( 
  "ESP32_PUMP_01",    
   def_path);

void report(int);

void setup() { 
  Serial.begin(115200);

  cur_state.pump.pin = PIN_RELAY_PUMP; cur_state.pump.name = "pump"; pinMode(cur_state.pump.pin, OUTPUT); digitalWrite(cur_state.pump.pin, LOW);
  cur_state.valve_01.pin = PIN_RELAY_VALVE_01; cur_state.valve_01.name = "01"; pinMode(cur_state.valve_01.pin, OUTPUT); digitalWrite(cur_state.valve_01.pin, LOW);
  cur_state.valve_02.pin = PIN_RELAY_VALVE_02; cur_state.valve_02.name = "02"; pinMode(cur_state.valve_02.pin, OUTPUT); digitalWrite(cur_state.valve_02.pin, LOW);
  cur_state.valve_03.pin = PIN_RELAY_VALVE_03; cur_state.valve_03.name = "03"; pinMode(cur_state.valve_03.pin, OUTPUT); digitalWrite(cur_state.valve_03.pin, LOW);
  cur_state.valve_04.pin = PIN_RELAY_VALVE_04; cur_state.valve_04.name = "04"; pinMode(cur_state.valve_04.pin, OUTPUT); digitalWrite(cur_state.valve_04.pin, LOW);
  cur_state.valve_05.pin = PIN_RELAY_VALVE_05; cur_state.valve_05.name = "05"; pinMode(cur_state.valve_05.pin, OUTPUT); digitalWrite(cur_state.valve_05.pin, LOW);
  cur_state.valve_06.pin = PIN_RELAY_VALVE_06; cur_state.valve_06.name = "06"; pinMode(cur_state.valve_06.pin, OUTPUT); digitalWrite(cur_state.valve_06.pin, LOW);
  cur_state.valve_07.pin = PIN_RELAY_VALVE_07; cur_state.valve_07.name = "07"; pinMode(cur_state.valve_07.pin, OUTPUT); digitalWrite(cur_state.valve_07.pin, LOW);
  cur_state.valve_08.pin = PIN_RELAY_VALVE_08; cur_state.valve_08.name = "08"; pinMode(cur_state.valve_08.pin, OUTPUT); digitalWrite(cur_state.valve_08.pin, LOW);
  cur_state.valve_09.pin = PIN_RELAY_VALVE_09; cur_state.valve_09.name = "09"; pinMode(cur_state.valve_09.pin, OUTPUT); digitalWrite(cur_state.valve_09.pin, LOW);
  cur_state.valve_10.pin = PIN_RELAY_VALVE_10; cur_state.valve_10.name = "10"; pinMode(cur_state.valve_10.pin, OUTPUT); digitalWrite(cur_state.valve_10.pin, LOW);
  cur_state.valve_11.pin = PIN_RELAY_VALVE_11; cur_state.valve_11.name = "11"; pinMode(cur_state.valve_11.pin, OUTPUT); digitalWrite(cur_state.valve_11.pin, LOW);
  cur_state.valve_12.pin = PIN_RELAY_VALVE_12; cur_state.valve_12.name = "12"; pinMode(cur_state.valve_12.pin, OUTPUT); digitalWrite(cur_state.valve_12.pin, LOW);
  cur_state.valve_13.pin = PIN_RELAY_VALVE_13; cur_state.valve_13.name = "13"; pinMode(cur_state.valve_13.pin, OUTPUT); digitalWrite(cur_state.valve_13.pin, LOW);
  cur_state.valve_14.pin = PIN_RELAY_VALVE_14; cur_state.valve_14.name = "14"; pinMode(cur_state.valve_14.pin, OUTPUT); digitalWrite(cur_state.valve_14.pin, LOW);

  client.begin();
}

void On_Off_PUMP(s_relay &rel, int new_state){
  int old_state = digitalRead(rel.pin);
  if(old_state != new_state){
    rel.changed = true;
    digitalWrite(rel.pin, new_state);
  }
}

void On_Off_Valve(s_relay &rel, int new_state){
  int old_state = digitalRead(rel.pin);
  if(old_state != new_state){
    rel.changed = true;
    digitalWrite(rel.pin, new_state);
    if(new_state == HIGH) cur_state.cnt++;
    else                  cur_state.cnt--;

    if(cur_state.cnt <= 0){
      cur_state.cnt = 0;
      On_Off_PUMP(cur_state.pump, LOW);
    } else {
      On_Off_PUMP(cur_state.pump, HIGH);
    }
  }
}

void all_off(){
  On_Off_Valve(cur_state.valve_01, LOW);
  On_Off_Valve(cur_state.valve_02, LOW);
  On_Off_Valve(cur_state.valve_03, LOW);
  On_Off_Valve(cur_state.valve_04, LOW);
  On_Off_Valve(cur_state.valve_05, LOW);
  On_Off_Valve(cur_state.valve_06, LOW);
  On_Off_Valve(cur_state.valve_07, LOW);
  On_Off_Valve(cur_state.valve_08, LOW);
  On_Off_Valve(cur_state.valve_09, LOW);
  On_Off_Valve(cur_state.valve_10, LOW);
  On_Off_Valve(cur_state.valve_11, LOW);
  On_Off_Valve(cur_state.valve_12, LOW);
  On_Off_Valve(cur_state.valve_13, LOW);
  On_Off_Valve(cur_state.valve_14, LOW);
  On_Off_PUMP(cur_state.pump, LOW);
}

void onMsgCommand( const String &message ){
  if(message == "valve_01-on"){
    On_Off_Valve(cur_state.valve_01, HIGH);
  } else if(message == "valve_01-off"){
    On_Off_Valve(cur_state.valve_01, LOW);
  }
  else if(message == "valve_02-on"){
    On_Off_Valve(cur_state.valve_02, HIGH);
  } else if(message == "valve_02-off"){
    On_Off_Valve(cur_state.valve_02, LOW);
  }
  else if(message == "valve_03-on"){
    On_Off_Valve(cur_state.valve_03, HIGH);
  } else if(message == "valve_03-off"){
    On_Off_Valve(cur_state.valve_03, LOW);
  }
  else if(message == "valve_04-on"){
    On_Off_Valve(cur_state.valve_04, HIGH);
  } else if(message == "valve_04-off"){
    On_Off_Valve(cur_state.valve_04, LOW);
  }
  else if(message == "valve_05-on"){
    On_Off_Valve(cur_state.valve_05, HIGH);
  } else if(message == "valve_05-off"){
    On_Off_Valve(cur_state.valve_05, LOW);
  }
  else if(message == "valve_06-on"){
    On_Off_Valve(cur_state.valve_06, HIGH);
  } else if(message == "valve_06-off"){
    On_Off_Valve(cur_state.valve_06, LOW);
  }
  else if(message == "valve_07-on"){
    On_Off_Valve(cur_state.valve_07, HIGH);
  } else if(message == "valve_07-off"){
    On_Off_Valve(cur_state.valve_07, LOW);
  }
  else if(message == "valve_08-on"){
    On_Off_Valve(cur_state.valve_08, HIGH);
  } else if(message == "valve_08-off"){
    On_Off_Valve(cur_state.valve_08, LOW);
  }
  else if(message == "valve_09-on"){
    On_Off_Valve(cur_state.valve_09, HIGH);
  } else if(message == "valve_09-off"){
    On_Off_Valve(cur_state.valve_09, LOW);
  }
  else if(message == "valve_10-on"){
    On_Off_Valve(cur_state.valve_10, HIGH);
  } else if(message == "valve_10-off"){
    On_Off_Valve(cur_state.valve_10, LOW);
  }
  else if(message == "valve_11-on"){
    On_Off_Valve(cur_state.valve_11, HIGH);
  } else if(message == "valve_11-off"){
    On_Off_Valve(cur_state.valve_11, LOW);
  }
  else if(message == "valve_12-on"){
    On_Off_Valve(cur_state.valve_12, HIGH);
  } else if(message == "valve_12-off"){
    On_Off_Valve(cur_state.valve_12, LOW);
  }
  else if(message == "valve_13-on"){
    On_Off_Valve(cur_state.valve_13, HIGH);
  } else if(message == "valve_13-off"){
    On_Off_Valve(cur_state.valve_13, LOW);
  }
  else if(message == "valve_14-on"){
    On_Off_Valve(cur_state.valve_14, HIGH);
  } else if(message == "valve_14-off"){
    On_Off_Valve(cur_state.valve_14, LOW);
  }
}

void onConnection()
{
}

void read_state_rel(s_relay &ret){
  int val = digitalRead(ret.pin);

  if(val != ret.state){
    ret.state = val;
    ret.changed = true;
  }
}

void Read_state( ){
  cur_state.pump.changed = false;
  cur_state.valve_01.changed = false;
  cur_state.valve_02.changed = false;
  cur_state.valve_03.changed = false;
  cur_state.valve_04.changed = false;
  cur_state.valve_05.changed = false;
  cur_state.valve_06.changed = false;
  cur_state.valve_07.changed = false;
  cur_state.valve_08.changed = false;
  cur_state.valve_09.changed = false;
  cur_state.valve_10.changed = false;
  cur_state.valve_11.changed = false;
  cur_state.valve_12.changed = false;
  cur_state.valve_13.changed = false;
  cur_state.valve_14.changed = false;

  if(!client.client->isConnected()){
    all_off();
    client.flag_start = false;
  }

  read_state_rel(cur_state.pump);
  read_state_rel(cur_state.valve_01);
  read_state_rel(cur_state.valve_02);
  read_state_rel(cur_state.valve_03);
  read_state_rel(cur_state.valve_04);
  read_state_rel(cur_state.valve_05);
  read_state_rel(cur_state.valve_06);
  read_state_rel(cur_state.valve_07);
  read_state_rel(cur_state.valve_08);
  read_state_rel(cur_state.valve_09);
  read_state_rel(cur_state.valve_10);
  read_state_rel(cur_state.valve_11);
  read_state_rel(cur_state.valve_12);
  read_state_rel(cur_state.valve_13);
  read_state_rel(cur_state.valve_14);
}

void report_rel(int mode, s_relay &rel){
  if(mode == 0 || ( mode == 1 && rel.changed ) ){
    client.Publish(rel.name + "/state", String(rel.state));
  }
}

void report( int mode ){
  report_rel(mode, cur_state.pump);
  report_rel(mode, cur_state.valve_01);
  report_rel(mode, cur_state.valve_02);
  report_rel(mode, cur_state.valve_03);
  report_rel(mode, cur_state.valve_04);
  report_rel(mode, cur_state.valve_05);
  report_rel(mode, cur_state.valve_06);
  report_rel(mode, cur_state.valve_07);
  report_rel(mode, cur_state.valve_08);
  report_rel(mode, cur_state.valve_09);
  report_rel(mode, cur_state.valve_10);
  report_rel(mode, cur_state.valve_11);
  report_rel(mode, cur_state.valve_12);
  report_rel(mode, cur_state.valve_13);
  report_rel(mode, cur_state.valve_14);
 
  client.flag_start = false;
}

void OnCheckState(){
  Read_state( );

  if(client.flag_start){ //первый запуск
    report(0); //отправляем все
  } else {
    report(1); //отправляем только то, что изменилось
  }
}

void OnLoad(){

}

void loop() {
  client.loop();
}