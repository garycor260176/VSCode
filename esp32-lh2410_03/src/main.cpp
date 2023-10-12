#include <arduino.h>
#include <ld2410.h>
#include <mqtt_ini.h>
#include <Preferences.h>

ld2410 radar;
uint32_t lastReading = 0;
bool radarConnected = false;

#define def_path "LD2410C_03"
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 18
#define RADAR_TX_PIN 19
#define PIN_IO       23
#define PIN_MS       5

Preferences preferences; //хранение текущего состояния

mqtt_ini client( 
  "ESP32_LD2410C_03",     // Client name that uniquely identify your device
   def_path);

struct s_settings{
  int pin_MS_interval;
  int pin_LD_interval;
};

uint32_t ms_mil = 0;

struct s_state{
  int pin_IO_value;
  int Stat_target;
  int Stat_power;
  int Moving_target;
  int Moving_power;
  int presenceDetected;
  int stationaryTargetDetected;
  int movingTargetDetected;

  int pin_MS_value;

  s_settings ini;
};
s_state cur_state;

void report_ini(s_settings ini, int mode);

s_settings read_eeprom(){
  s_settings ret;

  ret.pin_MS_interval = preferences.getInt("int_ms", 5);
  if(ret.pin_MS_interval < 500){
    ret.pin_MS_interval = 500;
  }
  Serial.println("read pin_MS_interval = " + String(ret.pin_MS_interval));

  ret.pin_LD_interval = preferences.getInt("int_ld", 500);
  if(ret.pin_LD_interval < 500){
    ret.pin_LD_interval = 500;
  }
  Serial.println("read pin_LD_interval = " + String(ret.pin_LD_interval));

  return ret;
}

s_settings write_eeprom(s_settings ini){
  s_settings ret = ini;
  if(ret.pin_MS_interval < 500){
    ret.pin_MS_interval = 500;
  }
  if(ret.pin_LD_interval < 500){
    ret.pin_LD_interval = 500;
  }

  if(cur_state.ini.pin_MS_interval != ret.pin_MS_interval)    {
    preferences.putInt("int_ms", ret.pin_MS_interval);
    Serial.println("save pin_MS_interval = " + String(ret.pin_MS_interval));
  }
  if(cur_state.ini.pin_LD_interval != ret.pin_LD_interval)    {
    preferences.putInt("int_ld", ret.pin_LD_interval);
    Serial.println("save pin_LD_interval = " + String(ret.pin_LD_interval));
  }
  return ret;
}

void setup(void)
{
  MONITOR_SERIAL.begin(115200); //Feedback over Serial Monitor
  
  pinMode(PIN_MS, INPUT);
  pinMode(PIN_IO, INPUT);

  preferences.begin("settings", false);

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

  cur_state.ini = read_eeprom();

  client.begin();
}

void onMsgCommand( const String &message ){}
void OnLoad(){}
void OnCheckState(){}

void onMS_interval(const String &message) {
  s_settings ini = cur_state.ini;
  ini.pin_MS_interval = message.toInt();
  ini = write_eeprom(ini);
  report_ini(ini, 1);
  cur_state.ini = ini;
}

void onLD_interval(const String &message) {
  s_settings ini = cur_state.ini;
  ini.pin_LD_interval = message.toInt();
  ini = write_eeprom(ini);
  report_ini(ini, 1);
  cur_state.ini = ini;
}

void onConnection(){
  client.Subscribe("settings/MS_interval", onMS_interval);
  client.Subscribe("settings/LD_interval", onLD_interval);
}

void report_ini(s_settings ini, int mode){
  if (mode == 0 || ( mode == 1 && cur_state.ini.pin_MS_interval != ini.pin_MS_interval) )  {
    client.Publish("settings/MS_interval", String(ini.pin_MS_interval));
  }
  if (mode == 0 || ( mode == 1 && cur_state.ini.pin_LD_interval != ini.pin_LD_interval) )  {
    client.Publish("settings/LD_interval", String(ini.pin_LD_interval));
  }
}

void report(s_state state, int mode){
  report_ini(state.ini, mode);

  if (mode == 0 || ( mode == 1 && cur_state.pin_MS_value != state.pin_MS_value ) )  {
    client.Publish("states/pin_MS", String(state.pin_MS_value));
  }
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

  int pin_ms = digitalRead(PIN_MS);
  if(pin_ms == HIGH) {
    ms_mil = millis();
    state.pin_MS_value = HIGH;
  } else {
    if(state.pin_MS_value == HIGH) {
      if(millis() - ms_mil > state.ini.pin_MS_interval) {
        state.pin_MS_value = LOW;
      }
    }
  }

  radar.read();
  if(radar.isConnected() && millis() - lastReading > state.ini.pin_LD_interval)  //Report every 1000ms
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
      }
      state.movingTargetDetected = radar.movingTargetDetected() ? 1 : 0;
      if(state.movingTargetDetected)
      {
        state.Moving_target = radar.movingTargetDistance();
        state.Moving_power = radar.movingTargetEnergy();
      }
    }
    else
    {
//      Serial.println(F("No target"));
    }
  }

  state.pin_IO_value = digitalRead(PIN_IO);
  report(state, (client.flag_start ? 0 : 1));
}