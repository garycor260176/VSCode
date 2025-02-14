#include <mqtt_ini.h>

#define def_path "LD2410/01"

#define LD2410_PIN D4

mqtt_ini client( 
  "LD2010_01",
   def_path,
   true,
  180U,
  "192.168.1.59");

struct s_ld{
  int pin;
  int value;
  
};

struct s_state{
  s_ld ld2410;
};

s_state cur_state;

void report( s_state, int);
void onMsgCommand(const String &message);
s_state Read_state();
void OnCheckState();
void onConnection();
void OnLoad();

void setup(void)
{
  Serial.begin(115200);
  Serial.println("\nStart...");
  
  cur_state.ld2410.pin = LD2410_PIN;

  pinMode(cur_state.ld2410.pin, INPUT);  

  client.begin(true);
}

void onMsgCommand( const String &message ){

}

void report(s_state state, int mode){
  if(mode == 0 || ( mode == 1 && state.ld2410.value != cur_state.ld2410.value) ){
    client.Publish("state", String(state.ld2410.value));
  }
  cur_state = state;
  client.flag_start = false;
}

s_state Read_state(){
  s_state ret = cur_state;
  ret.ld2410.value = digitalRead(ret.ld2410.pin);
  return ret;
};

void onConnection(){
}

void OnLoad(){
  client.flag_start = true;
}

void OnCheckState(){
  s_state state = Read_state( );

  if(client.flag_start){
    report(state, 0);
  } else {
    report(state, 1);
  }
}

void loop()
{
  client.loop();
}