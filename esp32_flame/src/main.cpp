#include <mqtt_ini.h>
#include <DallasTemperature.h>

#define def_path "home/flame"

#define PIN_DHT22    19
#define PIN_FLAME_A  32
#define PIN_FLAME_D  33
#define PIN_SMOKE_A  35
#define PIN_SMOKE_D  25

uint32_t LAST_CHECK = 0;

mqtt_ini client( 
  "ESP32_FLAME",     // Client name that uniquely identify your device
   def_path);

struct s_pin{
  int pin;
  int value;
};

struct s_ds18b20 {
  float f_t;

  uint32_t intervalRead; //в минутах
  uint32_t last_check; //в минутах

  String s_t;
};

struct s_AD{
  s_pin A;
  s_pin D;
  uint32_t intervalRead;
  uint32_t last_check; //в минутах
};

struct s_state{
  s_AD flame;
  s_AD smoke;
  s_ds18b20 dht22;
};

s_state cur_state; //предыдущее отправленное состояние 

void report( s_state, int);

OneWire oneWire(PIN_DHT22);
DallasTemperature DS18B20(&oneWire);

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  cur_state.flame.A.pin = PIN_FLAME_A; pinMode(cur_state.flame.A.pin, INPUT); 
  cur_state.flame.D.pin = PIN_FLAME_D; pinMode(cur_state.flame.D.pin, INPUT); 
  cur_state.flame.intervalRead = 1000;

  cur_state.smoke.A.pin = PIN_SMOKE_A; pinMode(cur_state.smoke.A.pin, INPUT); 
  cur_state.smoke.D.pin = PIN_SMOKE_D; pinMode(cur_state.smoke.D.pin, INPUT); 
  cur_state.smoke.intervalRead = 1000;

  cur_state.dht22.intervalRead = 10000;

  DS18B20.begin();

  client.begin(true);
}

void onMsgCommand( const String &message ){

}

void report( s_state sState, int mode ){
  if (mode == 0 || ( mode == 1 && sState.flame.A.value != cur_state.flame.A.value ) ) {
    client.Publish("flame/value_A", String(sState.flame.A.value));
  }
  if (mode == 0 || ( mode == 1 && sState.flame.D.value != cur_state.flame.D.value ) ) {
    client.Publish("flame/value_D", String(sState.flame.D.value));
  }

  if (mode == 0 || ( mode == 1 && sState.smoke.A.value != cur_state.smoke.A.value ) ) {
    client.Publish("smoke/value_A", String(sState.smoke.A.value));
  }
  if (mode == 0 || ( mode == 1 && sState.smoke.D.value != cur_state.smoke.D.value ) ) {
    client.Publish("smoke/value_D", String(sState.smoke.D.value));
  }
  if (mode == 0 || ( mode == 1 && sState.dht22.s_t != cur_state.dht22.s_t ) ) {
    client.Publish("dht22/t", String(sState.dht22.s_t));
  }

  cur_state = sState;
  client.flag_start = false;
}

s_ds18b20 Read_DHT22() {
  s_ds18b20 ret = cur_state.dht22;

  if(millis() - ret.last_check > ret.intervalRead){
    DS18B20.requestTemperatures();
    ret.f_t = DS18B20.getTempCByIndex(0);
    if (isnan(ret.f_t) || ret.f_t == -127 ) { //при сбое вернем предыдущее значение
      ret.f_t = cur_state.dht22.f_t;
      ret.s_t = cur_state.dht22.s_t;
    } else {
      ret.s_t = String(ret.f_t);
    }
    ret.last_check = millis( );
  }

  return ret;
}

s_state Read_state( ){
  s_state ret = cur_state;

  uint32_t mil = millis();

  if (mil - ret.flame.last_check > ret.flame.intervalRead || client.flag_start) {
    ret.flame.A.value = 4095 - analogRead(ret.flame.A.pin);
    if(digitalRead(ret.flame.D.pin) == HIGH){
      ret.flame.D.value = LOW;
    }else{
      ret.flame.D.value = HIGH;
    }
    ret.flame.last_check = mil;
  }

  if (mil - ret.smoke.last_check > ret.smoke.intervalRead || client.flag_start) {
    ret.smoke.A.value = analogRead(ret.smoke.A.pin);
    ret.smoke.D.value = digitalRead(ret.smoke.D.pin);
    ret.smoke.last_check = mil;
  }

  ret.dht22 = Read_DHT22();

  return ret;
};

void OnCheckState(){
  s_state state = Read_state( );

  if(client.flag_start){ //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void onConnection(){
}

void OnLoad(){
}

void loop() {
  client.loop();
}
