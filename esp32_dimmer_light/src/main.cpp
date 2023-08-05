#include <mqtt_ini.h>

#include <RBDdimmerESP32.h>
#define outputPin  32 
#define zerocross  33

#define MAX_VALUE 80

dimmerLampESP32 dimmer(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards

#define def_path "DIMMER"
mqtt_ini client( 
  "ESP32_DIMMER",
   def_path);

struct s_dimmer{
  int old_state;
  int new_state;
};

struct s_state{
  s_dimmer dim1;
  s_dimmer dim2;
};

s_state cur_state;

void setup() {
  Serial.begin(115200);                                         
  Serial.println("");  Serial.println("Start!");

  dimmer.begin(NORMAL_MODE, ON); //Initialize the dimmer
  dimmer.setPower(0);

  client.begin(true);
}

void onMsgCommand( const String &message ){}
void OnLoad(){}
void OnCheckState(){}

void Msg_PWR_1( const String &message ){
  Serial.println(message);
  cur_state.dim1.new_state = message.toInt();
  if(cur_state.dim1.new_state > MAX_VALUE) {
    cur_state.dim1.new_state = MAX_VALUE;
  }
  if(cur_state.dim1.new_state < 0) {
    cur_state.dim1.new_state = 0;
  }
}

void onConnection(){
  client.Subscribe("pwr1", Msg_PWR_1); 
}

void report(int mode ){
  if (mode == 0 || ( mode == 1 && cur_state.dim1.new_state != cur_state.dim1.old_state ) )  {
    client.Publish("pwr1", String(cur_state.dim1.new_state));
  }

  client.flag_start = false;
}

void loop() {
  client.loop();

  if(cur_state.dim1.old_state < cur_state.dim1.new_state) {
    for(int i = cur_state.dim1.old_state; i <= cur_state.dim1.new_state; i++) {
      dimmer.setPower(i);
      delay(50);
    }    
  } else if(cur_state.dim1.old_state > cur_state.dim1.new_state) {
    for(int i = cur_state.dim1.old_state; i >= cur_state.dim1.new_state; i--) {
      dimmer.setPower(i);
      delay(50);
    }    
  }

  cur_state.dim1.old_state = cur_state.dim1.new_state;

  report((client.flag_start ? 0 : 1));
}