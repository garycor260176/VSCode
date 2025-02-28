#include <mqtt_ini.h>
#include <Preferences.h>
#include <cl_ld2450.h>

#define MIN_INTERVAL 100
#define MIN_STEP_DIST 10

#define def_path "LD2450_01"
mqtt_ini client( 
  "LD2450_01",
   def_path);

struct struPoint {
  int x;
  int y;
};
   
struct struZone {
  struPoint LeftTop;
  struPoint RightBottom;
};

struct s_settings{
  struZone Zone1;
  struZone Zone2;
  struZone Zone3;
  struZone Zone4;
  uint16_t step_distance;
  uint16_t interval;
};
  
struct s_state{
  int Zone1_state;
  int Zone2_state;
  int Zone3_state;
  int Zone4_state;
  uint16_t distance;
};

s_state cur_state;
s_settings ini;
Preferences preferences;

boolean Started = false;

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 32
#define RADAR_TX_PIN 33

LD2450 ld2450;

struZone readIniZone(String num){
  struZone ret;
  String name;
  name = "zone" + num + "_x_l"; ret.LeftTop.x = preferences.getInt(name.c_str(), 0);
  name = "zone" + num + "_y_t"; ret.LeftTop.y = preferences.getInt(name.c_str(), 0);
  name = "zone" + num + "_x_r"; ret.RightBottom.x = preferences.getInt(name.c_str(), 0);
  name = "zone" + num + "_y_b"; ret.RightBottom.y = preferences.getInt(name.c_str(), 0);

  //Serial.println(num + ": [" + String(ret.LeftTop.x)+", " + String(ret.LeftTop.y) + "], [" + String(ret.RightBottom.x)+", " + String(ret.RightBottom.y) + "]");

  return ret;
}
void writeIniZone(String num, struZone zone_new, struZone zone_old){
  String name;
  if(zone_new.LeftTop.x != zone_old.LeftTop.x) { name = "zone" + num + "_x_l"; preferences.putInt(name.c_str(), zone_new.LeftTop.x); }
  if(zone_new.LeftTop.y != zone_old.LeftTop.y) { name = "zone" + num + "_y_t"; preferences.putInt(name.c_str(), zone_new.LeftTop.y); }
  if(zone_new.RightBottom.x != zone_old.RightBottom.x) { name = "zone" + num + "_x_r"; preferences.putInt(name.c_str(), zone_new.RightBottom.x); }
  if(zone_new.RightBottom.y != zone_old.RightBottom.y) { name = "zone" + num + "_y_b"; preferences.putInt(name.c_str(), zone_new.RightBottom.y); }
}

s_settings read_eeprom(){
  s_settings ret;

  ret.Zone1 = readIniZone("1");
  ret.Zone2 = readIniZone("2");
  ret.Zone3 = readIniZone("3");
  ret.Zone4 = readIniZone("4");

  ret.interval = preferences.getUShort("interval", MIN_INTERVAL);
//  Serial.println("interval: " + String(ret.interval));
ret.step_distance = preferences.getUShort("step_distance", MIN_STEP_DIST);
//  Serial.println("step_distance: " + String(ret.step_distance));

  return ret;
}
void write_eeprom(s_settings _ini){
  writeIniZone("1", _ini.Zone1, ini.Zone1);
  writeIniZone("2", _ini.Zone2, ini.Zone2);
  writeIniZone("3", _ini.Zone3, ini.Zone3);
  writeIniZone("4", _ini.Zone4, ini.Zone4);

  if(_ini.interval < MIN_INTERVAL) _ini.interval = MIN_INTERVAL;
  if(_ini.interval != ini.interval)  preferences.putUShort("interval", _ini.interval);

  if(_ini.step_distance < MIN_STEP_DIST) _ini.step_distance = MIN_STEP_DIST;
  if(_ini.step_distance != ini.step_distance) preferences.putUShort("step_distance", _ini.step_distance);

  ini = _ini;
}

void setup() {
  MONITOR_SERIAL.begin(115200);
  MONITOR_SERIAL.println("\nStart...");
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);

  preferences.begin("settings", false);

  ini = read_eeprom();

  ld2450.begin(RADAR_SERIAL, ini.interval);

  client.begin();
}

void onMsgCommand( const String &message ){}
void OnLoad(){}

s_state Read(){
  s_state ret = cur_state;

  if(!ld2450.read()) return ret;

//  Serial.println(ld2450.getLastTargetMessage());

  ret.Zone1_state = ret.Zone2_state = ret.Zone3_state = ret.Zone4_state = 0;
  uint16_t dist = 0;
  for(int i=0; i<LD2450_MAX_SENSOR_TARGETS; i++){
    LD2450::RadarTarget target = ld2450.getTarget(i);
    if(!target.valid) continue;

    if(dist == 0) dist = target.distance;
    else if(dist < target.distance) dist = target.distance;
    
    if(abs(cur_state.distance - dist) >= ini.step_distance) {
      ret.distance = dist;
    }

    if(target.valid) {
      if(target.x >= ini.Zone1.LeftTop.x && target.x <= ini.Zone1.RightBottom.x && target.y >= ini.Zone1.LeftTop.y && target.y <= ini.Zone1.RightBottom.y ) {
        ret.Zone1_state = 1;
      }
      if(target.x >= ini.Zone2.LeftTop.x && target.x <= ini.Zone2.RightBottom.x && target.y >= ini.Zone2.LeftTop.y && target.y <= ini.Zone2.RightBottom.y ) {
        ret.Zone2_state = 1;
      }
      if(target.x >= ini.Zone3.LeftTop.x && target.x <= ini.Zone3.RightBottom.x && target.y >= ini.Zone3.LeftTop.y && target.y <= ini.Zone3.RightBottom.y ) {
        ret.Zone3_state = 1;
      }
      if(target.x >= ini.Zone4.LeftTop.x && target.x <= ini.Zone4.RightBottom.x && target.y >= ini.Zone4.LeftTop.y && target.y <= ini.Zone4.RightBottom.y ) {
        ret.Zone4_state = 1;
      }
    }
  }
  
  return ret;
}

void reportIniZone(String num, struZone zone){
  String topic;
  topic = "settings/Zones/Zone" + num + "/LeftTop/x"; client.Publish(topic.c_str(), String(zone.LeftTop.x)); 
  topic = "settings/Zones/Zone" + num + "/LeftTop/y"; client.Publish(topic.c_str(), String(zone.LeftTop.y)); 
  topic = "settings/Zones/Zone" + num + "/RightBottom/x"; client.Publish(topic.c_str(), String(zone.RightBottom.x)); 
  topic = "settings/Zones/Zone" + num + "/RightBottom/y"; client.Publish(topic.c_str(), String(zone.RightBottom.y)); 
}

void report_ini(s_settings ini){
  client.Publish("settings/interval", String(ini.interval));
  client.Publish("settings/step_distance", String(ini.step_distance));
  
  reportIniZone("1", ini.Zone1);
  reportIniZone("2", ini.Zone2);
  reportIniZone("3", ini.Zone3);
  reportIniZone("4", ini.Zone4);
}

void report(s_state state, int mode){
  if (mode == 0 || ( mode == 1 && cur_state.Zone1_state != state.Zone1_state ) )  {
    client.Publish("states/Zone1", String(state.Zone1_state));
  }
  if (mode == 0 || ( mode == 1 && cur_state.Zone2_state != state.Zone2_state ) )  {
    client.Publish("states/Zone2", String(state.Zone2_state));
  }
  if (mode == 0 || ( mode == 1 && cur_state.Zone3_state != state.Zone3_state ) )  {
    client.Publish("states/Zone3", String(state.Zone3_state));
  }
  if (mode == 0 || ( mode == 1 && cur_state.Zone4_state != state.Zone4_state ) )  {
    client.Publish("states/Zone4", String(state.Zone4_state));
  }

  if (mode == 0 || ( mode == 1 && cur_state.distance != state.distance ) )  {
    client.Publish("states/distance", String(state.distance));
  }

  client.flag_start = false;
  cur_state = state;
}

void OnCheckState(){}

void sub_interval(const String &message){
  s_settings _ini = ini;
  _ini.interval = ld2450.setInterval(message.toInt());
  if(_ini.interval != ini.interval){
    write_eeprom(_ini);
    if(message.toInt() != _ini.interval) client.Publish("settings/interval", String(ini.interval));
  }
}

void sub_step_distance(const String &message){
  s_settings _ini = ini;
  _ini.step_distance = message.toInt();
  if(_ini.step_distance < MIN_STEP_DIST) _ini.step_distance = MIN_STEP_DIST;
  if(_ini.step_distance != ini.step_distance){
    write_eeprom(_ini);
    if(message.toInt() != _ini.step_distance) client.Publish("settings/step_distance", String(ini.step_distance));
  }
}

void sub_Zone1_LT_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone1.LeftTop.x = message.toInt();
  if(_ini.Zone1.LeftTop.x != ini.Zone1.LeftTop.x){
    write_eeprom(_ini);
  }
}
void sub_Zone1_LT_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone1.LeftTop.y = message.toInt();
  if(_ini.Zone1.LeftTop.y != ini.Zone1.LeftTop.y){
    write_eeprom(_ini);
  }
}
void sub_Zone1_RB_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone1.RightBottom.x = message.toInt();
  if(_ini.Zone1.RightBottom.x != ini.Zone1.RightBottom.x){
    write_eeprom(_ini);
  }
}
void sub_Zone1_RB_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone1.RightBottom.y = message.toInt();
  if(_ini.Zone1.RightBottom.y != ini.Zone1.RightBottom.y){
    write_eeprom(_ini);
  }
}

void sub_Zone2_LT_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone2.LeftTop.x = message.toInt();
  if(_ini.Zone2.LeftTop.x != ini.Zone2.LeftTop.x){
    write_eeprom(_ini);
  }
}
void sub_Zone2_LT_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone2.LeftTop.y = message.toInt();
  if(_ini.Zone2.LeftTop.y != ini.Zone2.LeftTop.y){
    write_eeprom(_ini);
  }
}
void sub_Zone2_RB_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone2.RightBottom.x = message.toInt();
  if(_ini.Zone2.RightBottom.x != ini.Zone2.RightBottom.x){
    write_eeprom(_ini);
  }
}
void sub_Zone2_RB_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone2.RightBottom.y = message.toInt();
  if(_ini.Zone2.RightBottom.y != ini.Zone2.RightBottom.y){
    write_eeprom(_ini);
  }
}

void sub_Zone3_LT_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone3.LeftTop.x = message.toInt();
  if(_ini.Zone3.LeftTop.x != ini.Zone3.LeftTop.x){
    write_eeprom(_ini);
  }
}
void sub_Zone3_LT_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone3.LeftTop.y = message.toInt();
  if(_ini.Zone3.LeftTop.y != ini.Zone3.LeftTop.y){
    write_eeprom(_ini);
  }
}
void sub_Zone3_RB_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone3.RightBottom.x = message.toInt();
  if(_ini.Zone3.RightBottom.x != ini.Zone3.RightBottom.x){
    write_eeprom(_ini);
  }
}
void sub_Zone3_RB_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone3.RightBottom.y = message.toInt();
  if(_ini.Zone3.RightBottom.y != ini.Zone3.RightBottom.y){
    write_eeprom(_ini);
  }
}

void sub_Zone4_LT_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone4.LeftTop.x = message.toInt();
  if(_ini.Zone4.LeftTop.x != ini.Zone4.LeftTop.x){
    write_eeprom(_ini);
  }
}
void sub_Zone4_LT_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone4.LeftTop.y = message.toInt();
  if(_ini.Zone4.LeftTop.y != ini.Zone4.LeftTop.y){
    write_eeprom(_ini);
  }
}
void sub_Zone4_RB_X(const String &message){
  s_settings _ini = ini;
  _ini.Zone4.RightBottom.x = message.toInt();
  if(_ini.Zone4.RightBottom.x != ini.Zone4.RightBottom.x){
    write_eeprom(_ini);
  }
}
void sub_Zone4_RB_Y(const String &message){
  s_settings _ini = ini;
  _ini.Zone4.RightBottom.y = message.toInt();
  if(_ini.Zone4.RightBottom.y != ini.Zone4.RightBottom.y){
    write_eeprom(_ini);
  }
}

void onConnection(){
  if(!Started) { 
    report_ini(ini); 
    Started = true; 
  }

  client.Subscribe("settings/interval", sub_interval); 
  client.Subscribe("settings/step_distance", sub_step_distance); 
  
  client.Subscribe("settings/Zones/Zone1/LeftTop/x", sub_Zone1_LT_X); 
  client.Subscribe("settings/Zones/Zone1/LeftTop/y", sub_Zone1_LT_Y); 
  client.Subscribe("settings/Zones/Zone1/RightBottom/x", sub_Zone1_RB_X); 
  client.Subscribe("settings/Zones/Zone1/RightBottom/y", sub_Zone1_RB_Y); 

  client.Subscribe("settings/Zones/Zone2/LeftTop/x", sub_Zone2_LT_X); 
  client.Subscribe("settings/Zones/Zone2/LeftTop/y", sub_Zone2_LT_Y); 
  client.Subscribe("settings/Zones/Zone2/RightBottom/x", sub_Zone2_RB_X); 
  client.Subscribe("settings/Zones/Zone2/RightBottom/y", sub_Zone2_RB_Y); 

  client.Subscribe("settings/Zones/Zone3/LeftTop/x", sub_Zone3_LT_X); 
  client.Subscribe("settings/Zones/Zone3/LeftTop/y", sub_Zone3_LT_Y); 
  client.Subscribe("settings/Zones/Zone3/RightBottom/x", sub_Zone3_RB_X); 
  client.Subscribe("settings/Zones/Zone3/RightBottom/y", sub_Zone3_RB_Y); 

  client.Subscribe("settings/Zones/Zone4/LeftTop/x", sub_Zone4_LT_X); 
  client.Subscribe("settings/Zones/Zone4/LeftTop/y", sub_Zone4_LT_Y); 
  client.Subscribe("settings/Zones/Zone4/RightBottom/x", sub_Zone4_RB_X); 
  client.Subscribe("settings/Zones/Zone4/RightBottom/y", sub_Zone4_RB_Y); 
}

void loop() {
  client.loop();

  if(ld2450.loop()) {
    report(Read(), (client.flag_start ? 0 : 1));
  }
}