#include <mqtt_ini.h>
#include "SparkFun_VL53L1X.h"
#include <Preferences.h>

#define SHUTDOWN_PIN 2    
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor(Wire);//, SHUTDOWN_PIN, INTERRUPT_PIN);

struct s_ROI{
  int ROI_size_w = 8;
  int ROI_size_h = 8;
  int center[2] = {167, 231}; /* center of the two zones */  
  int distance;
};
int Zone = 0;
s_ROI ROI_state;

static int time_budget_in_ms = 20;
static int delay_between_measurements = 22;
int first;

struct s_Zone{
  boolean state = false;
  uint32_t t_up = 0; 
  uint32_t t_down = 0;
};

struct s_state{
  s_Zone zone1;
  s_Zone zone2;
  int dir = 0;
  int Counter = 0;
	boolean any_HIGH;
};

s_state cur_state;

#define def_path "counters/cnt02"

mqtt_ini client( 
  "ESP32_CNT02",
   def_path);

Preferences preferences; 

void read_eeprom(s_ROI* ROI){
  ROI->ROI_size_w = preferences.getInt("ROIs", 8);  Serial.println("read ROI_size_w = " + String(ROI->ROI_size_w));
  ROI->ROI_size_h = preferences.getInt("ROIsH", 8);  Serial.println("read ROI_size_h = " + String(ROI->ROI_size_h));
  ROI->center[0] = preferences.getInt("c0", 137);  Serial.println("read center[0] = " + String(ROI->center[0]));
  ROI->center[1] = preferences.getInt("c1", 231);  Serial.println("read center[1] = " + String(ROI->center[1]));
  ROI->distance = preferences.getInt("dist", 1500);  Serial.println("read distance = " + String(ROI->distance));
}

void write_eeprom(){
  s_ROI old;
  read_eeprom(&old);

  if(old.ROI_size_w != ROI_state.ROI_size_w) {
    preferences.putInt("ROIs", ROI_state.ROI_size_w);
    Serial.println("write ROI_size_w = " + String(ROI_state.ROI_size_w));
  }

  if(old.ROI_size_h != ROI_state.ROI_size_h) {
    preferences.putInt("ROIsH", ROI_state.ROI_size_h);
    Serial.println("write ROI_size_h = " + String(ROI_state.ROI_size_h));
  }

  if(old.center[0] != ROI_state.center[0]) {
    preferences.putInt("c0", ROI_state.center[0]);
    Serial.println("write center[0] = " + String(ROI_state.center[0]));
  }

  if(old.center[1] != ROI_state.center[1]) {
    preferences.putInt("c1", ROI_state.center[1]);
    Serial.println("write center[1] = " + String(ROI_state.center[1]));
  }

  if(old.distance != ROI_state.distance) {
    preferences.putInt("dist", ROI_state.distance);
    Serial.println("write distance = " + String(ROI_state.distance));
  }
}

void report(s_state state, int mode = 0);

void setup(void)
{
  Wire.begin();
  Serial.begin(115200);

  preferences.begin("settings", false);
  read_eeprom(&ROI_state);

  Serial.println("VL53L1X Qwiic Test");
  if (distanceSensor.init() == false)
    Serial.println("Sensor online!");

  client.begin();
  delay(1000);
}

s_Zone Clear_time(s_Zone sensor){
  s_Zone ret = sensor;
  ret.t_down  = ret.t_up = 0;
  return ret;
}

void onMsgCommand( const String &message ){
  if(message == "clear") {
    s_state state = cur_state;
    state.Counter = 0;
    report(state);
  }
}

void OnLoad(){}

void OnCheckState(){}

void Msg_ROI_size_w( const String &message ){
  if(ROI_state.ROI_size_w == message.toInt( )) return;
  ROI_state.ROI_size_w = message.toInt( );
  write_eeprom();
}

void Msg_ROI_size_h( const String &message ){
  if(ROI_state.ROI_size_h == message.toInt( )) return;
  ROI_state.ROI_size_h = message.toInt( );
  write_eeprom();
}

void Msg_Zone1( const String &message ){
  if(ROI_state.center[0] == message.toInt( )) return;
  ROI_state.center[0] = message.toInt();
  write_eeprom();
}

void Msg_Zone2( const String &message ){
  if(ROI_state.center[1] == message.toInt( )) return;
  ROI_state.center[1] = message.toInt();
  write_eeprom();
}


void onConnection(){
  client.Subscribe("settings/ROI_size", Msg_ROI_size_w); 
  client.Subscribe("settings/ROI_size_h", Msg_ROI_size_h); 
  client.Subscribe("settings/center0", Msg_Zone1); 
  client.Subscribe("settings/center1", Msg_Zone2); 
}

void report(s_state state, int mode ){
  if (mode == 0 || ( mode == 1 && cur_state.Counter != state.Counter ) )  {
    client.Publish("Counter", String(state.Counter));
  }
  if (mode == 0 || ( mode == 1 && cur_state.any_HIGH != state.any_HIGH ) )  {
    client.Publish("any_HIGH", (state.any_HIGH ? "1" : "0"));
  }
  if (mode == 0 || ( mode == 1 && cur_state.dir != state.dir ) )  {
    client.Publish("dir", String(state.dir));
  }

  if(mode == 0){
    client.Publish("settings/ROI_size", String(ROI_state.ROI_size_w));
    client.Publish("settings/ROI_size_h", String(ROI_state.ROI_size_h));
    client.Publish("settings/distance", String(ROI_state.distance));
    client.Publish("settings/center0", String(ROI_state.center[0]));
    client.Publish("settings/center1", String(ROI_state.center[1]));
  }

  client.flag_start = false;
  cur_state = state;
}

void loop(void)
{
  client.loop();

  s_state state = cur_state;

  uint16_t distance;

  state.dir = 0;

  distanceSensor.setROI(ROI_state.ROI_size_h, ROI_state.ROI_size_w, ROI_state.center[Zone]);  // first value: height of the zone, second value: width of the zone
  delay(delay_between_measurements);
  distanceSensor.setTimingBudgetInMs(time_budget_in_ms);
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();

  boolean changed = false;

  switch(Zone){
    case 0:
      state.zone1.state = (distance <= ROI_state.distance);
      changed = (state.zone1.state != cur_state.zone1.state);
      if(state.zone1.state) {
        state.zone1.t_up = millis();
      } else {
        state.zone1.t_down = millis();
      }
    break;

    case 1:
      state.zone2.state = (distance <= ROI_state.distance);
      changed = (state.zone2.state != cur_state.zone2.state);
      if(state.zone2.state) {
        state.zone2.t_up = millis();
      } else {
        state.zone2.t_down = millis();
      }
    break;
  }

  if(!cur_state.zone1.state && !cur_state.zone2.state) first = 0;
  state.any_HIGH = (state.zone1.state || state.zone2.state);

  if(changed) {
    if(first == 0) {
        if(state.zone1.state && !state.zone2.state) {
          first = 1;
        } else 
        if(!state.zone1.state && state.zone2.state) {
          first = 2;
        }
    }

    if(!state.zone1.state && !state.zone2.state && ( cur_state.zone1.state || cur_state.zone2.state)) {
      switch(first){
        case 1:
          if(
            state.zone1.t_up <= state.zone2.t_up &&
            state.zone2.t_up <= state.zone1.t_down &&
            state.zone1.t_down <= state.zone2.t_down
          ) {
            state.dir = 1;
          }
        break;

        case 2:
          if(
            state.zone2.t_up <= state.zone1.t_up &&
            state.zone1.t_up <= state.zone2.t_down &&
            state.zone2.t_down <= state.zone1.t_down
          ) {
            state.dir = 2;
          }
        break;
      }
    }
  }

  switch(state.dir){
    case 1: state.Counter++; break;
    case 2: state.Counter--; if(state.Counter < 0) state.Counter = 0; break;
  }

  if(!state.zone1.state && !state.zone2.state) {
    first = 0;
    state.zone1 = Clear_time(state.zone1);
    state.zone2 = Clear_time(state.zone2);
		state.any_HIGH = false;
	}

  Zone++;
  if(Zone > 1) Zone = 0;

  report(state, (client.flag_start ? 0 : 1)); //отправляем все
}