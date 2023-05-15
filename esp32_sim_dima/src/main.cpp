#include <Arduino.h>
#include <esp32_sim800L.h>
#include <Bounce2.h>

#define PIN_WATCHDOG_IN 19
#define PIN_WATCHDOG_OUT 23

HardwareSerial SIM800(2);
//                               RX  TX
esp32_sim800l sim(&SIM800, 9600, 16, 17, "+79162903383;+79252066179", 60000, true);

#define PIN_END   18

#define Interval_working 43200000
uint32_t Last_Working;

#define Interval_started 60000
uint32_t started_time;

boolean started;
boolean started1;
boolean first_read = false;

String sendUSSD="";
uint32_t sendUSSD_t;

String answUSSD="";
uint32_t answUSSD_t;

struct s_end{
  int pin;
  int state;
  boolean changed;
};

struct s_state{
  s_end end;
};

s_state cur_state;

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  cur_state.end.pin = PIN_END; pinMode(cur_state.end.pin, INPUT);
  debouncer.attach(cur_state.end.pin);
  debouncer.interval(5);
  
  pinMode(PIN_WATCHDOG_IN, INPUT);
  pinMode(PIN_WATCHDOG_OUT, OUTPUT); digitalWrite(PIN_WATCHDOG_OUT, LOW);

  sim.begin( );
  sim.delSMS( );

  started_time = millis( );
}

void Read_state( ){
  cur_state.end.changed = false; 
  int state = cur_state.end.state;
  debouncer.update();
  cur_state.end.state = debouncer.read();
  if(cur_state.end.state != state || !first_read){
    cur_state.end.changed = true; 
  }
  first_read = true;
};

void Send_msg(String msg) {
  Serial.println(MISC_REG_2);
  sim.sendSMSinPDU("+79162903383", msg);
  sim.sendSMSinPDU("+79252066179", msg);
}

void loop() {
 //выдаем то, что нам выставил сторожевой таймер
  int wdt_in = digitalRead(PIN_WATCHDOG_IN);
  int wdt_out = digitalRead(PIN_WATCHDOG_OUT);
  if(wdt_out != wdt_in){
    digitalWrite(PIN_WATCHDOG_OUT, wdt_in);
  }

  if(!started){
    if(millis( ) - started_time > Interval_started){
      started = true;
    }
  } else {
    if(!started1) {
      Send_msg("Сторож стартанул.");
      started1 = true;
      Last_Working = millis();
    }
  }
  if(started1){
    Read_state( );

    if(millis( ) - Last_Working > Interval_working ) 
    {
      Send_msg("Сторож работает.");
      Last_Working = millis( );
    }

    if(cur_state.end.changed) {
      if(cur_state.end.state == HIGH){
        Send_msg("Щиток открыт");
      } else {
        Send_msg("Щиток закрыт");
      }
    }
  }

  if(sendUSSD.length() > 0 && (millis() - sendUSSD_t) > 2000){
    sim.sendUSSD(sendUSSD);
    sendUSSD = "";
  }

  if(answUSSD.length() > 0 && (millis() - answUSSD_t) > 2000){
    Send_msg(answUSSD);
    answUSSD = "";
  }

  sim.loop();
}

void NewMessage(String msgbody, String msgphone ){
  Serial.println("Get: " + msgphone + "=" + msgbody);
  if (sim.is_MyPhone(msgphone)) { // Если телефон в белом списке, то...
    if(msgbody.indexOf("баланс") > -1){
      sendUSSD = "*100#";
      sendUSSD_t = millis();
    } else {
    }
  } else {
  }
}

void USSD_ANSWER( String msg ){
  Serial.println("USSD_ANSWER: " + msg);
  answUSSD = msg;
  answUSSD_t = millis();
}

void pub_response( String response ){
  //Serial.println("command: " + response);
}

void ParseSms_01( String msg ){
  //Serial.println("source_sms: " + msg);
}
