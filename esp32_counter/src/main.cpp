#include <mqtt_ini.h>
#include <Wire.h>
#include "VL53L1X_ULD.h"

#define XSHUT_1		33
#define INT_1			32
#define INT_2			23
#define SENSOR_2_I2C_ADDRESS 0x55

#define MAX_DIST 1200

VL53L1X_ULD sensor_1;
VL53L1X_ULD sensor_2;

int status;

struct s_zone{
	uint16_t distance;
  boolean in;

  uint64_t up_time;
  uint64_t down_time;
};

struct s_dev{
	s_zone z1;
};

struct s_state{
  s_dev dev_1;
  s_dev dev_2;

  int dir;
  int first;
  boolean on;
};

s_state last_state;
boolean f = true;

#define def_path "home/cnt01"
mqtt_ini client( 
  "ESP32_CNT01",     // Client name that uniquely identify your device
   def_path,
   false);

void setup() {
  //uint8_t x = 15;
  //uint8_t y = 4;

  Serial.begin(115200); // Start the serial port
  Wire.begin(); // Initialize the I2C controller
  Wire.setClock(400000);

  Serial.println(F(""));  Serial.println(F("Start..."));

	pinMode(XSHUT_1, OUTPUT);	pinMode(INT_1, INPUT);
  delay(100);
  digitalWrite(XSHUT_1, LOW);
  delay(100);

  // Initialize sensor 2
  VL53L1_Error status = sensor_2.Begin(SENSOR_2_I2C_ADDRESS);
  if (status != VL53L1_ERROR_NONE) {
    // If the sensor could not be initialized print out the error code. -7 is timeout
    Serial.println("Could not initialize sensor 2, error code: " + String(status));
    while (1) {}
  }
  Serial.println(F("Sensor_2 initialized"));

  // Set the I2C address of sensor 2 to a different address as the default. 
  sensor_2.SetI2CAddress(SENSOR_2_I2C_ADDRESS);
  sensor_2.SetInterruptPolarity(ActiveLOW);
  sensor_2.SetDistanceMode(Short);
  sensor_2.SetInterMeasurementInMs(10);
  sensor_2.SetTimingBudgetInMs(10);
  //sensor_2.SetROI(x, y);  
  //sensor_2.SetROICenter(186);

  digitalWrite(XSHUT_1, HIGH);
  // Initialize sensor 1
  status = sensor_1.Begin();
  if (status != VL53L1_ERROR_NONE) {
    // If the sensor could not be initialized print out the error code. -7 is timeout
    Serial.println("Could not initialize sensor 1, error code: " + String(status));
    while (1) {}
  }
  Serial.println(F("Sensor_1 initialized"));
  sensor_1.SetInterruptPolarity(ActiveLOW);
  sensor_1.SetDistanceMode(Short);
  sensor_1.SetInterMeasurementInMs(10);
  sensor_1.SetTimingBudgetInMs(10);
  //sensor_1.SetROI(x, y);  
  //sensor_1.SetROICenter(186);

  sensor_1.StartRanging();
  sensor_2.StartRanging();

  client.begin(true);
}

uint16_t read_distance_1(s_zone z){
  uint16_t  distance = 0;
  while (digitalRead(INT_1));	// slightly faster
  status += sensor_1.GetDistanceInMm(&distance);
  return distance;  
}

uint16_t read_distance_2(s_zone z){
  uint16_t  distance = 0;
  while (digitalRead(INT_2));	// slightly faster
  status += sensor_2.GetDistanceInMm(&distance);
  return distance;
}

s_state Read_state( ){
  s_state ret = last_state;
  uint64_t mil = millis( );

  if(f) {
    //первый датчик
    ret.dev_1.z1.distance = read_distance_1(ret.dev_1.z1);  ret.dev_1.z1.in = (ret.dev_1.z1.distance < MAX_DIST ? true : false) ;  sensor_1.ClearInterrupt();
    // из 0 -> 1
    if(!last_state.dev_1.z1.in && ret.dev_1.z1.in) ret.dev_1.z1.up_time = mil;
    // из 1 -> 0
    if(last_state.dev_1.z1.in && !ret.dev_1.z1.in) ret.dev_1.z1.down_time = mil;
  } else {
    //второй датчик
    ret.dev_2.z1.distance = read_distance_2(ret.dev_2.z1);  ret.dev_2.z1.in = (ret.dev_2.z1.distance < MAX_DIST ? true : false) ;  sensor_2.ClearInterrupt();
    // из 0 -> 1
    if(!last_state.dev_2.z1.in && ret.dev_2.z1.in) ret.dev_2.z1.up_time = mil;
    // из 1 -> 0
    if(last_state.dev_2.z1.in && !ret.dev_2.z1.in) ret.dev_2.z1.down_time = mil;
  }
  f = !f;

  if(ret.dev_1.z1.in || ret.dev_2.z1.in) ret.on = true;

  if(ret.first == 0) {
    if(ret.dev_1.z1.in) ret.first = 1;
    else if(ret.dev_2.z1.in) ret.first = 2;
  }

  ret.dir = 0;

  if(!ret.dev_1.z1.in && !ret.dev_2.z1.in && ret.first > 0) {
    switch(ret.first){
      case 1:
        if( ret.dev_2.z1.down_time > ret.dev_1.z1.down_time &&
            //ret.dev_1.z1.down_time > ret.dev_1.z1.up_time && 
            ret.dev_2.z1.up_time > ret.dev_1.z1.up_time )
        {
          ret.dir = 1;
        }         
        break;

      case 2:
        if( ret.dev_1.z1.down_time > ret.dev_2.z1.down_time &&
            //ret.dev_2.z1.down_time > ret.dev_2.z1.up_time && 
            ret.dev_1.z1.up_time > ret.dev_2.z1.up_time )
        {
          ret.dir = 2;
        }
        break;
    }
  }

  return ret;
};

void onMsgCommand( const String &message ){

}

void OnCheckState(){
  s_state new_state = Read_state( );
  s_state ini_state;

  if(new_state.on != last_state.on) {
    client.Publish("on", (new_state.on ? "1" : "0"));
  } else

  if(last_state.dir != new_state.dir && new_state.dir > 0) {
    client.Publish("dir", String(new_state.dir));
  }

  if(new_state.dir > 0 || ( !new_state.dev_1.z1.in && !new_state.dev_2.z1.in )) 
  { 
    new_state = ini_state;
    new_state.first = 0;
  }

  last_state = new_state;

  client.flag_start = false;
}

void onConnection(){
  
}

void OnLoad(){

}

void loop() {
  client.loop();
}