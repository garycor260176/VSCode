#include <Arduino.h>

#define INT_1			D4
#define INT_2			D6

struct s_dev{
  boolean in;

  uint64_t up_time;
  uint64_t down_time;
};

struct s_state{
  s_dev dev_1;
  s_dev dev_2;

  int dir;
  int first;
};

//последнее состояние
s_state last_state;

void setup() {
  pinMode(INT_1, INPUT);
  pinMode(INT_2, INPUT);
  delay(100);
}

s_state Read_state( ){
  s_state ret = last_state;
  //первый датчик
  int State_1 = digitalRead(INT_1); //текущее состояние (0 - датчик взвелся, 1 - сбросился)
  uint32_t ContactTime_1 = millis( ); //время
  ret.dev_1.in = (State_1==0 ? true : false) ; //true - если луч был прерван
  if(!last_state.dev_1.in && ret.dev_1.in) ret.dev_1.up_time = ContactTime_1; //если состояние изменилось с - на +, то запоминаем время взвода датчика
  if(last_state.dev_1.in && !ret.dev_1.in) ret.dev_1.down_time = ContactTime_1; //если состояние изменилось с + на -, то запоминаем время сброса датчика
  if(ret.first == 0 && ret.dev_1.in) { //если еще не происходило взвода датчиков, то запоминаем, что первым сработал первый датчик
    if(ret.dev_1.in) ret.first = 1;
  }

  int State_2 = digitalRead(INT_2);
  uint32_t ContactTime_2 = ContactTime_1; //время
  ret.dev_2.in = (State_2==0 ? true : false) ; //true - если луч был прерван
  if(!last_state.dev_2.in && ret.dev_2.in) ret.dev_2.up_time = ContactTime_2; //если состояние изменилось с - на +, то запоминаем время взвода датчика
  if(last_state.dev_2.in && !ret.dev_2.in) ret.dev_2.down_time = ContactTime_2; //если состояние изменилось с + на -, то запоминаем время сброса датчика
  if(ret.first == 0 && ret.dev_2.in) { //если еще не происходило взвода датчиков, то запоминаем, что первым сработал второй датчик
    if(ret.dev_2.in) ret.first = 2;
  }

  ret.dir = 0; //сбрасываем признак направления

  //если оба датчика сброшены и какой-то сработал первый (либо полностью зашли. либо полностью вышли)
  if(!ret.dev_1.in && !ret.dev_2.in && ret.first > 0) {
    switch(ret.first){
      case 1: //первым сработал первый датчик
        //если сработало в порядке: 1й взвелся, 2й взвелся, 1й сбросился, 2й сбросился, значит прошли вперед
        if( ret.dev_2.down_time > ret.dev_1.down_time &&
            ret.dev_1.down_time > ret.dev_1.up_time && 
            ret.dev_2.up_time > ret.dev_1.up_time )
        {
          ret.dir = 1;
        }         
        break;

      case 2: //первым сработал второй датчик
        //если сработало в порядке: 2й взвелся, 1й взвелся, 2й сбросился, 1й сбросился, значит прошли назад
        if( ret.dev_1.down_time > ret.dev_2.down_time &&
            ret.dev_2.down_time > ret.dev_2.up_time && 
            ret.dev_1.up_time > ret.dev_2.up_time )
        {
          ret.dir = 2;
        }
        break;
    }
  }

  return ret;
};

void OnCheckState(){
  s_state new_state = Read_state( );

  if(new_state.dir == 1) {
    //счетчик надо увеличить
  }
  else if(new_state.dir == 2) {
    //счетчик надо уменьшить
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
}

void loop() {
  OnCheckState( );
}