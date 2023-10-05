#include <arduino.h>
#include <ld2410.h>
#include <mqtt_ini.h>

ld2410 radar;
uint32_t lastReading = 0;
bool radarConnected = false;

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 18
#define RADAR_TX_PIN 19
#define PIN_IO       23

#define def_path "LD2410C_02"

mqtt_ini client( 
  "ESP32_LD2410C_02",     // Client name that uniquely identify your device
   def_path);

struct s_state{
  int pin_IO_value;
  int Stat_target;
  int Stat_power;
  int Moving_target;
  int Moving_power;
  int presenceDetected;
  int stationaryTargetDetected;
  int movingTargetDetected;
};
s_state cur_state;

void setup(void)
{
  MONITOR_SERIAL.begin(115200); //Feedback over Serial Monitor
  
  //radar.debug(MONITOR_SERIAL); //Uncomment to show debug information from the library on the Serial Monitor. By default this does not show sensor reads as they are very frequent.
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN); //UART for monitoring the radar
  delay(500);
  MONITOR_SERIAL.print(F("\nConnect LD2410 radar TX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_RX_PIN);
  MONITOR_SERIAL.print(F("Connect LD2410 radar RX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_TX_PIN);
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
  if(radar.begin(RADAR_SERIAL))
  {
    MONITOR_SERIAL.println(F("OK"));
    MONITOR_SERIAL.print(F("LD2410 firmware version: "));
    MONITOR_SERIAL.print(radar.firmware_major_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.print(radar.firmware_minor_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  }
  else
  {
    MONITOR_SERIAL.println(F("not connected"));
  }

  client.begin(true);
}

void onMsgCommand( const String &message ){}
void OnLoad(){}
void OnCheckState(){}
void onConnection(){}

void report(s_state state, int mode){
  if (mode == 0 || ( mode == 1 && cur_state.pin_IO_value != state.pin_IO_value ) )  {
    client.Publish("states/pin_IO", String(state.pin_IO_value));
  }
  if (mode == 0 || ( mode == 1 && cur_state.Moving_power != state.Moving_power ) )  {
    client.Publish("states/Moving/Moving_power", String(state.Moving_power));
  }
  if (mode == 0 || ( mode == 1 && cur_state.Moving_target != state.Moving_target ) )  {
    client.Publish("states/Moving/Moving_target", String(state.Moving_target));
  }
  if (mode == 0 || ( mode == 1 && cur_state.movingTargetDetected != state.movingTargetDetected ) )  {
    client.Publish("states/Moving/movingTargetDetected", String(state.movingTargetDetected));
  }
  if (mode == 0 || ( mode == 1 && cur_state.presenceDetected != state.presenceDetected ) )  {
    client.Publish("states/presenceDetected", String(state.presenceDetected));
  }
  if (mode == 0 || ( mode == 1 && cur_state.Stat_power != state.Stat_power ) )  {
    client.Publish("states/stationary/Stat_power", String(state.Stat_power));
  }
  if (mode == 0 || ( mode == 1 && cur_state.Stat_target != state.Stat_target ) )  {
    client.Publish("states/stationary/Stat_target", String(state.Stat_target));
  }
  if (mode == 0 || ( mode == 1 && cur_state.stationaryTargetDetected != state.stationaryTargetDetected ) )  {
    client.Publish("states/stationary/stationaryTargetDetected", String(state.stationaryTargetDetected));
  }
  client.flag_start = false;
  cur_state = state;
}

void loop()
{
  client.loop();

  s_state state = cur_state;

  radar.read();
  if(radar.isConnected() && millis() - lastReading > 500)  //Report every 1000ms
  {
    lastReading = millis();
    state.presenceDetected = radar.presenceDetected() ? 1 : 0;
    if(state.presenceDetected)
    {
      state.stationaryTargetDetected = radar.stationaryTargetDetected() ? 1 : 0;
      if(state.stationaryTargetDetected)
      {
        state.Stat_target = radar.stationaryTargetDistance();
        state.Stat_power = radar.stationaryTargetEnergy();
/*        Serial.print(F("Stationary target: "));
        Serial.print(state.Stat_target);
        Serial.print(F("cm energy:"));
        Serial.print(state.Stat_power);
        Serial.print(' '); */
      }
      state.movingTargetDetected = radar.movingTargetDetected() ? 1 : 0;
      if(state.movingTargetDetected)
      {
        state.Moving_target = radar.movingTargetDistance();
        state.Moving_power = radar.movingTargetEnergy();

/*        Serial.print(F("Moving target: "));
        Serial.print(state.Moving_target);
        Serial.print(F("cm energy:"));
        Serial.print(state.Moving_power); */
      }
//      Serial.println();
    }
    else
    {
//      Serial.println(F("No target"));
    }
  }

  state.pin_IO_value = digitalRead(PIN_IO);
  report(state, (client.flag_start ? 0 : 1));
}