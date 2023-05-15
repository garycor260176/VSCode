#include <mqtt_ini.h>
#include <Int64String.h>

#define PIN_1 D5
#define PIN_2 D6

unsigned long ContactTime1;// Timer to manage any contact bounce in interrupt routine 
unsigned long ContactTime2;// Timer to manage any contact bounce in interrupt routine 

struct s_pin{
  int state;
  uint64_t up_time;
  uint64_t down_time;
};

struct s_state{
  s_pin p1;
  s_pin p2;

  int dir;
  int first;
};

boolean p1_changed = false;
uint64_t p1_mil;

boolean p2_changed = false;
uint64_t p2_mil;

s_state last_state;

#define def_path "home/cnt01"
mqtt_ini client( 
  "ESP32_CNT01",     // Client name that uniquely identify your device
   def_path,
   true);

void IRAM_ATTR pin_1(){
  if((millis() - ContactTime1) > 15 ) { // debounce of sensor signal 
    p1_changed = true;
    p1_mil = ContactTime1 = millis(); 

    //
    Serial.println("1 - " + String(digitalRead(PIN_1)) + " : " + int64String(p1_mil));
  } 
}

void IRAM_ATTR pin_2(){
  if((millis() - ContactTime2) > 15 ) { // debounce of sensor signal 
    p2_changed = true;
    p2_mil = ContactTime2 = millis(); 
    //Serial.println("2 - " + String(digitalRead(PIN_2)) + " : " + int64String(p2_mil));
  } 
}

void setup() {
  Serial.begin(115200); // Start the serial port

  // put your setup code here, to run once:
  pinMode(PIN_1, INPUT_PULLUP);
  pinMode(PIN_2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_1),pin_1, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(PIN_2),pin_2, CHANGE); 

  client.begin(true);
}

s_state Read_state( ){
 // put your main code here, to run repeatedly:
  s_state ret = last_state;

  if(p1_changed){
    ret.p1.state = (digitalRead(PIN_1) == LOW ? HIGH: LOW);
    p1_changed = false;
  }

  if(p2_changed){
    ret.p2.state = (digitalRead(PIN_2) == LOW ? HIGH: LOW);
    p2_changed = false;
  }

  if(last_state.p1.state == LOW && ret.p1.state == HIGH) ret.p1.up_time = p1_mil;
  if(last_state.p1.state == HIGH && ret.p1.state == LOW) ret.p1.down_time = p1_mil;

  if(last_state.p2.state == LOW && ret.p2.state == HIGH) ret.p2.up_time = p2_mil;
  if(last_state.p2.state == HIGH && ret.p2.state == LOW) ret.p2.down_time = p2_mil;

  if(ret.first == 0 && ret.p1.state == HIGH && last_state.p1.state == LOW) {
    ret.first = 1;
  }

  if(ret.first == 0 && ret.p2.state == HIGH && last_state.p2.state == LOW) {
    ret.first = 2;
  }

  ret.dir = 0;

  if(ret.p1.state == LOW && ret.p2.state == LOW && ret.first > 0) {
    switch(ret.first){
      case 1:
        if( ret.p2.down_time > ret.p1.down_time &&
            ret.p1.down_time > ret.p1.up_time && 
            ret.p2.up_time > ret.p1.up_time )
        {
          ret.dir = 1;
        }         
        break;

      case 2:
        if( ret.p1.down_time > ret.p2.down_time &&
            ret.p2.down_time > ret.p2.up_time && 
            ret.p1.up_time > ret.p2.up_time )
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

  if(last_state.dir != new_state.dir && new_state.dir > 0) {
    client.Publish("dir", String(new_state.dir));
  }

  if(new_state.dir > 0 || ( new_state.p1.state == LOW && new_state.p2.state == LOW )) 
  { 
    new_state.first = 0;
    new_state.dir = 0;

    new_state.p1.up_time = 0;
    new_state.p1.down_time = 0;

    new_state.p2.up_time = 0;
    new_state.p2.down_time = 0;
  }

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