#include <mqtt_ini.h>
#include <Preferences.h>

#define MIN_TIME_CHANGE_STATE 10000
uint32_t TIME_CHANGE_STATE; //время в млс на открытие/закрытие
uint32_t old_TIME_CHANGE_STATE; //время в млс на открытие/закрытие

#include <actuator.h>

#define MQTT_PATH       "GH/MOTOR"
#define PIN_PWR         23

mqtt_ini client( 
  "GH_MOTOR",     // Client name that uniquely identify your device
   MQTT_PATH);

actuator* actuators[8];
int pins_motor[8][2] = { {13, 12}, {14, 27}, {26, 25}, {33, 32}, {19, 18}, {5, 17}, {16, 4}, { 2, 15} };

Preferences preferences; //хранение текущего состояния

String uint64ToString(uint64_t input);

void read_eeprom(){
  TIME_CHANGE_STATE = preferences.getULong("timproc", 40000);
  if(TIME_CHANGE_STATE < MIN_TIME_CHANGE_STATE) TIME_CHANGE_STATE = MIN_TIME_CHANGE_STATE;
  Serial.println("read TIME_CHANGE_STATE = " + String(TIME_CHANGE_STATE));
  old_TIME_CHANGE_STATE = TIME_CHANGE_STATE;
}

void write_eeprom(){
  if(old_TIME_CHANGE_STATE != TIME_CHANGE_STATE) {
    preferences.putULong("timproc", TIME_CHANGE_STATE);
    Serial.println("save TIME_CHANGE_STATE = " + String(TIME_CHANGE_STATE));
  }
  old_TIME_CHANGE_STATE = TIME_CHANGE_STATE;
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  preferences.begin("settings", false);

  pinMode(PIN_PWR, OUTPUT); digitalWrite(PIN_PWR, LOW);

  read_eeprom();

  for(int i = 0; i < 8; i++) {
    actuators[i] = new actuator(pins_motor[i][0], pins_motor[i][1], &client, "W"+String(i));
  }

  client.begin(true);
}

void OnCheckState(){}

void onMsgCommand( const String &message ){}

void onSubscribe(const String &topic, const String &message){
  for(int i = 0; i < 8; i++) {
    actuators[i]->onSubscribe(topic, message);
  }
}

void onSubscribe_T(const String &message){
  TIME_CHANGE_STATE = message.toInt();
  if(TIME_CHANGE_STATE < MIN_TIME_CHANGE_STATE) TIME_CHANGE_STATE = MIN_TIME_CHANGE_STATE;
  write_eeprom();
}

void onConnection(){
  for(int i = 0; i < 8; i++) {
    client.Subscribe(actuators[i]->getName() + "/new_state", onSubscribe);
  }
  client.Subscribe("settings/TIME_CHANGE_STATE", onSubscribe_T);
}  

void OnLoad(){}

void report(int mode ){
  static int OldStatePwr;
  int pwr = digitalRead(PIN_PWR);
  if(mode == 0 || (mode == 1 && OldStatePwr != pwr)) {
    client.Publish("pwr24/state", String(pwr));
  }
  OldStatePwr = pwr;

  if(mode == 0 || (mode == 1 && TIME_CHANGE_STATE != old_TIME_CHANGE_STATE)){
    client.Publish("settings/TIME_CHANGE_STATE", String(TIME_CHANGE_STATE));
  }
  old_TIME_CHANGE_STATE = TIME_CHANGE_STATE;

  for(int i = 0; i < 8; i++) {
    actuators[i]->publish(mode);
  }

  client.flag_start = false;
}

boolean canOpen(String name) {
  for(int i = 0; i < 8; i++) {
    if( actuators[i]->getName() != name) {
      if(actuators[i]->inProcess()) return false;
    }
  }
  return true;
}

void pwrState(int state){
  if(digitalRead(PIN_PWR) == state) return;
  digitalWrite(PIN_PWR, state);
}

void loop() {
  client.loop();

  boolean actuator_working = false;
  for(int i = 0; i < 8; i++) {
    actuators[i]->loop();
    if(actuators[i]->inProcess()) actuator_working = true;
  }
  if(!actuator_working) pwrState(LOW);

  if (client.flag_start) { //первый запуск
    report(0); //отправляем все
  } else {
    report(1); //отправляем все
  }  
}
