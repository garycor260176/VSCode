#include <mqtt_ini.h>

#define def_path "RELS02"
#define def_relays  17

int pins[] = {
   15,
   2,
   4,
   16,
   17,
   5,
   18,
   19,
   23,
   32,
   33,
   25,
   26,
   27,
   14,
   12,
   13 };

mqtt_ini client( 
  "ESP32_RELS02",     // Client name that uniquely identify your device
   def_path);

struct s_relay {
  String name;
  int state;
};

s_relay relays[17];

String getName(int i) {
  String ret = String(i);
  if(ret.length() == 1) ret = "r0" + ret;
  else ret = "r" + ret;
  return ret;
}

void setup() {
  Serial.begin(115200);                                         
  Serial.println("");  Serial.println("Start!");

  for(int i = 0; i < def_relays; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
    relays[i].name = getName(i);
    relays[i].state = 0;
  }

  client.begin();
}

void Msg_state(const String &topicStr, const String &message) {
  Serial.println("topic = " + topicStr + ", msg = " + message);

  String topic = def_path; topic = topic + "/states/";

  for(int i = 0; i < def_relays; i++) {
    if(topic + getName(i) == topicStr) {
      relays[i].state = ( message.toInt() == 1 ? 1 : 0 );
    }
  }
}

void onMsgCommand( const String &message ){}
void OnLoad(){}
void OnCheckState(){}
void onConnection(){
  client.Subscribe("states/#", Msg_state); 
}

void report(int mode ){
  for(int i = 0; i < def_relays; i++) {
    if (mode == 0 || ( mode == 1 && relays[i].state != digitalRead(pins[i]) ) )  
      client.Publish("states/" + getName(i), String(relays[i].state));
  }
  client.flag_start = false;
}

void loop() {
  client.loop();

  if (client.flag_start) { //первый запуск
    report(0); //отправляем все
  } else {
    report(1); //отправляем все
  }  

  for(int i = 0; i < def_relays; i++) {
    if(relays[i].state != digitalRead(pins[i])) {
      digitalWrite(pins[i], relays[i].state);
    }
  }
}