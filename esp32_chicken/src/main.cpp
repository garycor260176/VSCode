#include <mqtt_ini.h>
#include "DHT.h"
#include <Preferences.h>

#include <cl_dht22.h>

#define def_path "CHICKEN"
#define def_subpath_dht22     "dht22"
#define def_relays  3

int pins[] = {
   23, //свет
   5, //вытяжка
   14 //отопление
}; 

mqtt_ini client( 
  "ESP32_CHICKEN",
   def_path);

#define DHTPIN              32
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
cl_dht22 dht22(&dht, &client, String(def_subpath_dht22), 30000, false);

struct s_relay { 
  String name;
  int state;
  int mode;
};

struct s_ini{
  int t_on;
  int t_off;
  int h_on;
  int h_off;
};

struct s_state{
  s_relay relays[def_relays];
  s_ini ini;
};

s_state cur_state;

Preferences preferences; 

String getName(int i) {
  String ret = String(i);
  if(ret.length() == 1) ret = "r0" + ret;
  else ret = "r" + ret;
  return ret;
}

void read_eeprom(s_state* state, boolean debug = true){
  for(int i = 0; i < def_relays; i++) {
    String t = getName(i) + "_mode";
    state->relays[i].mode = preferences.getInt(t.c_str(), 0);
    if(state->relays[i].mode < 0 || state->relays[i].mode > 2) state->relays[i].mode = 0;
    if(debug) Serial.println("read " + t + " = " + String(state->relays[i].mode));
  }
  
  state->ini.t_on = preferences.getInt("t_on", 10);  if(debug) Serial.println("read t_on = " + String(state->ini.t_on));
  state->ini.t_off = preferences.getInt("t_off", 20);  if(debug) Serial.println("read t_off = " + String(state->ini.t_off));

  state->ini.h_on = preferences.getInt("h_on", 10);  if(debug) Serial.println("read h_on = " + String(state->ini.h_on));
  state->ini.h_off = preferences.getInt("h_off", 20);  if(debug) Serial.println("read h_off = " + String(state->ini.h_off));
}

void write_eeprom(){
  s_state readed;
  read_eeprom(&readed, false);

  for(int i = 0; i < def_relays; i++) {
    if(cur_state.relays[i].mode != readed.relays[i].mode) {
      String t = getName(i) + "_mode";
      preferences.putInt(t.c_str(), cur_state.relays[i].mode);
      Serial.println("write " + t + " = " + String(cur_state.relays[i].mode));
    }
  }

  if(cur_state.ini.t_on != readed.ini.t_on) {
    preferences.putInt("t_on", cur_state.ini.t_on);
    Serial.println("write t_on = " + String(cur_state.ini.t_on));
  }
  if(cur_state.ini.t_off != readed.ini.t_off) {
    preferences.putInt("t_off", cur_state.ini.t_off);
    Serial.println("write t_off = " + String(cur_state.ini.t_off));
  }

  if(cur_state.ini.h_on != readed.ini.h_on) {
    preferences.putInt("h_on", cur_state.ini.h_on);
    Serial.println("write h_on = " + String(cur_state.ini.h_on));
  }
  if(cur_state.ini.h_off != readed.ini.h_off) {
    preferences.putInt("h_off", cur_state.ini.h_off);
    Serial.println("write h_off = " + String(cur_state.ini.h_off));
  }
}

void setup() {
  Serial.begin(115200);                                         
  Serial.println("");  Serial.println("Start!");

  preferences.begin("settings", false);

  dht22.begin();

  for(int i = 0; i < def_relays; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
    cur_state.relays[i].name = getName(i);
  }
  read_eeprom(&cur_state);

  client.begin(true);
}

void onMsgCommand( const String &message ){}
void OnLoad(){}

void OnCheckState(){
}

void Msg_t_on( const String &message ){
  cur_state.ini.t_on = message.toInt( );
  write_eeprom();
}
void Msg_t_off( const String &message ){
  cur_state.ini.t_off = message.toInt( );
  write_eeprom();
}

void Msg_h_on( const String &message ){
  cur_state.ini.h_on = message.toInt( );
  write_eeprom();
}
void Msg_h_off( const String &message ){
  cur_state.ini.h_off = message.toInt( );
  write_eeprom();
}

void Msg_relays_mode(const String &topic, const String &message) {
  for(int i = 0; i < def_relays; i++) {
    if(topic == "CHICKEN/modes/" + getName(i)) {
      cur_state.relays[i].mode = message.toInt();
      if(cur_state.relays[i].mode < 0 || cur_state.relays[i].mode > 2) cur_state.relays[i].mode = 0;
      write_eeprom();
      return;
    }    
  }
}

void onConnection(){
  dht22.subscribe( );

  for(int i = 0; i < def_relays; i++) {
    client.Subscribe("modes/" + getName(i), Msg_relays_mode); 
  }
  
  client.Subscribe("settings/t/on", Msg_t_on); 
  client.Subscribe("settings/t/off", Msg_t_off); 
  client.Subscribe("settings/h/on", Msg_h_on); 
  client.Subscribe("settings/h/off", Msg_h_off); 
}

void report(int mode ){
  for(int i = 0; i < def_relays; i++) {
    if (mode == 0 || ( mode == 1 && cur_state.relays[i].state != digitalRead(pins[i]) ) )  {

      digitalWrite(pins[i], cur_state.relays[i].state);
      client.Publish("states/" + getName(i), String(cur_state.relays[i].state));
    }
    if (mode == 0 )  {
      client.Publish("modes/" + getName(i), String(cur_state.relays[i].mode));
    }
  }

  if (mode == 0 )  {
    client.Publish("settings/h/on", String(cur_state.ini.h_on));
    client.Publish("settings/h/off", String(cur_state.ini.h_off));
    client.Publish("settings/t/on", String(cur_state.ini.t_on));
    client.Publish("settings/t/off", String(cur_state.ini.t_off));
  }

  client.flag_start = false;
}

void loop() {
  client.loop();

  dht22.loop( );
  
  s_dht22_val dht22val = dht22.get_last_value();

//свет
  switch(cur_state.relays[0].mode){
    case 1: cur_state.relays[0].state = HIGH; break;
    case 2: cur_state.relays[0].state = LOW; break;
  }

//вытяжка
  switch(cur_state.relays[1].mode){
    case 1: cur_state.relays[1].state = HIGH; break;
    case 2: cur_state.relays[1].state = LOW; break;
    default:
      if(dht22val.readed){
        if(dht22val.h_value >= cur_state.ini.h_on) {
          cur_state.relays[1].state = HIGH;
        } else if(dht22val.h_value < cur_state.ini.h_off) {
          cur_state.relays[1].state = LOW;
        }
      }
    break;
  }
  
//отопление
  switch(cur_state.relays[2].mode){
    case 1: cur_state.relays[2].state = HIGH; break;
    case 2: cur_state.relays[2].state = LOW; break;
    default:
      if(dht22val.readed){
        if(dht22val.t_value <= cur_state.ini.t_on) {
          cur_state.relays[2].state = HIGH;
        } else if(dht22val.t_value > cur_state.ini.t_off) {
          cur_state.relays[2].state = LOW;
        }
      }
    break;
  }

  report((client.flag_start ? 0 : 1)); //отправляем все
}