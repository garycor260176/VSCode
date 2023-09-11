#include <mqtt_ini.h>

#define DEF_PATH "LAMPS/L01"

#define RELAYS_COUNT  4

int pins[RELAYS_COUNT] = { 5, 23, 19, 18 };

mqtt_ini client( 
  "ESP32_L01",
   DEF_PATH);

struct s_relay {
  String name;
  int state;
};

s_relay relays[RELAYS_COUNT];

String getName(int i) {
  String ret = String(i);
  if(ret.length() == 1) ret = "r0" + ret;
  else ret = "r" + ret;
  return ret;
}

void setup() {
  Serial.begin(115200);                                         
  Serial.println("");  Serial.println("Start!");

  for(int i = 0; i < RELAYS_COUNT; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
    relays[i].name = getName(i);
    relays[i].state = 0;
  }

  client.begin(true);
}

void Msg_state(const String &topicStr, const String &message) {
  Serial.println("topic = " + topicStr + ", msg = " + message);

  String topic = DEF_PATH; topic = topic + "/states/";

  for(int i = 0; i < RELAYS_COUNT; i++) {
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
  for(int i = 0; i < RELAYS_COUNT; i++) {
    if (mode == 0 || ( mode == 1 && relays[i].state != digitalRead(pins[i]) ) )  
      client.Publish("states/" + getName(i), String(relays[i].state));
  }
  client.flag_start = false;
}

void loop() {
  client.loop();

  for(int i = 0; i < RELAYS_COUNT; i++) {
    if(relays[i].state != digitalRead(pins[i])) {
      digitalWrite(pins[i], relays[i].state);
    }
  }

  if (client.flag_start) { //первый запуск
    report(0);
  } else {
    report(1);
  }  
}
