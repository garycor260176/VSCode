#include <mqtt_ini.h>
#include "SparkFun_VL53L1X.h"
#include <Preferences.h>

#define SHUTDOWN_PIN 2    
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor(Wire);//, SHUTDOWN_PIN, INTERRUPT_PIN);

struct s_ROI{
  int ROI_size = 8;
  int center[2] = {167, 231}; /* center of the two zones */  
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

#define def_path "counters/cnt01"

mqtt_ini client( 
  "ESP32_CNT01",
   def_path);

Preferences preferences; 

void read_eeprom(s_ROI* ROI){
  ROI->ROI_size = preferences.getInt("ROIs", 8);  Serial.println("read ROI_size = " + String(ROI->ROI_size));
  ROI->center[0] = preferences.getInt("c0", 137);  Serial.println("read center[0] = " + String(ROI->center[0]));
  ROI->center[1] = preferences.getInt("c1", 231);  Serial.println("read center[1] = " + String(ROI->center[1]));
}

void write_eeprom(){
  s_ROI old;
  read_eeprom(&old);

  if(old.ROI_size != ROI_state.ROI_size) {
    preferences.putInt("ROIs", ROI_state.ROI_size);
    Serial.println("write ROI_size = " + String(ROI_state.ROI_size));
  }

  if(old.center[0] != ROI_state.center[0]) {
    preferences.putInt("c0", ROI_state.center[0]);
    Serial.println("write center[0] = " + String(ROI_state.center[0]));
  }

  if(old.center[1] != ROI_state.center[1]) {
    preferences.putInt("c1", ROI_state.center[1]);
    Serial.println("write center[1] = " + String(ROI_state.center[1]));
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

  client.begin(true);
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

void Msg_ROI_size( const String &message ){
  ROI_state.ROI_size = message.toInt( );
  write_eeprom();
}

void Msg_Zone1( const String &message ){
  ROI_state.center[0] = message.toInt();
  write_eeprom();
}

void Msg_Zone2( const String &message ){
  ROI_state.center[1] = message.toInt();
  write_eeprom();
}

void onConnection(){
  client.Subscribe("settings/ROI_size", Msg_ROI_size); 
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

  if(mode == 0){
    client.Publish("settings/ROI_size", String(ROI_state.ROI_size));
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

  uint32_t mil = millis( );

  distanceSensor.setROI(ROI_state.ROI_size, ROI_state.ROI_size, ROI_state.center[Zone]);  // first value: height of the zone, second value: width of the zone
  delay(delay_between_measurements);
  distanceSensor.setTimingBudgetInMs(time_budget_in_ms);
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();

  switch(Zone){
    case 0:
      state.zone1.state = (distance <= 1500);
      if(state.zone1.state) {
        state.zone1.t_up = mil;
      } else {
        state.zone1.t_down = mil;
      }
    break;

    case 1:
      state.zone2.state = (distance <= 1500);
      if(state.zone2.state) {
        state.zone2.t_up = mil;
      } else {
        state.zone2.t_down = mil;
      }
    break;
  }

  Zone++;
  if(Zone > 1) Zone = 0;

	if(cur_state.zone1.state != state.zone1.state || cur_state.zone2.state != state.zone2.state){
    if(state.zone1.state && state.zone1.t_up > 0 && !state.zone2.state && state.zone2.t_up == 0){
      first = 1;
    } else if(state.zone2.state && state.zone2.t_up > 0 && !state.zone1.state && state.zone1.t_up == 0){
      first = 2;
    }

    if(!state.zone1.state && !state.zone2.state) {
      if(	first == 1 &&
          state.zone1.t_down > 0 &&
          state.zone2.t_down >= state.zone2.t_up && state.zone2.t_down >= state.zone1.t_down && state.zone2.t_down >= state.zone1.t_up &&
          state.zone1.t_down >= state.zone1.t_up && state.zone1.t_down >= state.zone2.t_up &&
          state.zone2.t_up >= state.zone1.t_up ) {
        state.dir = 1;
        state.Counter++;
      } else if( first == 2 &&
                state.zone1.t_down > 0 &&
                state.zone1.t_down >= state.zone1.t_up && state.zone1.t_down >= state.zone2.t_down && state.zone1.t_down >= state.zone2.t_up &&
                state.zone2.t_down >= state.zone2.t_up && state.zone2.t_down >= state.zone1.t_up &&
                state.zone1.t_up >= state.zone2.t_up) {
        state.dir = 2;
        state.Counter--;
        if(state.Counter < 0) state.Counter = 0;
      }
      state.zone1 = Clear_time(state.zone1);
      state.zone2 = Clear_time(state.zone2);
      first = 0;
    }
  }

  if(!state.zone1.state && !state.zone2.state) {
    state.zone1 = Clear_time(state.zone1);
    state.zone2 = Clear_time(state.zone2);
  }

	if(state.zone1.state || state.zone2.state){
		state.any_HIGH = true;
	}
	else {
		state.any_HIGH = false;
	}  

  report(state, (client.flag_start ? 0 : 1)); //отправляем все
}