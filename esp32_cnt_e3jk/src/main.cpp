#include <mqtt_ini.h>

#define INT_1			18
#define INT_2			19

struct s_dev{
  boolean in;

  uint64_t up_time;
  uint64_t down_time;
};

struct s_state{
  s_dev dev_1;
  s_dev dev_2;

  int on;
  int dir;
  int first;
};

//последнее состояние
s_state last_state;

//mqtt-клиент
#define def_path "home/cnt01"
mqtt_ini client( 
  "ESP32_CNT01",     // Client name that uniquely identify your device
   def_path,
   true);

unsigned long ContactTime_1;// Timer to manage any contact bounce in interrupt routine 
boolean newData_1 = false;
int State_1 = LOW;

unsigned long ContactTime_2;// Timer to manage any contact bounce in interrupt routine 
boolean newData_2 = false;
int State_2 = LOW;

void IRAM_ATTR L1() { 
  if((millis() - ContactTime_1) > 10 ) { // debounce of sensor signal 
    newData_1 = true;
    ContactTime_1 = millis(); 
  } 
} 

void IRAM_ATTR L2() { 
  if((millis() - ContactTime_2) > 30 ) { // debounce of sensor signal 
    newData_2 = true;
    ContactTime_2 = millis(); 
  } 
} 

void setup() {
  Serial.begin(115200); // Start the serial port
  Serial.println(F(""));  Serial.println(F("Start..."));

  pinMode(INT_1, INPUT);
  pinMode(INT_2, INPUT);
  delay(100);

  attachInterrupt(INT_1, L1, CHANGE); 
  attachInterrupt(INT_2, L2, CHANGE); 

  client.begin(true);
}

s_state Read_state( ){
  s_state ret = last_state;

  boolean f = false; //признак изменения состояния (чисто для вывода в сериал для отладки)

  //первый датчик
  if(newData_1) {
    State_1 = digitalRead(INT_1); //текущее состояние (0 - датчик взвелся, 1 - сбросился)
    newData_1 = false;

    ret.dev_1.in = (State_1==0 ? true : false); ; //true - если луч был прерван
    if(!last_state.dev_1.in && ret.dev_1.in) ret.dev_1.up_time = ContactTime_1; //если состояние изменилось с - на +, то запоминаем время взвода датчика
    if(last_state.dev_1.in && !ret.dev_1.in) ret.dev_1.down_time = ContactTime_1; //если состояние изменилось с + на -, то запоминаем время сброса датчика
    if(ret.first == 0 && ret.dev_1.in) { //если еще не происходило взвода датчиков, то запоминаем, что первым сработал первый датчик
      if(ret.dev_1.in) ret.first = 1;
    }

    if(last_state.dev_1.in != ret.dev_1.in) f = true; //если состояние изменилось значит надо будет выдать в сериал
  }

  //второй датчик
  if(newData_2) {
    State_2 = digitalRead(INT_2);
    newData_2 = false;

    ret.dev_2.in = (State_2==0 ? true : false) ; //true - если луч был прерван
    if(!last_state.dev_2.in && ret.dev_2.in) ret.dev_2.up_time = ContactTime_2; //если состояние изменилось с - на +, то запоминаем время взвода датчика
    if(last_state.dev_2.in && !ret.dev_2.in) ret.dev_2.down_time = ContactTime_2; //если состояние изменилось с + на -, то запоминаем время сброса датчика
    if(ret.first == 0 && ret.dev_2.in) { //если еще не происходило взвода датчиков, то запоминаем, что первым сработал второй датчик
      if(ret.dev_2.in) ret.first = 2;
    }

    if(last_state.dev_2.in != ret.dev_2.in) f = true; //если состояние изменилось значит надо будет выдать в сериал
  }
  
  ret.dir = 0; //сбрасываем признак направления

  if(f) Serial.println(String(ret.first) + " " + String(ret.dev_1.in) + String(ret.dev_2.in)); //отладка

  ret.on = 0;
  if(ret.dev_1.in && ret.dev_2.in) ret.on = 1;

  //если оба датчика сброшены и какой-то сработал первый (либо полностью зашли. либо полностью вышли)
  if(!ret.dev_1.in && !ret.dev_2.in && ret.first > 0) {
    switch(ret.first){
      case 1: //первым сработал первый датчик
        //если сработало в порядке: 1й взвелся, 2й взвелся, 1й сбросился, 2й сбросился, значит прошли вперед
        if( 
            ret.dev_2.down_time > ret.dev_1.down_time &&
            ret.dev_1.down_time > ret.dev_2.up_time && 
            ret.dev_2.up_time > ret.dev_1.up_time
            )
        {
          ret.dir = 1;
        }         
        break;

      case 2: //первым сработал второй датчик
        //если сработало в порядке: 2й взвелся, 1й взвелся, 2й сбросился, 1й сбросился, значит прошли назад
        if( 
            ret.dev_1.down_time > ret.dev_2.down_time &&
            ret.dev_2.down_time > ret.dev_1.up_time && 
            ret.dev_1.up_time > ret.dev_2.up_time 
            )
        {
          ret.dir = 2;
        }
        break;
    }
  }

  return ret;
};

void onMsgCommand( const String &message ){

}

void OnCheckState(){
  s_state new_state = Read_state( );

  //если признак направления изменился, то выдаем его в MQTT для отработки скрипта управления светом
  //если 1, то счетчик увеличим, если 2 - уменьшим
  if(last_state.dir != new_state.dir && new_state.dir > 0) {
    client.Publish("dir", String(new_state.dir));
  }

  if(last_state.on != new_state.dir && new_state.on > 0) {
    client.Publish("on", String(new_state.on));
  }

  //если направление было посчитано или оба датчика сброшены, то сбросим все, что запоминали
  if(new_state.dir > 0 || ( !new_state.dev_1.in && !new_state.dev_2.in )) 
  { 
    new_state.dir = 0;
    new_state.first = 0;

    new_state.dev_1.up_time = 
    new_state.dev_1.down_time = 
    new_state.dev_2.up_time = 
    new_state.dev_2.down_time = 0;
  }

  //запомним состояние
  last_state = new_state;

  client.flag_start = false;
}

void onConnection(){
  
}

void OnLoad(){
}

void loop() {
  client.loop();
}