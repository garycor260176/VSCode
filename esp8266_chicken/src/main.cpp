#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <mqtt_ini.h>
#include <AsyncUDP_Ethernet.h> 
#include <ArduinoJson.h>
#include <Preferences.h>
#include <cl_dht22.h>

#define def_path "CHICKEN/SENS"
#define def_subpath_dht22 "dht22"

#define DHTPIN   D5

const uint16_t PORT = 8888;
IPAddress ADDR; 
AsyncUDP udp;
boolean flag_answer = false;
String answer = "";

Preferences preferences; //хранение текущего состояния

#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
cl_dht22 dht22(&dht, &client, String(def_subpath_dht22), 30000, false);

uint32_t disconnect_start = 0; 
boolean f_disconnect_start = false;
#define disconnect_time_reload 60000 //если в течении 60ти секунд нет связи

mqtt_ini client( 
  "CHICKEN_DHT22",     // Client name that uniquely identify your device
   def_path);

struct s_ip{
  uint8_t ip0;
  uint8_t ip1;
  uint8_t ip2;
  uint8_t ip3;
};

struct s_state{
  s_ip addr;
};

s_state cur_state;
s_state eeprom;

void report( s_state, int);
boolean PubInterval(String topic, uint32_t val);
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

  eeprom = cur_state;  

  return ret;
}

void setup() {
  Serial.begin(115200);

  preferences.begin("settings", false);
  dht22.begin();

  read_eeprom();

  client.begin();
 
  udp.onPacket(parsePacket);
}

void onMsgCommand( const String &message ){}

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

  if(write_eeprom()) 
  ESP.restart( );
}

void onConnection()
{
   client.Subscribe("CHICKEN/ip",  msg_settings_pwr_ip, true);
   dht22.subscribe( ); //[def_path]/[def_subpath_ds18b20]/interval
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
  dht22.loop( );
  
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
