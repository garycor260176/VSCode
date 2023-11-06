#include <arduino.h>
#include <mqtt_ini.h>
#include <cl_ld2410c.h>
#include <Preferences.h>

ld2410 radar;

#define def_path "LD2410C_05"

Preferences preferences; //хранение текущего состояния

mqtt_ini client( 
  "ESP32_LD2410C_05",     // Client name that uniquely identify your device
   def_path);

cl_ld2410c ld2410c(&radar, &preferences, &client, "", &Serial1);

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