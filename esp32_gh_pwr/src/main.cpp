#include <mqtt_ini.h>
#include <Wire.h>
#include <Preferences.h>
#include <AsyncUDP.h>
#include <ArduinoJson.h>

#include <pin.h>
#include <settings.h>
#include <wnd.h>

#define def_path "GH/MOTOR"

#define start_after 60000
uint64_t mil_start_after = 0;
int f_start_after = 0;

#define time_open_wnd   40000 //время в млс на открытие/закрытие
uint64_t start_process_wnd = 0;
boolean f_start_process_wnd = false;
int proc_wnd_num = -1;

#define PIN_PWR24    23
int pins_motor[16] = {
                      13, 12, 14, 27, 
                      26, 25, 33, 32, 
                      19, 18, 5, 17,
                      16, 4, 2, 15 
                     };

int count_wnd = 0;

pin_h* pwr24;
wnd* wnds[8];

mqtt_ini client( 
  "GH_MOTOR",     // Client name that uniquely identify your device
   def_path);

Preferences preferences; //хранение текущего состояния
settings ini(&preferences);

IPAddress local_IP(192, 168, 1, 136);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
AsyncUDP udp;
const uint16_t port = 8888;

void parsePacket(AsyncUDPPacket packet);
int get_wnd_in_process();

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  Wire.begin();

  ini.begin("settings");
  ini.read();

//pwr24
  pwr24 = new pin_h(PIN_PWR24, OUTPUT, &client, "pwr24", NULL);

// Настраивает статический IP-адрес
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

//окна
  for(int i=0; i<8; i++){ 
    wnds[count_wnd] = new wnd(pwr24, &ini, NULL, pins_motor[2*i], pins_motor[2*i + 1], &client, "W"+String(count_wnd), "W"+String(count_wnd));
    wnds[count_wnd]->read_ini( );
    count_wnd++;
  }
  client.begin(true);

  // Инициируем сервер
  if(udp.listen(port)) {
      // При получении пакета вызываем callback функцию
      udp.onPacket(parsePacket);
  }

}

void onMsgCommand( const String &message ){
}

void msg_settings_t_open( const String &message ){
  Serial.println("msg_settings_t_open");
  ini.t_open = message.toInt( );
  if(ini.t_open < 0) {
    ini.t_open = 0;
  }

  if(ini.t_open < ini.t_close) {
    ini.t_open = ini.t_close;
  }
}

void msg_settings_t_close( const String &message ){
  Serial.println("msg_settings_t_close");
  ini.t_close = message.toInt( );
  if(ini.t_close < 0) {
    ini.t_close = 0;
  }
  if(ini.t_close > ini.t_open) {
    ini.t_close = ini.t_open;
  }
}

void set_wnd_mode(wnd* win, int mode){
  if(win->mode == mode) return;
  win->mode = mode;
  if(win->mode < 0 || win->mode > 2) win->mode = 0;
}

void msg_settings_W0_mode( const String &message ){
  Serial.println("msg_settings_W0_mode = " + message);
  set_wnd_mode(wnds[0], message.toInt( ));
  wnds[0]->set_need_state(ini.d1_temp);
}

void msg_settings_W1_mode( const String &message ){
  Serial.println("msg_settings_W1_mode = " + message);
  set_wnd_mode(wnds[1], message.toInt( ));
  wnds[1]->set_need_state(ini.d1_temp);
}

void msg_settings_W2_mode( const String &message ){
  Serial.println("msg_settings_W2_mode = " + message);
  set_wnd_mode(wnds[2], message.toInt( ));
  wnds[2]->set_need_state(ini.d2_temp);
}

void msg_settings_W3_mode( const String &message ){
  Serial.println("msg_settings_W3_mode = " + message);
  set_wnd_mode(wnds[3], message.toInt( ));
  wnds[3]->set_need_state(ini.d2_temp);
}

void msg_settings_W4_mode( const String &message ){
  Serial.println("msg_settings_W4_mode = " + message);
  set_wnd_mode(wnds[4], message.toInt( ));
  wnds[4]->set_need_state(ini.d3_temp);
}

void msg_settings_W5_mode( const String &message ){
  Serial.println("msg_settings_W5_mode = " + message);
  set_wnd_mode(wnds[5], message.toInt( ));
  wnds[5]->set_need_state(ini.d3_temp);
}

void msg_settings_W6_mode( const String &message ){
  Serial.println("msg_settings_W6_mode = " + message);
  set_wnd_mode(wnds[6], message.toInt( ));
  wnds[6]->set_need_state(ini.d4_temp);
}

void msg_settings_W7_mode( const String &message ){
  Serial.println("msg_settings_W7_mode = " + message);
  set_wnd_mode(wnds[7], message.toInt( ));
  wnds[7]->set_need_state(ini.d4_temp);
}

void onConnection(){
  client.Subscribe("settings/t_open",  msg_settings_t_open);
  client.Subscribe("settings/t_close",  msg_settings_t_close);

  client.Subscribe("W0/mode",  msg_settings_W0_mode);
  client.Subscribe("W1/mode",  msg_settings_W1_mode);
  client.Subscribe("W2/mode",  msg_settings_W2_mode);
  client.Subscribe("W3/mode",  msg_settings_W3_mode);
  client.Subscribe("W4/mode",  msg_settings_W4_mode);
  client.Subscribe("W5/mode",  msg_settings_W5_mode);
  client.Subscribe("W6/mode",  msg_settings_W6_mode);
  client.Subscribe("W7/mode",  msg_settings_W7_mode);
}

void OnLoad(){}

void loop() {
  client.loop();

  if(!client.CheckStateCalled){
    OnCheckState();
  }
}

// Определяем callback функцию обработки пакета
void parsePacket(AsyncUDPPacket packet)
{
  String msg = packet.readString();
  Serial.println("UDP message: " + msg);
  packet.print("ok!");

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, msg);  
  if( error ) { Serial.printf("Error on deserialization: %s\n", error.c_str() ); return; }

  String d = doc["d"]; //номер сенсора
  String t = doc["t"]; //температура

  int w_from = -1;
  int w_to = -1;
  int temp = false;
  boolean f_temp;
  if(d == "1") {
    temp = ini.d1_temp = t.toInt( );
    f_temp = ini.f_d1_temp = true;
    w_from = 0; w_to = 1;
  } else if(d == "2") {
    temp = ini.d2_temp = t.toInt( );
    f_temp = ini.f_d2_temp = true;
    w_from = 2; w_to = 3;
  } else if(d == "3") {
    temp = ini.d3_temp = t.toInt( );
    f_temp = ini.f_d3_temp = true;
    w_from = 4; w_to = 5;
  } else if(d == "4") {
    temp = ini.d4_temp = t.toInt( );
    f_temp = ini.f_d4_temp = true;
    w_from = 6; w_to = 7;
  } else {
    return;
  }

  if(f_temp){
    for(int i=w_from; i<=w_to; i++){
      wnds[i]->set_need_state(temp);
    }
  }
}

void OnCheckState(){
  pwr24->loop();

  switch(f_start_after){
    case 0:
      f_start_after++;
      mil_start_after = millis();
      break;
    case 1:
      if(millis() - mil_start_after > start_after) {
        f_start_after++;
      }
      break;
    case 2:
      if(f_start_process_wnd){
        if(millis() - start_process_wnd > time_open_wnd){
          wnds[proc_wnd_num]->stop();
          f_start_process_wnd = false;
        }
      }

      if(!f_start_process_wnd) {
        for(int i=0; i<count_wnd; i++){
          int t;
          boolean f = false;      
          if(i>=0 && i<=1) { t = ini.d1_temp; f = ini.f_d1_temp; }
          else if(i>=2 && i<=3) { t = ini.d2_temp; f = ini.f_d2_temp; }
          else if(i>=4 && i<=5) { t = ini.d3_temp; f = ini.f_d3_temp; }
          else if(i>=6 && i<=7) { t = ini.d4_temp; f = ini.f_d4_temp; }
          if(f || wnds[i]->new_state( )){
            if(wnds[i]->start_process( t )) {
              proc_wnd_num = i;
              f_start_process_wnd = true;
              start_process_wnd = millis();
              break;
            }
          }
        }
      }
      break;
  }

  if(!f_start_process_wnd) {
    proc_wnd_num = 0;
    pwr24->set(LOW);
  }

  ini.write();
  for(int i=0; i<count_wnd; i++){
    wnds[i]->publish( );
    wnds[i]->save_ini( );
  }
  
  client.flag_start = false;
}