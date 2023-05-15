#include <mqtt_ini.h>
#include <WiFi.h>
#include <AsyncUDP.h>

#define def_path "GH/PWR24_test"

#define PIN_PWR24    12

IPAddress local_IP(192, 168, 1, 136);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

AsyncUDP udp;
const uint16_t port = 8888;

mqtt_ini client( 
  "GH_PWR24",     // Client name that uniquely identify your device
   def_path);

struct s_pin{
  int pin;
  int state;
};

struct s_pwr{
  s_pin pwr;
  int count_on;
};

struct s_state{
  s_pwr pwr24;
};

s_state cur_state;

void report( s_state, int);

void parsePacket(AsyncUDPPacket packet)
{

    // Записываем адрес начала данных в памяти
    const uint8_t* msg = packet.data();

    // Записываем размер данных
    const size_t len = packet.length();

    // Объект для хранения состояния светодиода в строковом формате
    String state;

    // Если адрес данных не равен нулю и размер данных больше нуля...
    if (msg != NULL && len > 0) {
        // Если первый байт данных содержит 0x1
        if (msg[0] == 49) {
            // записываем строку в объект String
            state = "включён";
        }
        // Если первый байт данных содержит 0x0
        else if (msg[0] == 48) {
            // записываем строку в объект String
            state = "выключен";
        }

        // Отправляем данные клиенту
        packet.printf("Светодиод %s", state.c_str());

        // Выводим состояние светодиода в последовательный порт
        Serial.println("Светодиод " + state);
    }
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println(F(""));
  Serial.println(F("Start!"));

  cur_state.pwr24.pwr.pin = PIN_PWR24;

//pwr
  pinMode(cur_state.pwr24.pwr.pin, OUTPUT);
  digitalWrite(cur_state.pwr24.pwr.pin, LOW);

// Настраивает статический IP-адрес
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  client.begin(true);

  // Инициируем сервер
  if(udp.listen(port)) {
      // При получении пакета вызываем callback функцию
      udp.onPacket(parsePacket);
  }
}

void onMsgCommand( const String &message ){}

void report( s_state sState, int mode ){
  if (mode == 0 || ( mode == 1 && sState.pwr24.pwr.state != cur_state.pwr24.pwr.state )) {
    client.Publish("state", String(sState.pwr24.pwr.state));
  }

  if (mode == 0 || ( mode == 1 && sState.pwr24.count_on != cur_state.pwr24.count_on )) {
    client.Publish("count_on", String(sState.pwr24.count_on));
  }

  cur_state = sState;
  client.flag_start = false;
}

s_state Read_state( s_state S){
  s_state ret = S;

  ret.pwr24.pwr.state   = digitalRead(ret.pwr24.pwr.pin);

  return ret;
};

void OnCheckState(){
  s_state state = Read_state( cur_state );

  if(client.flag_start){ //первый запуск    
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void onConnection(){}

void OnLoad(){}

void loop() {
  client.loop();
}