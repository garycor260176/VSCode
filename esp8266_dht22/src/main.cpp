#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <mqtt_ini.h>

#define def_path "CHICKEN/DHT22"
#define DHTPIN   D5

uint32_t LastRead_DHT22 = 0; //запоминаем время последней публикации
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

uint32_t disconnect_start = 0; 
boolean f_disconnect_start = false;
#define disconnect_time_reload 60000 //если в течении 60ти секунд нет связи

mqtt_ini client( 
  "ESP32_CHICKEN_DHT22",     // Client name that uniquely identify your device
   def_path);

struct s_dht22{
  float f_h;
  float f_t;

  String s_h;
  String s_t;
};

struct s_state{
  uint32_t interval_DHT22;
  s_dht22 dht22;
};

s_state cur_state;

void report( s_state, int);
boolean PubInterval(String topic, uint32_t val);

void setup() {
  Serial.begin(115200);

  cur_state.interval_DHT22 = 5000;
  dht.begin();

  client.begin();
  delay(200);
}

void onMsgCommand( const String &message ){}

void onConnection()
{
}

s_dht22 Read_DHT22(){
  s_dht22 ret;
  
  delay(200);
  ret.f_t = dht.readTemperature();
  ret.f_h = dht.readHumidity();
  if (isnan(ret.f_h) || isnan(ret.f_t)) { //при сбое вернем предыдущее значение
    ret.f_t = cur_state.dht22.f_t;
    ret.f_h = cur_state.dht22.f_h;
    ret.s_t = cur_state.dht22.s_t;
    ret.s_h = cur_state.dht22.s_h;
  } else {
    ret.s_t = String(ret.f_t);
    ret.s_h = String(ret.f_h);
  }
  return ret;
}

s_state Read_state( ){
  s_state ret = cur_state;

  if(client.flag_start || (millis( ) - LastRead_DHT22 > cur_state.interval_DHT22 && cur_state.interval_DHT22 >= 0 ) ) {
    ret.dht22 = Read_DHT22( );
    LastRead_DHT22 = millis();
  }
  return ret;
};

boolean PubInterval(String topic, uint32_t val){
  int intval = val / 1000;
  return client.Publish(topic, String(intval));
}

void report( s_state sState, int mode){
  if(mode == 0 || ( mode == 1 && sState.interval_DHT22 != cur_state.interval_DHT22 ) ){
    PubInterval("interval", sState.interval_DHT22);
  }
  if(mode == 0 || ( mode == 1 && sState.dht22.s_t != cur_state.dht22.s_t ) ){
    client.Publish("t", sState.dht22.s_t);
  }
  if(mode == 0 || ( mode == 1 && sState.dht22.s_h != cur_state.dht22.s_h ) ){
    client.Publish("h", sState.dht22.s_h);
  }
  cur_state = sState;
  client.flag_start = false;
}

void OnCheckState(){
  s_state state = Read_state( );
  
  if(client.flag_start){ //первый запуск    
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void OnLoad(){}

void loop() {
  // перегрузиться если нет связи в течении disconnect_time_reload миллисекунд
  if(!client.client->isConnected( )){
    if(!f_disconnect_start) {
      f_disconnect_start = true;
      disconnect_start = millis( );
    } else {
      if(millis( ) - disconnect_start > disconnect_time_reload) {
        ESP.restart();
      }
    }
  } else {
    f_disconnect_start = false;
  }

  client.loop();
}