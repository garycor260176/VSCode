#include "SparkFun_VL53L1X.h"

#define SHUTDOWN_PIN 2    
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor(Wire);//, SHUTDOWN_PIN, INTERRUPT_PIN);

struct s_ROI{
  int ROI_size_w = 4;
  int ROI_size_h = 4;
  //int center[4] = {167, 231, 0, 0}; /* center of the two zones */  
  int center[4] = {151, 239, 0, 0}; /* center of the two zones */  
  int distance = 1500;
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
  s_Zone zone3;
  s_Zone zone4;
  int dir = 0;
  int Counter = 0;
	boolean any_HIGH;
};

s_state cur_state;

void setup(void)
{
  Wire.begin();
  Serial.begin(115200);

  Serial.println("VL53L1X Qwiic Test");
  if (distanceSensor.init() == false)
    Serial.println("Sensor online!");

  distanceSensor.setDistanceModeLong();
  distanceSensor.setTimingBudgetInMs(20);
  distanceSensor.setIntermeasurementPeriod(20);
  distanceSensor.startRanging();

  delay(1000);
}

s_Zone Clear_time(s_Zone sensor){
  s_Zone ret = sensor;
  ret.t_down  = ret.t_up = 0;
  return ret;
}

void loop(void)
{
  s_state state = cur_state;

  uint16_t distance;

  uint32_t mil = millis( );

  distanceSensor.setROI(ROI_state.ROI_size_h, ROI_state.ROI_size_w, ROI_state.center[Zone]);  // first value: height of the zone, second value: width of the zone
  //delay(delay_between_measurements);
  //distanceSensor.setTimingBudgetInMs(time_budget_in_ms);
  //distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  //distanceSensor.stopRanging();

//Serial.println("Z " + String(Zone) + " = " + String(distance));

  switch(Zone){
    case 0:
      state.zone1.state = (distance <= ROI_state.distance);
      if(state.zone1.state) {
        state.zone1.t_up = mil;
      } else {
        state.zone1.t_down = mil;
      }
    break;

    case 1:
      state.zone2.state = (distance <= ROI_state.distance);
      if(state.zone2.state) {
        state.zone2.t_up = mil;
      } else {
        state.zone2.t_down = mil;
      }
    break;

    case 2:
      state.zone3.state = (distance <= ROI_state.distance);
      if(state.zone3.state) {
        state.zone3.t_up = mil;
      } else {
        state.zone3.t_down = mil;
      }
    break;

    case 3:
      state.zone4.state = (distance <= ROI_state.distance);
      if(state.zone4.state) {
        state.zone4.t_up = mil;
      } else {
        state.zone4.t_down = mil;
      }
      break;
  }

  Zone++;
  if(Zone > 1) Zone = 0;
  //if(Zone > 3) Zone = 0;

  if(cur_state.zone1.state != state.zone1.state) Serial.println("Zone 1 : " + String(state.zone1.state));
  if(cur_state.zone2.state != state.zone2.state) Serial.println("Zone 2 : " + String(state.zone2.state));
  if(cur_state.zone3.state != state.zone3.state) Serial.println("Zone 3 : " + String(state.zone3.state));
  if(cur_state.zone4.state != state.zone4.state) Serial.println("Zone 4 : " + String(state.zone4.state));


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

  cur_state = state;
}