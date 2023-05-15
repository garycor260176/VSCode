#include <Adafruit_NeoPixel.h>
#include <mqtt_ini.h>

#define def_path "home/lamp/flower"

#define FLAG_SEND_STATE     1

#define PIN_LAMP            D6
#define PIN_BTN             D7

uint32_t timeSinceLastBtn = 0; //запоминаем время последней публикации
boolean flagSinceLastBtn = false;

mqtt_ini client( 
  "ESP8266_FLOWER",     // Client name that uniquely identify your device
   def_path);

struct stru_btn{
  int pin;
  int state;
};

struct s_state{
  stru_btn lamp;
  stru_btn btn;
};

s_state cur_state; //предыдущее отправленное состояние 

void report( s_state, int, unsigned long);

void setup() {

  Serial.begin(115200);
  Serial.println("Start...");

  cur_state.lamp.pin = PIN_LAMP;
  cur_state.btn.pin = PIN_BTN;
  pinMode(cur_state.lamp.pin, OUTPUT);  digitalWrite(cur_state.lamp.pin, LOW);
  pinMode(cur_state.btn.pin, INPUT);  

  client.begin();
}

void onMsgCommand( const String &message ){
  if (message == "command=on") {
    digitalWrite(cur_state.lamp.pin, HIGH);
  }
  if (message == "command=off") {
    digitalWrite(cur_state.lamp.pin, LOW);
  }
}

void report( s_state sState1, int mode, unsigned long flagSend ){
  s_state sState = sState1;
  
  unsigned long flagS = flagSend;
  if(flagS == 0) {
    flagS = 
            FLAG_SEND_STATE;
  }

  if( (flagS & FLAG_SEND_STATE) == FLAG_SEND_STATE){
    if(mode == 0 || ( mode == 1 && sState.lamp.state != cur_state.lamp.state ) ){
      client.Publish("state", String(sState.lamp.state));
    }
  }

  cur_state = sState;

  client.flag_start = false;
}

void SetLampState( s_state sState ){
  digitalWrite(sState.lamp.pin, sState.lamp.state);
}

s_state Read_state( ){
  s_state ret = cur_state;
  ret.lamp.state = digitalRead( ret.lamp.pin );
  ret.btn.state = digitalRead( ret.btn.pin );

  return ret;
};

void OnCheckState(){
  s_state state = Read_state( );

  if(state.btn.state == HIGH){
    flagSinceLastBtn = true;
  } else {
    if(flagSinceLastBtn){
      if( state.lamp.state == HIGH ) state.lamp.state = LOW;
      else                           state.lamp.state = HIGH;
      SetLampState(state);
    }
    flagSinceLastBtn = false;
  }
  
  if(client.flag_start){ //первый запуск
    report(state, 0, 0); //отправляем все
  } else {
    report(state, 1, 0); //отправляем только то, что изменилось
  }
}

void onConnection()
{
}

void OnLoad(){}

void loop() {
  client.loop();
}