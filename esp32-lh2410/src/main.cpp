#include <arduino.h>
#include <mqtt_ini.h>
#include <cl_ld2410c.h>

ld2410 radar;

#define def_path "LD2410C_01"

Preferences preferences; //хранение текущего состояния

mqtt_ini client( 
  "ESP32_LD2410C_01",     // Client name that uniquely identify your device
   def_path);

cl_ld2410c ld2410c(&radar, &preferences, &client, "", &Serial1, "int_ld", INTERVAL_LD, 256000, SERIAL_8N1, 18, 19, 23);

void setup(void)
{
  Serial.begin(115200); //Feedback over Serial Monitor
  Serial.println("");  Serial.println("Start!");
  
  preferences.begin("settings", false);
  ld2410c.begin();
  client.begin();
}

void onMsgCommand( const String &message ){}
void OnLoad(){}
void OnCheckState(){}

void onConnection(){
  ld2410c.subscribe();
}

void loop()
{
  client.loop();
  ld2410c.loop();
  client.flag_start = false;
}