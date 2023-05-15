#include <mqtt_ini.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include <WiFi.h>
#include "AsyncUDP.h"
#include <ArduinoJson.h>

#include <cl_ds18b20.h>

#define DEF_INTERVAL 30000

const uint16_t PORT = 8888;
IPAddress ADDR; 
AsyncUDP udp;
boolean flag_answer = false;
String answer = "";

String def_udp_id = "1";
#define def_path "GH/T1"
#define def_subpath_ds18b20   "DS18B20"

//пины
#define PIN_DS18B20  32 //датчик температуры

Preferences preferences; //хранение текущего состояния

//MQTT-клиент
mqtt_ini client( 
  "GH_T1",     // Client name that uniquely identify your device
   def_path);

//объект датчика температуры
OneWire oneWire(PIN_DS18B20);
DallasTemperature DS18B20(&oneWire);
cl_ds18b20 ds18b20(&DS18B20, &client, String(def_subpath_ds18b20), DEF_INTERVAL, false);
s_ds18b20_val old_val;

struct s_ip{
  uint8_t ip0;
  uint8_t ip1;
  uint8_t ip2;
  uint8_t ip3;
};

struct s_state{
  s_ip addr;
  uint32_t interval_t;
};

s_state cur_state;
s_state eeprom;

void parsePacket(AsyncUDPPacket packet);
boolean Send_message( String command);

void read_eeprom(){
  cur_state.addr.ip0 = preferences.getInt("ip0", 0);
  cur_state.addr.ip1 = preferences.getInt("ip1", 0);
  cur_state.addr.ip2 = preferences.getInt("ip2", 0);
  cur_state.addr.ip3 = preferences.getInt("ip3", 0);
  ADDR[0] = cur_state.addr.ip0;
  ADDR[1] = cur_state.addr.ip1;
  ADDR[2] = cur_state.addr.ip2;
  ADDR[3] = cur_state.addr.ip3;

  cur_state.interval_t = preferences.getInt("int_t", 0);
  if(cur_state.interval_t <= 0) cur_state.interval_t = DEF_INTERVAL;

  eeprom = cur_state;
}

boolean write_eeprom(){
  boolean ret = false;
  if(cur_state.addr.ip0 != eeprom.addr.ip0)    {
    preferences.putInt("ip0", cur_state.addr.ip0);
    ret = true;
  }
  if(cur_state.addr.ip1 != eeprom.addr.ip1)    {
    preferences.putInt("ip1", cur_state.addr.ip1);
    ret = true;
  }
  if(cur_state.addr.ip2 != eeprom.addr.ip2)    {
    preferences.putInt("ip2", cur_state.addr.ip2);
    ret = true;
  }
  if(cur_state.addr.ip3 != eeprom.addr.ip3)    {
    preferences.putInt("ip3", cur_state.addr.ip3);
    ret = true;
  }
  ADDR[0] = cur_state.addr.ip0;
  ADDR[1] = cur_state.addr.ip1;
  ADDR[2] = cur_state.addr.ip2;
  ADDR[3] = cur_state.addr.ip3;

  cur_state.interval_t = ds18b20.get_interval();

  if(cur_state.interval_t != eeprom.interval_t)    {
    preferences.putInt("int_t", cur_state.interval_t);
  }

  eeprom = cur_state;  

  return ret;
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  preferences.begin("settings", false);

  cur_state.interval_t = DEF_INTERVAL;
  ds18b20.set_interval(cur_state.interval_t);
  ds18b20.begin();

  read_eeprom();

  client.begin(true);

  udp.onPacket(parsePacket);
}

void onMsgCommand( const String &message ){}

void OnCheckState(){
  ds18b20.loop( );
  s_ds18b20_val new_val = ds18b20.get_value( );
  if(new_val.s_value != old_val.s_value){
    String msg = "";
    DynamicJsonDocument doc(1024);
    doc["d"] = def_udp_id;
    doc["t"] = new_val.s_value;
    serializeJson(doc, msg);
    Send_message(msg);
  }
  old_val = new_val;
  write_eeprom( );

  client.flag_start = false;
}

void msg_settings_pwr_ip( const String &message ){
  if(message.length() == 0) return;

  IPAddress apip;
  if(!apip.fromString(message)) return;

  cur_state.addr.ip0 = apip[0];
  cur_state.addr.ip1 = apip[1];
  cur_state.addr.ip2 = apip[2];
  cur_state.addr.ip3 = apip[3];

  String new_ip = String(cur_state.addr.ip0) + "." + String(cur_state.addr.ip1) + "." + String(cur_state.addr.ip2) + "." + String(cur_state.addr.ip3);
  client.Publish("pwr_ip", new_ip);

  if(write_eeprom()) ESP.restart( );
}

void onConnection(){
  client.Subscribe("GH/MOTOR/ip",  msg_settings_pwr_ip, true);
  ds18b20.subscribe( ); //[def_path]/[def_subpath_ds18b20]/interval
}

void OnLoad(){}

void loop() {
  client.loop();
  if(!client.CheckStateCalled){
    OnCheckState( );
  }
}

// Определяем callback функцию обработки пакета
void parsePacket(AsyncUDPPacket packet)
{
  flag_answer = true;
  answer = packet.readString();
  Serial.println("UDP message: " + answer);
}

boolean Send_message(String command){
  boolean ret = false;

  Serial.println("Send UDP-message: " + command);

  if ((WiFi.status() == WL_CONNECTED)) {
    if(!udp.connected() && ADDR[0] != 0) {
      if (udp.connect(ADDR, PORT)) {
            Serial.println("UDP подключён");
      }
    }
    if(udp.connected()) {
      flag_answer = false;
      answer = "";
      uint64_t mil = millis( );
      udp.broadcastTo(command.c_str( ), PORT);
      while(!flag_answer) {
        if(millis() - mil > 5000) {
          break;
        }
      }
      if(answer == "ok!") {
        ret = true;
      }
    } else {
      Serial.println("Ошибка UDP-запроса");
    }
  } 
  return ret;
}
