#include <mqtt_ini.h>
#include <DHT.h>

#define def_path "outside/gh/b01"

#define PIN_DHT22         32
#define PIN_PW            15 //включение БП 24В
// первый актуатор
#define PIN_A1_D1         18  //крутить в одну сторону
#define PIN_A1_D2         19  //крутить в другую сторону
#define PIN_A1_D1_END     33  
#define PIN_A1_D2_END     25  

mqtt_ini client( 
  "ESP32_GH_B01",     // Client name that uniquely identify your device
   def_path);

struct s_level {
  int pin;
  int state;
  boolean changed;
};

struct s_relay{
  String rel_p;
  s_level rel;
};

struct s_actuator{
  String name;

  s_relay dir_1;
  s_relay dir_1_end;
  s_relay dir_2;
  s_relay dir_2_end;
};

struct s_dht22 {
  float f_h;
  float f_t;

  uint32_t intervalRead; //в минутах

  uint32_t last_check; //в минутах

  String s_h; 
  String s_t; 

  bool h_changed;
  bool t_changed; 
};

struct s_state{
  s_relay rpw;
  s_dht22 dht22;

  s_actuator act_1;

  int dir; boolean dir_changed;
};

#define DHTTYPE DHT22
DHT dht(PIN_DHT22, DHTTYPE);

s_state cur_state; //предыдущее отправленное состояние 

void report( int);

void ini_relay(s_relay &ret, int pin_relay, String rel_p){
  ret.rel.pin = pin_relay; pinMode(ret.rel.pin, OUTPUT);
  ret.rel_p = rel_p;
}

void ini_acturator(s_actuator &ret, String _name, int  pin_d1, int pin_d1_end, int  pin_d2, int pin_d2_end){
  ret.name = _name;

  ini_relay(ret.dir_1, pin_d1, "dir_1");
  ini_relay(ret.dir_2, pin_d2, "dir_2");

  ret.dir_1_end.rel.pin = pin_d1_end; pinMode(ret.dir_1_end.rel.pin, INPUT); ret.dir_1_end.rel_p = "dir_1_end";
  ret.dir_2_end.rel.pin = pin_d2_end; pinMode(ret.dir_2_end.rel.pin, INPUT); ret.dir_2_end.rel_p = "dir_2_end";
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  ini_relay(cur_state.rpw, PIN_PW, "rpw");
  cur_state.dir = 0;

  ini_acturator( cur_state.act_1, "actuator_1", PIN_A1_D1, PIN_A1_D1_END, PIN_A1_D2, PIN_A1_D2_END );

  cur_state.dht22.intervalRead = 30000;
  dht.begin();

  client.begin();
}

void read_state_rel(s_relay &ret){
  ret.rel.changed = false;
  int val = digitalRead(ret.rel.pin);

  if(val != ret.rel.state){
    ret.rel.state = val;
    ret.rel.changed = true;
  }
}

void read_actuator_state( s_actuator &ret){
  read_state_rel( ret.dir_1 );
  read_state_rel( ret.dir_2 );

  read_state_rel( ret.dir_1_end );
  read_state_rel( ret.dir_2_end );
}

void Read_DHT22() {
  uint32_t mil = millis();
  String s_h = cur_state.dht22.s_h;
  String s_t = cur_state.dht22.s_t;

  cur_state.dht22.t_changed = false;
  cur_state.dht22.h_changed = false;

  if (mil - cur_state.dht22.last_check > cur_state.dht22.intervalRead || client.flag_start) {
    cur_state.dht22.f_t = dht.readTemperature();
    cur_state.dht22.f_h = dht.readHumidity();
    if (isnan(cur_state.dht22.f_h) || isnan(cur_state.dht22.f_t)) {

    } else {
      s_t = String(cur_state.dht22.f_t);
      s_h = String(cur_state.dht22.f_h);
    }

    if(s_t != cur_state.dht22.s_t) {
      cur_state.dht22.s_t = s_t;
      cur_state.dht22.t_changed = true;
    }
    if(s_h != cur_state.dht22.s_h) {
      cur_state.dht22.s_h = s_h;
      cur_state.dht22.h_changed = true;
    }

    cur_state.dht22.last_check = mil;
  }
}

void Read_state( ){
  Read_DHT22();

  read_state_rel(cur_state.rpw);

  read_actuator_state( cur_state.act_1 );
}

void PW_ON_OFF( s_relay &ret, int state){
  if(digitalRead(ret.rel.pin) != state){
    digitalWrite( ret.rel.pin , state);
  }
  if(state == LOW){
    if(cur_state.dir != 0){
      cur_state.dir = 0;
      cur_state.dir_changed = true;
    }
  }
}

void onMsgCommand( const String &message ){
  cur_state.dir_changed = false;
  int dir = cur_state.dir;
  if(message == "open"){
    dir = 1;
  } else if(message == "close"){
    dir = 2;
  } else if(message == "stop") {
    dir = 0;
  }

  if(cur_state.dir != dir) {
    cur_state.dir = dir;
    cur_state.dir_changed = true;
  }
}

void report_rel(s_relay rel, int mode){
  if (mode == 0 || ( mode == 1 && rel.rel.changed ) ) {
    client.Publish(rel.rel_p + "/state", String(rel.rel.state));
  }

  rel.rel.changed = false;
}

void report_actuator( s_actuator act_new, int mode ){

  if (mode == 0 || ( mode == 1 && act_new.dir_1.rel.changed ) ) {
    client.Publish( act_new.name + "/" + act_new.dir_1.rel_p + "/state", String(act_new.dir_1.rel.state));
  }

  if (mode == 0 || ( mode == 1 && act_new.dir_2.rel.changed ) ) {
    client.Publish( act_new.name + "/" + act_new.dir_2.rel_p + "/state", String(act_new.dir_2.rel.state));
  }

  if (mode == 0 || ( mode == 1 && act_new.dir_1_end.rel.changed ) ) {
    client.Publish( act_new.name + "/" + act_new.dir_1_end.rel_p + "/state", String(act_new.dir_1_end.rel.state));
  }

  if (mode == 0 || ( mode == 1 && act_new.dir_2_end.rel.changed ) ) {
    client.Publish( act_new.name + "/" + act_new.dir_2_end.rel_p + "/state", String(act_new.dir_2_end.rel.state));
  }

  act_new.dir_1.rel.changed = 
  act_new.dir_2.rel.changed = false;
  act_new.dir_1_end.rel.changed = 
  act_new.dir_2_end.rel.changed = false;
}

void report( int mode ){
  report_rel(cur_state.rpw, mode);
  report_actuator( cur_state.act_1, mode );

  if (mode == 0 || ( mode == 1 && cur_state.dir_changed ) ) {
    client.Publish("dir", String(cur_state.dir));
  }

  if (mode == 0 || ( mode == 1 && cur_state.dht22.h_changed ) ) {
    client.Publish("dht22/h", String(cur_state.dht22.s_h));
  }
  if (mode == 0 || ( mode == 1 && cur_state.dht22.t_changed ) ) {
    client.Publish("dht22/t", String(cur_state.dht22.s_t));
  }

  cur_state.dir_changed = false;
  cur_state.dht22.h_changed = false;
  cur_state.dht22.t_changed = false;

  client.flag_start = false;
}

void actuator_dir(s_actuator &ret, int dir){
  if(dir != 0){
    //включаем БП
    PW_ON_OFF(cur_state.rpw, HIGH);
    delay(100);
  } else {
    PW_ON_OFF(cur_state.rpw, LOW);
  }

// включаем направление вращения
  switch(dir){
    case 1:
      if(digitalRead( ret.dir_2.rel.pin )==HIGH){
        digitalWrite( ret.dir_2.rel.pin, LOW);
        delay(100);
      }
      if(digitalRead( ret.dir_1.rel.pin )!=HIGH){
        digitalWrite( ret.dir_1.rel.pin, HIGH);
      }
      break;
    case 2:
      if(digitalRead( ret.dir_1.rel.pin )==HIGH){
        digitalWrite( ret.dir_1.rel.pin, LOW);
        delay(100);
      }
      if(digitalRead( ret.dir_2.rel.pin )!=HIGH){
        digitalWrite( ret.dir_2.rel.pin, HIGH);
      }
      break;
    default:
      digitalWrite( ret.dir_1.rel.pin, LOW);
      digitalWrite( ret.dir_2.rel.pin, LOW);
      break;
  }
}

void OnCheckState(){
  int a1_dir_1_end = digitalRead( cur_state.act_1.dir_1_end.rel.pin );
  int a1_dir_2_end = digitalRead( cur_state.act_1.dir_2_end.rel.pin );

  switch(cur_state.dir){
    case 1:
//    открываем первый если у него концевик не замкнут
      if(a1_dir_1_end != HIGH){
        actuator_dir(cur_state.act_1, cur_state.dir);
      
//    если концевик последнего актуатора замкнут, то вырубить блок питания
      } else if(a1_dir_1_end == HIGH) {
        PW_ON_OFF(cur_state.rpw, LOW);
      }
      break;

    case 2:
//    закрываем первый
      if(a1_dir_2_end != HIGH){
        actuator_dir(cur_state.act_1, cur_state.dir);

//    если концевик последнего актуатора замкнут, то вырубить блок питания
      } else if(a1_dir_2_end == HIGH) {
        PW_ON_OFF(cur_state.rpw, LOW);
      }
      break;

    default:
//    выключаем все актуаторы и блок питания
      actuator_dir(cur_state.act_1, cur_state.dir);
      PW_ON_OFF(cur_state.rpw, LOW);
      break;
  }

  Read_state( );

  if(client.flag_start){ //первый запуск
    report(0); //отправляем все
  } else {
    report(1); //отправляем только то, что изменилось
  }
}

void onConnection(){

}

void OnLoad(){

}

void loop() {
  client.loop();
}