#include <Arduino.h>
#include <Bounce2.h>
#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SPIFFS.h"
#include <Preferences.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 300

String esp32_cap_ip = "192.168.2.1";

#define STASSID  "hacker"
#define STAPSK   "nthfgtdn"
const char* ssid = STASSID;
const char* pass = STAPSK;

#define MOVE_PIN          32
#define LED_PIN           13
#define PIN_END           18
#define PIN_WATCHDOG_IN   25
#define PIN_WATCHDOG_OUT  26

#define LED_COUNT     8
#define CURRENT_LIMIT 1000 
CRGB strip[LED_COUNT];

boolean photo_creating;
boolean photo_not_sended;

// Выбираем модем
#define TINY_GSM_MODEM_SIM800 // Выбираем нашу плату
 
// Резервируем последовательный порт для монитора порта
#define SerialMon Serial
// И второй порт для команд AT
#define SerialAT Serial1
 
#define TINY_GSM_DEBUG SerialMon

uint64_t save_interval;
uint64_t interval_start;

// указываем пин GSM, если он есть
#define GSM_PIN ""
 
boolean first_read;
#define Interval_working 43200000
uint64_t Last_Working;

#define Interval_started 5000
uint64_t started_time;

boolean started;
boolean started1;

// Указываем пин-код сим-карты, если требуется
const char simPIN[]   = ""; 
 
#define DUMP_AT_COMMANDS

#include <Wire.h>
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

uint64_t lastUpdate;                                   // Время последнего обновления
#define updatePeriod 60000                             // Проверять каждую минуту
enum ReadSMSMode
{
  ReceivedUnread,
  ReceivedRead,
  StoredUnsent,
  StoredSent,
  AllSMS
};

TinyGsmClient client(modem);
WiFiClient wifi;

struct s_end{
  int pin;
  int state;
  boolean changed;
};

struct s_state{
  String sms_text = "";
  String phone = "";
  int proc = 0;

  s_end end;
  s_end move;
};

s_state cur_state; //предыдущее отправленное состояние 

#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

// определяем выход I2C для платы SIM800 
TwoWire I2CPower = TwoWire(0);

void sendUSSD(const String& code);
bool setPowerBoostKeepOn(int en);
void Send_msg(String msg);
void Create_photo( );
void LedOff();
void LedOn();
void Delete_photo( );
uint32_t get_free_size();
void Reboot( );
byte sendATcommand(String ATcommand, String answer1, String answer2, unsigned int timeout);
boolean gprs_modem_function(String filename, String fileSPIFF, uint64_t file_num);
void send_msg_01(String phone, String msg);
String uint64ToString(uint64_t input);
boolean SendFile2FTP(String filename, boolean save = false);
void SaveLastNum(uint64_t last);
boolean DownloadBin();

Bounce* debouncer = new Bounce[1];

Preferences preferences;

struct settings{
  String APN;
  String APN_user;
  String APN_pwd;

  boolean no_ftp;
  String ftp;
  String ftp_user;
  String ftp_pwd;
  uint64_t last_ftp_fn;

  uint64_t CPAO;
  boolean start_photo;

  String phone1;
  String phone2;
  String phone3;

  boolean no_flash;

  int sms_level;
};

String rec_phone;

String set_phone = "+79162903383;+79683736287;+79151249560";

settings set;

void setup() {
  SerialMon.begin(115200);                                         // Скорость обмена данными с компьютером
  SerialMon.println(F(""));
  SerialMon.println(F("Start!"));

  SerialMon.println(F("Inizializing FS..."));
  if (SPIFFS.begin(true)){
      SerialMon.println(F("SPIFFS mounted correctly."));
  } else {
      SerialMon.println(F("!An error occurred during SPIFFS mounting"));
  }

  SerialMon.println(F("Read settings..."));
  preferences.begin("set-cat", true); 
  set.APN = preferences.getString("APN", "internet.mts.ru"); //SerialMon.println("APN = " + set.APN);
  set.APN_user = preferences.getString("APN_user", "mts"); //SerialMon.println("APN_user = " + set.APN_user);
  set.APN_pwd = preferences.getString("APN_pwd", "mts"); //SerialMon.println("APN_pwd = " + set.APN_pwd);
  set.no_ftp = preferences.getBool("no_ftp", true);   if(set.no_ftp) //SerialMon.println("no_ftp = true"); else SerialMon.println("no_ftp = false");
  set.ftp = preferences.getString("ftp", ""); //SerialMon.println("ftp = " + set.ftp);
  set.ftp_user = preferences.getString("ftp_user", ""); //SerialMon.println("ftp_user = " + set.ftp_user);
  set.ftp_pwd = preferences.getString("ftp_pwd", ""); //SerialMon.println("ftp_pwd = " + set.ftp_pwd);
  set.last_ftp_fn = preferences.getULong64("last_ftp_fn", 0); //SerialMon.println("last_ftp_fn = " + uint64ToString(set.last_ftp_fn));
  set.CPAO = preferences.getULong64("CPAO", 3000); //SerialMon.print("CPAO = "); SerialMon.println(set.CPAO);
  set.start_photo = preferences.getBool("start_photo", true); //SerialMon.println("start_photo = " + String(set.start_photo));
  set.phone1 = preferences.getString("phone1", ""); //SerialMon.println("phone1 = " + set.phone1);
  set.phone2 = preferences.getString("phone2", ""); //SerialMon.println("phone2 = " + set.phone2);
  set.phone3 = preferences.getString("phone3", ""); //SerialMon.println("phone3 = " + set.phone3);
  set.no_flash = preferences.getBool("no_flash", true);   //if(set.no_flash) SerialMon.println("no_flash = true"); else SerialMon.println("no_flash = false");
  set.sms_level = preferences.getInt("sms_level", 1);   //SerialMon.println("sms_level = " + String(set.sms_level));

  preferences.end();

  FastLED.addLeds<WS2812B, LED_PIN, RGB>(strip, LED_COUNT);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(255);
  FastLED.clear();
  LedOff();

  cur_state.end.pin = PIN_END; pinMode(cur_state.end.pin, INPUT);
  debouncer[0].attach(cur_state.end.pin);
  debouncer[0].interval(25);

  cur_state.move.pin = MOVE_PIN; pinMode(cur_state.move.pin, INPUT);

  pinMode(PIN_WATCHDOG_IN, INPUT);
  pinMode(PIN_WATCHDOG_OUT, OUTPUT); digitalWrite(PIN_WATCHDOG_OUT, LOW);

  SerialMon.println("Initializing modem...");
  I2CPower.begin(I2C_SDA, I2C_SCL, 400000);
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  modem.restart();
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }

  SerialMon.println();  SerialMon.print("Connecting to ");  SerialMon.print(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED) {
    delay(2000);
    SerialMon.print(".");
  }
  SerialMon.println("");
  // print the SSID of the network you're attached to:
  SerialMon.print("SSID: ");  SerialMon.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  SerialMon.print("IP Address: ");  SerialMon.println(ip);

  modem.sendAT(GF("+CMGF=1"));
  modem.sendAT(GF("+CMGD=1,4"));

  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

}
  
//set mode 0 for reading latest unread message first
//set mode 1 for reading oldest unread message first
int newMessageIndex(bool mode){
  modem.sendAT(GF("+CMGF=1"));
  modem.waitResponse();  
  modem.sendAT(GF("+CMGL=\"REC UNREAD\",1"));
  String h = modem.stream.readString();
  //SerialMon.println("modem = " + h);
  int i;
  if(mode){
      i  = h.indexOf("+CMGL: ");
  }else{
      i  = h.lastIndexOf("+CMGL: ");
  }
  int index=h.substring(i+7,i+9).toInt();
  if(index<=0)return 0;
  return index;
}

bool streamSkipUntil(const char c, const unsigned long timeout = 3000L) {
  unsigned long startMillis = millis();
  while (millis() - startMillis < timeout) {
    while (millis() - startMillis < timeout && !modem.stream.available()) {
      TINY_GSM_YIELD();
    }
    if (modem.stream.read() == c)
      return true;
  }
  return false;
}

String readSMS(int index, const bool changeStatusToRead = true){
  modem.sendAT(GF("+CMGF=1"));
  modem.waitResponse();  
  modem.sendAT(GF("+CMGR="), index, GF(","), static_cast<const uint8_t>(!changeStatusToRead)); 
  String h="";
  streamSkipUntil('\n');
  streamSkipUntil('\n');
  h = modem.stream.readStringUntil('\n');
  if(h.length()>0){
    h = h.substring(0, h.length()-1);
  }
  return h;
}

String getSenderID(int index, const bool changeStatusToRead = true){
  modem.sendAT(GF("+CMGF=1"));
  modem.waitResponse(); 
  modem.sendAT(GF("+CMGR="), index, GF(","), static_cast<const uint8_t>(changeStatusToRead)); 
  String h="";
  streamSkipUntil('"');
  streamSkipUntil('"');
  streamSkipUntil('"');
  h = modem.stream.readStringUntil('"');
  modem.stream.readString();
  return h;
}

String waitResponse() {                                           // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                                              // Переменная для хранения результата
  long _timeout = millis() + 10000;                               // Переменная для отслеживания таймаута (10 секунд)
  while (!modem.stream.available() && millis() < _timeout)  {};         // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (modem.stream.available()) {                                       // Если есть, что считывать...
    _resp = modem.stream.readString();                                  // ... считываем и запоминаем
  }
  else {                                                          // Если пришел таймаут, то...
    //SerialMon.println("Timeout...");                                 // ... оповещаем об этом и...
  }
  return _resp;                                                   // ... возвращаем результат. Пусто, если проблема
}

unsigned char HexSymbolToChar(char c) {
  if      ((c >= 0x30) && (c <= 0x39)) return (c - 0x30);
  else if ((c >= 'A') && (c <= 'F'))   return (c - 'A' + 10);
  else                                 return (0);
}

String UCS2ToString(String s) {                       // Функция декодирования UCS2 строки
  String result = "";
  unsigned char c[5] = "";                            // Массив для хранения результата
  for (int i = 0; i < s.length() - 3; i += 4) {       // Перебираем по 4 символа кодировки
    unsigned long code = (((unsigned int)HexSymbolToChar(s[i])) << 12) +    // Получаем UNICODE-код символа из HEX представления
                         (((unsigned int)HexSymbolToChar(s[i + 1])) << 8) +
                         (((unsigned int)HexSymbolToChar(s[i + 2])) << 4) +
                         ((unsigned int)HexSymbolToChar(s[i + 3]));
    if (code <= 0x7F) {                               // Теперь в соответствии с количеством байт формируем символ
      c[0] = (char)code;                              
      c[1] = 0;                                       // Не забываем про завершающий ноль
    } else if (code <= 0x7FF) {
      c[0] = (char)(0xC0 | (code >> 6));
      c[1] = (char)(0x80 | (code & 0x3F));
      c[2] = 0;
    } else if (code <= 0xFFFF) {
      c[0] = (char)(0xE0 | (code >> 12));
      c[1] = (char)(0x80 | ((code >> 6) & 0x3F));
      c[2] = (char)(0x80 | (code & 0x3F));
      c[3] = 0;
    } else if (code <= 0x1FFFFF) {
      c[0] = (char)(0xE0 | (code >> 18));
      c[1] = (char)(0xE0 | ((code >> 12) & 0x3F));
      c[2] = (char)(0x80 | ((code >> 6) & 0x3F));
      c[3] = (char)(0x80 | (code & 0x3F));
      c[4] = 0;
    }
    result += String((char*)c);                       // Добавляем полученный символ к результату
  }
  return (result);
}

float getFloatFromString(String str) {            // Функция извлечения цифр из сообщения - для парсинга баланса из USSD-запроса
  bool   flag     = false;
  String result   = "";
  str.replace(",", ".");                          // Если в качестве разделителя десятичных используется запятая - меняем её на точку.

  for (int i = 0; i < str.length(); i++) {
    if (isDigit(str[i]) || (str[i] == (char)46 && flag)) {        // Если начинается группа цифр (при этом, на точку без цифр не обращаем внимания),
      if (result == "" && i > 0 && (String)str[i - 1] == "-") {   // Нельзя забывать, что баланс может быть отрицательным
        result += "-";                            // Добавляем знак в начале
      }
      result += str[i];                           // начинаем собирать их вместе
      if (!flag) flag = true;                     // Выставляем флаг, который указывает на то, что сборка числа началась.
    }
    else  {                                       // Если цифры закончились и флаг говорит о том, что сборка уже была,
      if (str[i] != (char)32) {                   // Если порядок числа отделен пробелом - игнорируем его, иначе...
        if (flag) break;                          // ...считаем, что все.
      }
    }
  }

  return result.toFloat();                        // Возвращаем полученное число.
}

void save_set_string(String name, String value){
  //SerialMon.println("Save settings: " + name + " = " + value);
  if(preferences.begin("set-cat", false)){
    //SerialMon.println("Opened");
    //SerialMon.println(preferences.putString(name.c_str(), value));
    preferences.putString(name.c_str(), value);
    preferences.end();
    send_msg_01(rec_phone, name+":" + value);
  } else {
    //SerialMon.println("Error open settings");
  }
}

void save_set_bool(String name, boolean value){
  //SerialMon.println("Save settings: " + name + " = " + String(value));
  if(preferences.begin("set-cat", false)){
    preferences.putBool(name.c_str(), value);
    preferences.end();
    if(value){
      send_msg_01(rec_phone, name+":1");
    } else {
      send_msg_01(rec_phone, name+":0");
    }
  }
}

void save_set_uint(String name, uint64_t value){
  //SerialMon.print("Save settings: " + name + " = ");  SerialMon.println(value);
  if(preferences.begin("set-cat", false)){
    preferences.putLong64(name.c_str(), value);
    preferences.end();
    send_msg_01(rec_phone, name + ":" + uint64ToString(value));
  }
}

void save_set_int(String name, int value){
  //SerialMon.print("Save settings: " + name + " = ");  SerialMon.println(value);
  if(preferences.begin("set-cat", false)){
    preferences.putInt(name.c_str(), value);
    preferences.end();
    send_msg_01(rec_phone, name + ":" + String(value));
  }
}

uint64_t Get_last_ftp_file( ){
  uint64_t ret = 0;
  if(preferences.begin("set-cat", true)){
    ret = preferences.getULong64("last_ftp_fn", 0);
    preferences.end();
  }
  return ret;
}

void Start_save_ftp( ){
  uint64_t last = Get_last_ftp_file( ) + 1;
  uint64_t cnt;

  send_msg_01(rec_phone, "Start save to ftp...");

  while(true){
    String filename = "pic_" + uint64ToString(last) + ".jpg";
    if(!SendFile2FTP(filename, true)) return;
    cnt++;

    last++;
  }
  send_msg_01(rec_phone, uint64ToString(cnt) + "Files saved on ftp.");
}

void Send_set_bool(String name){
  if(preferences.begin("set-cat", true)){
    boolean v = preferences.getBool(name.c_str(), false);
    int vi = 0; if(v) vi = true;
    preferences.end();
    send_msg_01(rec_phone, name + ":" + String(vi));
  }  
}

void Send_set_string(String name){
  if(preferences.begin("set-cat", true)){
    String v = preferences.getString(name.c_str(), "");
    preferences.end();
    send_msg_01(rec_phone, name + ":" + v);
  }  
}

void Send_set_uint(String name){
  if(preferences.begin("set-cat", true)){
    uint64_t v = preferences.getULong64(name.c_str(), 0);
    preferences.end();
    send_msg_01(rec_phone, name + ":" + uint64ToString(v));
  }  
}

void Send_set_int(String name){
  if(preferences.begin("set-cat", true)){
    int v = preferences.getInt(name.c_str(), 0);
    preferences.end();
    send_msg_01(rec_phone, name + ":" + String(v));
  }  
}
void readAllSMS()
{
  if((millis() - lastUpdate > updatePeriod)) {
    while(true){
      int index = newMessageIndex(1);
      //SerialMon.println("SMS index = " + String(index));
      if(index  > 0){
        String SMS = readSMS(index);
        String ID = getSenderID(index);
        rec_phone = ID;

        //SerialMon.println("----------------------");
        //SerialMon.println("new SMS: " + SMS);
        //SerialMon.println("ID: " + ID);
        //SerialMon.println("----------------------");

//      получено SMS-сообщение
        boolean my = false;
        
        if(set_phone.indexOf(ID) >= 0){
          my = true;
          if(SMS == "NOFLASH") {
            Send_set_bool("no_flash");
          } else if(SMS.substring(0, 8) == "NOFLASH:") {
            if(SMS.substring(8) == "1") {
              set.no_flash = true;
              save_set_bool("no_flash", set.no_flash);
            }
            else if(SMS.substring(8) == "0") {
              set.no_flash = false;
              save_set_bool("no_flash", set.no_flash);
            }
          } 
          
          else if(SMS == "APN") {
            Send_set_string("APN");
          } else if(SMS.substring(0, 4) == "APN:") {
            set.APN = SMS.substring(4);
            save_set_string("APN", set.APN);
          } 
          
          else if(SMS == "APNU") {
            Send_set_string("APN_user");
          } else if(SMS.substring(0, 5) == "APNU:") {
            set.APN_user = SMS.substring(5);
            save_set_string("APN_user", set.APN_user);
          }
          
          else if(SMS == "APNP") {
            Send_set_string("APN_pwd");
          } else if(SMS.substring(0, 5) == "APNP:") {
            set.APN_pwd = SMS.substring(5);
            save_set_string("APN_pwd", set.APN_pwd);
          }

          else if(SMS == "NOFTP") {
            Send_set_bool("no_ftp");
          } else if(SMS.substring(0, 6) == "NOFTP:") {
            if(SMS.substring(6) == "1") {
              set.no_ftp = true;
              save_set_bool("no_ftp", set.no_ftp);
            }
            else if(SMS.substring(6) == "0") {
              set.no_ftp = false;
              save_set_bool("no_ftp", set.no_ftp);
            }
          }

          else if(SMS == "FTP") {
            Send_set_string("ftp");
          } else if(SMS.substring(0, 4) == "FTP:") {
            set.ftp = SMS.substring(4);
            save_set_string("ftp", set.ftp);
          }

          else if(SMS == "FTPU") {
            Send_set_string("ftp_user");
          } else if(SMS.substring(0, 5) == "FTPU:") {
            set.ftp_user = SMS.substring(5);
            save_set_string("ftp_user", set.ftp_user);
          }

          else if(SMS == "FTPP") {
              Send_set_string("ftp_pwd");
          } else if(SMS.substring(0, 5) == "FTPP:") {
            set.ftp_pwd = SMS.substring(5);
            save_set_string("ftp_pwd", set.ftp_pwd);
          }

          else if(SMS == "CPAO") {
            Send_set_uint("CPAO");
          } else if(SMS.substring(0, 5) == "CPAO:") {
            set.CPAO = SMS.substring(5).toInt();
            if(set.CPAO <= 1000) set.CPAO = 1000;
            save_set_uint("CPAO", set.CPAO);
          }

          else if(SMS == "SPHOTO") {
            Send_set_bool("start_photo");
          } else if(SMS.substring(0, 7) == "SPHOTO:") {
            if(SMS.substring(7) == "1") {
              set.start_photo = true;
              save_set_bool("start_photo", set.start_photo);
            }
            else if(SMS.substring(7) == "0") {
              set.start_photo = false;
              save_set_bool("start_photo", set.start_photo);
            }
          }

          else if(SMS == "P1") {
            Send_set_string("phone1");
          } else if(SMS.substring(0, 3) == "P1:") {
            set.phone1 = SMS.substring(3);
            save_set_string("phone1", set.phone1);
          }

          else if(SMS == "P2") {
            Send_set_string("phone2");
          } else if(SMS.substring(0, 3) == "P2:") {
            set.phone2 = SMS.substring(3);
            save_set_string("phone2", set.phone2);
          }

          else if(SMS == "P3") {
            Send_set_string("phone3");
          } else if(SMS.substring(0, 3) == "P3:") {
            set.phone3 = SMS.substring(3);
            save_set_string("phone3", set.phone3);
          }

          else if(SMS == "SMSL") {
            Send_set_int("sms_level");
          } else if(SMS.substring(0, 5) == "SMSL:") {
            set.sms_level = SMS.substring(5).toInt();
            save_set_int("sms_level", set.sms_level);
          }
        }

        if(ID == set.phone1 || ID == set.phone2 || ID == set.phone3){
          my = true;

          if(SMS == "reboot") {
            Reboot( );
          }
          else if(SMS == "start_ftp") {
            Start_save_ftp( );
          }
          else if(SMS == "balans"){
            sendUSSD("*100#");
          }
          else if(SMS == "photo") {
            //сделать фото
            Create_photo( );
          }
          else if(SMS == "delete") {
            Delete_photo( );
          }
          else if(SMS=="get_used") {
            uint32_t free_size = get_free_size();
            send_msg_01(rec_phone, "SD-card free: " + String(free_size) + "M");
          }
        }

        if(!my && ( ID.substring(0,3) == "MTC"  || ID.substring(0,3) == "MTS" )) {
          //SerialMon.println("МТС message: " + UCS2ToString(SMS));
        }

      } else {
        break;
      }
    }

    lastUpdate = millis();
  }

  if (modem.stream.available())   {                         // Если модем, что-то отправил...
    String _response = waitResponse();                       // Получаем ответ от модема для анализа
    _response.trim();                                 // Убираем лишние пробелы в начале и конце
    //SerialMon.println(_response);                        // Если нужно выводим в монитор порта
    if (_response.indexOf("+CMTI:")>-1) {             // Пришло сообщение об отправке SMS
      lastUpdate = millis() -  updatePeriod;          // Теперь нет необходимости обрабатываеть SMS здесь, достаточно просто
                                                      // сбросить счетчик автопроверки и в следующем цикле все будет обработано
    } else if(_response.startsWith("+CUSD:")) {
      String msg = _response.substring(_response.indexOf("\"") + 1);
      msg = msg.substring(0, msg.indexOf("\""));
      msg = UCS2ToString(msg);
      // пересылаем ответ на USSD
//SerialMon.println("{---------Это пересылка отета USSD---------");
      msg = (String)getFloatFromString(msg);
      if(msg.length() > 0) Send_msg(msg);
//SerialMon.println("}---------Это пересылка отета USSD---------");
    }
  }
}

bool setPowerBoostKeepOn(int en){
  I2CPower.beginTransmission(IP5306_ADDR);
  I2CPower.write(IP5306_REG_SYS_CTL0);
  if (en) {
    I2CPower.write(0x37);
  } else {
    I2CPower.write(0x35); 
  }
  return I2CPower.endTransmission() == 0;
}

void Read_sate_end(){
  cur_state.end.changed = false; 
  int state = cur_state.end.state;
  debouncer[0].update();
  cur_state.end.state = debouncer[0].read();
  if(cur_state.end.state != state || !first_read){
    cur_state.end.changed = true; 
  }
}

void Read_state_move(){
  cur_state.move.changed = false; 
  int state = cur_state.move.state;
  cur_state.move.state = digitalRead(cur_state.move.pin);
  if(cur_state.move.state != state || !first_read){
    cur_state.move.changed = true; 
  }
}

void Read_state( ){
  Read_sate_end();
  Read_state_move();
  first_read = true;  
};

static inline String TinyGsmDecodeHex8bit(String& instr) {
  String result;
  for (uint16_t i = 0; i < instr.length(); i += 2) {
    char buf[4] = {
        0,
    };
    buf[0] = instr[i];
    buf[1] = instr[i + 1];
    char b = strtol(buf, NULL, 16);
    result += b;
  }
  return result;
}

static inline String TinyGsmDecodeHex16bit(String& instr) {
  String result;
  for (uint16_t i = 0; i < instr.length(); i += 4) {
    char buf[4] = {
        0,
    };
    buf[0] = instr[i];
    buf[1] = instr[i + 1];
    char b = strtol(buf, NULL, 16);
    if (b) {  // If high byte is non-zero, we can't handle it ;(
#if defined(TINY_GSM_UNICODE_TO_HEX)
      result += "\\x";
      result += instr.substring(i, i + 4);
#else
      result += "?";
#endif
    } else {
      buf[0] = instr[i + 2];
      buf[1] = instr[i + 3];
      b      = strtol(buf, NULL, 16);
      result += b;
    }
  }
  return result;
}

void sendUSSD(const String& code) {
  //SerialMon.println("Send USSD: " + cur_state.sms_text);
  modem.sendAT(GF("+CMGF=1"));
  modem.waitResponse();
  modem.sendAT(GF("+CUSD=1,\""), code, GF("\""));
  modem.waitResponse();
}

void send_msg_01(String phone, String msg){
  if(phone.length() == 0) return;
  if(msg.length() == 0) return;
  //SerialMon.println("Send SMS to " + phone + " = " + msg);
  
  if(set.sms_level != 1) return;

  if(modem.sendSMS(phone, msg)){
    //SerialMon.println("Sended sms to " + phone + ": " + msg);
  }
  else{
    //SerialMon.println("SMS to " + phone + " failed to send");
  }
}

void Send_msg(String msg) {
  if(set.phone1.length() == 12){
    send_msg_01(set.phone1, msg);
  }
  if(set.phone2.length() == 12){
    send_msg_01(set.phone2, msg);
  }
}

void loop() {
  
  boolean me_started = false;

  rec_phone = "";

  if(!started){
    if(millis( ) - started_time > Interval_started){
      started = true;
    }
  } else {
    if(!started1) {
      started1 = true;
      Last_Working = millis();
      me_started = true;
      LedOn(); delay(200);
      LedOff(); delay(200);
      LedOn(); delay(200);
      LedOff(); delay(200);
      Read_state( );
      if(cur_state.end.state == HIGH){
        Send_msg("device started. opened.");
      }
      else{
        Send_msg("device started. closed.");
      }
    }
  }
  if(started1){
    Read_state( );

    if(millis( ) - Last_Working > Interval_working ) 
    {
      Send_msg("device working");
      Last_Working = millis( );
    }

    uint64_t t = 0;
    boolean f=false;
    if(cur_state.end.changed) {
      if(cur_state.end.state == HIGH){
        if(!me_started){
          if(set.start_photo) {
            t = set.CPAO;
            f = true;
          }
        }
        Send_msg("opened");
      } else {
        Send_msg("closed");
      }
    } else //if(cur_state.end.state == HIGH) 
    {
      if(cur_state.move.changed && cur_state.move.state == HIGH){
        //SerialMon.println("Есть движение.");
        if(!me_started){
          if(set.start_photo) {
            f  = true;
            t = 0;
          }
        }
      }
    }

    if(save_interval > 0) {
      if(millis() - interval_start > save_interval) {
        save_interval = 0;
        Create_photo( );
      }
    } else if(f){
      save_interval = t;
      interval_start = millis();
      if(save_interval > 0){
        //SerialMon.println("Сделать фото потом");
      } else {
        Create_photo( );
      }
    }
  }

  readAllSMS();

  esp_task_wdt_reset();
}

void LedOn(){
  if(set.no_flash) return;
  // Включаем все светодиоды
  for (int i = 0; i < LED_COUNT; i++)
  {
    strip[i] = CRGB::White;
  }
  FastLED.show();  
}

void LedOff(){
  // Включаем все светодиоды
  for (int i = 0; i < LED_COUNT; i++)
  {
    strip[i] = CRGB::Black;
  }
  FastLED.show();  
}

void Delete_photo( ){
  if(photo_creating) return;

  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://" + esp32_cap_ip + "/del?deloper=1");
    int httpCode = http.GET();
    if(httpCode > 0) {
      SaveLastNum(0);
      String payload = http.getString();
      //SerialMon.println(httpCode);
      //SerialMon.println(payload);
      send_msg_01(rec_phone, "Deleted");
    }
    else {
      //SerialMon.println("Ошибка HTTP-запроса");
      send_msg_01(rec_phone, "Error");
    }
    // освобождаем ресурсы микроконтроллера
    http.end();
  } else {
    ESP.restart();
  }
}

uint64_t GetNumFromFile(String filename){
  uint64_t ret;
  int i = filename.indexOf("pic_")+4;
  String f = filename.substring(i);
  i = f.indexOf(".jpg");
  f = f.substring(0, i);
  ret = f.toInt();
  return ret;
}

boolean SendFile2FTP(String filename, boolean save) {
  boolean ret = false;

  //SerialMon.println("filename = " + filename);

  if(set.ftp.length() == 0 || set.APN.length() == 0) return false;
  if(set.no_ftp && !save) return false;

  //download file from esp32cam
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://" + esp32_cap_ip + "/download?download=/" + filename);
    int httpCode = http.GET();
    if(httpCode == 200) {
      int len = http.getSize();
      //SerialMon.println("len: " + String(len));
      uint8_t buff[128] = { 0 };
      WiFiClient * stream = http.getStreamPtr();
      File file = SPIFFS.open("/pic_ftp.jpg", FILE_WRITE);
      if(!file){
        // File not found
        //SerialMon.println("Failed to open file");
        return false;
      }

      while(http.connected() && (len > 0 || len == -1)) {
        // get available data size
        size_t size = stream->available();
        if(size) {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          // write it to Serial
          file.write(buff, c);
          if(len > 0) {
            len -= c;
          }
        }
        delay(1);
      }      
      file.close();
      //SerialMon.println("file saved to SPIFFS.");
      uint64_t ui64 = GetNumFromFile(filename);
      ret = gprs_modem_function(filename, "/pic_ftp.jpg", ui64);
    } else {
      //SerialMon.println("Ошибка HTTP-запроса");
    }
    // освобождаем ресурсы микроконтроллера
    http.end();
  } else {
    //ESP.restart();
  }
  return ret;
}

void Create_photo(){
  if(photo_creating) return;

  //SerialMon.println("Делаем фото...");

  String filename;

  photo_creating = true;

  int f = 0;

  if ((WiFi.status() == WL_CONNECTED)) {

    LedOn();
    delay(1000);

    HTTPClient http;
    http.begin("http://" + esp32_cap_ip + "/capture");
    int httpCode = http.GET();
    if(httpCode > 0) {
      String payload = http.getString();
      //SerialMon.println(httpCode);
      //SerialMon.println(payload);
      int i = payload.indexOf("/pic_") + 1;
      if(i>=0){
        payload = payload.substring(i);
        i = payload.indexOf(".jpg");
        if(i>0){
          filename = payload.substring(0, i+4);
        }
        f = f + 1;
      }
    }
    else {
      //SerialMon.println("Ошибка HTTP-запроса");
    }
    // освобождаем ресурсы микроконтроллера
    http.end();

    delay(1000);

    LedOff();

    if(f == 0) { 
      send_msg_01(rec_phone, "Error");
      return;
    }

    uint32_t free_size = get_free_size();
    if( free_size < 500) {
      Send_msg("SD-card free: " + String(free_size) + "M");
    }

    if(filename.length()>0){
      if(SendFile2FTP(filename)){
        f = f + 2;
      }
    }
  } else {
    ESP.restart();
  }

  String msg = "";
  if(( f & 1 ) == 1){
    msg = "Photo created.";
  }

  if(!set.no_ftp) {
    if((f & 2) == 2) {
      msg = msg + " Photo saved on ftp.";
    } else {
      msg = msg + " Error saved on ftp.";
    }
  }
  send_msg_01(rec_phone, msg);

  photo_creating = false;
  save_interval = 0;
}

uint32_t get_free_size(){
  uint32_t ret = 0;

  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://" + esp32_cap_ip + "/");
    int httpCode = http.GET();
    if(httpCode > 0) {
      String payload = http.getString();
      //SerialMon.println(httpCode);
      //SerialMon.println(payload);
      if(httpCode == 200) {
        int i = payload.indexOf("[Free]");
        if(i>=0){
          i = i +  6;
          payload = payload.substring(i);
          i = payload.indexOf(")");
          if(i>0) {
            payload = payload.substring(0, i);
            ret = payload.toInt();
          }
        }
      }
    }
    else {
      //SerialMon.println("Ошибка HTTP-запроса");
    }
    // освобождаем ресурсы микроконтроллера
    http.end();
  } else {
    ESP.restart();
  }
  return ret;
}

void Reboot( ){
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://" + esp32_cap_ip + "/reboot");
    http.GET();
  }
  ESP.restart();
}

byte sendATcommand(String ATcommand, String answer1, String answer2, unsigned int timeout){
  byte reply = 1;
  String content = "";
  char character;
  //Clean the modem input buffer
  while(SerialAT.available()>0) SerialAT.read();

  //SerialMon.println(ATcommand);

  //Send the atcommand to the modem
  SerialAT.println(ATcommand);
  delay(100);
  unsigned int timeprevious = millis();
  while((reply == 1) && ((millis() - timeprevious) < timeout)){
    while(SerialAT.available()>0) {
      character = SerialAT.read();
      content.concat(character);
      //SerialMon.print(character);
      delay(10);
    }
    //Stop reading conditions
    if (content.indexOf(answer1) != -1){
      reply = 0;
    }else if(content.indexOf(answer2) != -1){
      reply = 2;
    }else{
      //Nothing to do...
    }
  }
  return reply;
}

void SaveLastNum(uint64_t last){
  if(preferences.begin("set-cat", false)){
    preferences.putULong64("last_ftp_fn", last);
    //SerialMon.println("Last_file_num: " + uint64ToString(last));
    preferences.end();
  }
}

uint64_t ReadLastNum(uint64_t last){
  uint64_t ret = 0;
  if(preferences.begin("set-cat", true)){
    ret = preferences.getLong64("last_ftp_fn", 0);
    preferences.end();
  }
  return ret;
}

boolean gprs_modem_function(String filename, String fileSPIFF, uint64_t file_num){
  byte reply = 1;
  int i = 0;
  char char_buffer;
  String string_buffer = "";
  int buffer_space = 1000;

  boolean sended=false;

  File fileData = SPIFFS.open(fileSPIFF, FILE_READ);
  if(!fileData){
    return sended;
  }
  //SerialMon.println("File size: " + String(fileData.size()));

  //SerialMon.println("Start sending file: " + filename + "..." );

  while (i < 10 && reply == 1){ //Try 10 times...
    reply = sendATcommand("AT+CREG?","+CREG: 0,1","ERROR", 1000);
    i++;
    delay(1000);
  }
  if (reply == 0){
    reply = sendATcommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK","ERROR", 1000);
    if (reply == 0){
      reply = sendATcommand("AT+SAPBR=3,1,\"APN\",\"" + set.APN + "\"", "OK", "ERROR", 1000);
      if (reply == 0){
        reply = sendATcommand("AT+SAPBR=3,1,\"USER\",\"" + set.APN_user + "\"", "OK", "ERROR", 1000);
        if (reply == 0){
          reply = sendATcommand("AT+SAPBR=3,1,\"PWD\",\"" + set.APN_pwd + "\"", "OK", "ERROR", 1000);
          if (reply == 0){
            reply = 2;
            i = 0;
            while (i < 3 && reply == 2){ //Try 3 times...
              reply = sendATcommand("AT+SAPBR=1,1", "OK", "ERROR", 10000);
              if (reply == 2){
                sendATcommand("AT+SAPBR=0,1", "OK", "ERROR", 10000);
              }
              i++;
            }
            if (reply == 0){
              reply = sendATcommand("AT+SAPBR=2,1", "OK", "ERROR", 1000);
             if (reply == 0){
                reply = sendATcommand("AT+FTPCID=1", "OK", "ERROR", 1000);
               if (reply == 0){
                reply = sendATcommand("AT+FTPMODE=1", "OK", "ERROR", 1000);  // Пассивный режим (1) Активный режим (0)
                 if (reply == 0){
                  reply = sendATcommand("AT+FTPSERV=\"" + set.ftp + "\"", "OK", "ERROR", 1000);  //сервер фтп, можно как айпи указать так и адрес типа ftp.tra-ta-ta.com
                  if (reply == 0){
                    reply = sendATcommand("AT+FTPPORT=21", "OK", "ERROR", 1000);
                    if (reply == 0){
                      reply = sendATcommand("AT+FTPUN=\"" + set.ftp_user + "\"", "OK", "ERROR", 1000);  // Логин Ford
                      if (reply == 0){
                        reply = sendATcommand("AT+FTPPW=\"" + set.ftp_pwd + "\"", "OK", "ERROR", 1000); // Пароль Jastin
                        if (reply == 0){
                          reply = sendATcommand("AT+FTPPUTNAME=\"" + filename + "\"", "OK", "ERROR", 1000);
                          if (reply == 0){
                            reply = sendATcommand("AT+FTPPUTPATH=\"/\"", "OK", "ERROR", 1000);
                            if (reply == 0){
                              unsigned int ptime = millis();
                              reply = sendATcommand("AT+FTPPUT=1", "+FTPPUT: 1,1", "+FTPPUT: 1,6", 60000);
                              //SerialMon.println("Time: " + String(millis() - ptime));
                              if (reply == 0){
                                if (fileData) {
                                  int i = 0;
                                  //SerialMon.println("Start sended...");
                                  while (fileData.available()>0) {
                                    char_buffer = fileData.read();
                                    string_buffer.concat(char_buffer);
                                    i++;
                                    if (i == buffer_space) {
                                      sendATcommand("AT+FTPPUT=2," + String(buffer_space), "AT+FTPPUT=2,10", "ERROR", 1000);
                                      sendATcommand(string_buffer, "OK", "ERROR", 5000);
                                      string_buffer = "";
                                      i = 0;
                                    }
                                  }
                                  if (string_buffer != ""){
                                    sendATcommand("AT+FTPPUT=2," + String(i), "AT+FTPPUT=2,10", "ERROR", 1000);
                                    sendATcommand(string_buffer, "OK", "ERROR", 5000);
                                    sendATcommand("AT+FTPPUT=2,0", "OK", "ERROR", 1000);
                                  }
                                  //SerialMon.println("End sended...");
                                  SaveLastNum(file_num);

                                  sended = true;
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  fileData.close();

  if(!sended && !photo_not_sended) {
    photo_not_sended = true;
    Send_msg("Error send photo to ftp-server");
  }

  return sended;
}

String uint64ToString(uint64_t input) {
  String result = "";
  uint8_t base = 10;
  
  do {
    char c = input % base;
    input /= base;
    
    if (c < 10)
    c +='0';
    else
    c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

