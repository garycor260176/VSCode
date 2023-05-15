#include <mqtt_ini.h>

#define def_path "outside/greenhouse/gbed_1_and_2"

#define FLAG_SEND_STATE         1
#define PIN_SOIL_1  32
#define PIN_SOIL_2  33
#define PIN_RELAY_1 18
#define PIN_RELAY_2 19

struct relay{
  int pin;
  int state;
};

struct soil{
  int pin;
  int value;

  relay rel;
};

struct s_state{
  soil soil_1;
  soil soil_2;

  uint32_t interval_read_soil;
  uint32_t last_read;// Timer to manage any contact bounce in interrupt routine 
};



s_state cur_state;

mqtt_ini client( 
  "ESP32_GH_01",     // Client name that uniquely identify your device
   def_path);

void ini_soil(soil s){
  pinMode(s.pin, INPUT);
  pinMode(s.rel.pin, OUTPUT); digitalWrite(s.rel.pin, LOW);
}

void setup() { 
  Serial.begin(115200);

  cur_state.soil_1.pin = PIN_SOIL_1; cur_state.soil_1.rel.pin = PIN_RELAY_1; ini_soil( cur_state.soil_1 );
  cur_state.soil_2.pin = PIN_SOIL_2; cur_state.soil_2.rel.pin = PIN_RELAY_2; ini_soil( cur_state.soil_2 );
  cur_state.interval_read_soil = 5000;

  client.begin();
}


void onMsgCommand( const String &message ){
  if(message == "command=1_rel:on"){
    digitalWrite(cur_state.soil_1.rel.pin, HIGH);
  }
  if(message == "command=1_rel:off"){
    digitalWrite(cur_state.soil_1.rel.pin, LOW);
  }
  if(message == "command=2_rel:on"){
    digitalWrite(cur_state.soil_2.rel.pin, HIGH);
  }
  if(message == "command=2_rel:off"){
    digitalWrite(cur_state.soil_2.rel.pin, LOW);
  }
}

void onConnection()
{
}

s_state Read_state( ){
  s_state ret = cur_state;

  ret.soil_1.rel.state = digitalRead(ret.soil_1.rel.pin);
  ret.soil_2.rel.state = digitalRead(ret.soil_2.rel.pin);

  uint32_t mil = millis();

  if(mil - ret.last_read > ret.interval_read_soil){
  
    ret.soil_1.value = analogRead(ret.soil_1.pin);
    //ret.soil_2.value = analogRead(ret.soil_2.pin);

    ret.last_read = mil;
  }

  return ret;
}

void report( s_state sState, int mode, unsigned long flagSend ){
  unsigned long flagS = flagSend;
  if(flagS == 0) {
    flagS = 
            FLAG_SEND_STATE;
  }
  
  if( (flagS & FLAG_SEND_STATE) == FLAG_SEND_STATE){
    if(mode == 0 || ( mode == 1 && sState.soil_1.rel.state != cur_state.soil_1.rel.state ) ){
      client.Publish("soil_1/relay/state", String(sState.soil_1.rel.state));
    }
    if(mode == 0 || ( mode == 1 && sState.soil_1.value != cur_state.soil_1.value ) ){
      client.Publish("soil_1/soil_1.soilMoistureValue", String(sState.soil_1.value));
    }

    if(mode == 0 || ( mode == 1 && sState.soil_2.value != cur_state.soil_2.value ) ){
      client.Publish("soil_2/soil_2.soilMoistureValue", String(sState.soil_2.value));
    }
    if(mode == 0 || ( mode == 1 && sState.soil_2.rel.state != cur_state.soil_2.rel.state ) ){
      client.Publish("soil_2/relay/state", String(sState.soil_2.rel.state));
    }
  }

  cur_state = sState;

  client.flag_start = false;
}

void OnCheckState(){
  s_state state = Read_state( );

  if(client.flag_start){ //первый запуск
    report(state, 0, 0); //отправляем все
  } else {
    report(state, 1, 0); //отправляем только то, что изменилось
  }
}

void OnLoad(){}

void loop() {
  client.loop();
}