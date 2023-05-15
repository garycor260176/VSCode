#include <mqtt_ini.h>
#include <esp32_sim800L.h>

#define def_path "home/SIM"

mqtt_ini client( 
  "ESP32_SIM",     // Client name that uniquely identify your device
   def_path);

HardwareSerial SIM800(1);
esp32_sim800l sim(&SIM800, 9600, 4, 2, "+79162903383", 60000, true);

//uint32_t last_Check_gprs;
//#define interval_Check_gprs  600000

/*
uint32_t last_Check_dt;
#define interval_get_dt  10000
*/

struct s_state{
  String sms_text = "";
  String phone = "";
  int proc = 0;
};

s_state cur_state; //предыдущее отправленное состояние 

void report( s_state, int, unsigned long);

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("Start!");

  client.begin();
  sim.begin( );
  //sim.gprsConnect("internet.mts.ru", "mts", "mts");
  //sim.SetAutoTimeSync();
  //last_Check_gprs = millis( );
}

void onMsgCommand( const String &message ){}

void onMsgSendSMS( const String &message ){
  cur_state.sms_text = cur_state.phone = "";

  if(message.length()==0) return;

  int i = message.indexOf('=');
  if(i < 0 ) return;

  cur_state.phone = message.substring(0, i);
  cur_state.sms_text = message.substring(i+1);
  cur_state.proc = 0;
}

void onMsgSendSMS_PDU( const String &message ){
  cur_state.sms_text = cur_state.phone = "";

  if(message.length()==0) return;

  int i = message.indexOf('=');
  if(i < 0 ) return;

  cur_state.phone = message.substring(0, i);
  cur_state.sms_text = message.substring(i+1);
  cur_state.proc = 1;
}

void onMsgSendUSSD( const String &message ){
  cur_state.sms_text = cur_state.phone = "";

  if(message.length()==0) return;

  cur_state.sms_text = message;
  cur_state.proc = 2;
}

void report( s_state sState1, int mode, unsigned long flagSend ){
  s_state sState = sState1;
  
  unsigned long flagS = flagSend;
  if(flagS == 0) {

  }

  cur_state = sState;

  client.flag_start = false;
}

s_state Read_state( ){
  s_state ret = cur_state;
  
  return ret;
};

void OnCheckState(){
  s_state state = Read_state( );


    if(cur_state.sms_text.length()>0){
      switch(state.proc){
        case 1:
          if(cur_state.phone.length()>0){
            sim.sendSMSinPDU(state.phone, state.sms_text);
            client.Publish("SEND_SMS_PDU", "");
          }
          break;
        case 2:
          sim.sendUSSD(state.sms_text);
          client.Publish("SEND_SMS_USSD", "");
          break;
        default:
          if(cur_state.phone.length()>0){
            sim.sendSMS(state.phone, state.sms_text);
            client.Publish("SEND_SMS", "");
          }
          break;
      }
      state.sms_text = state.phone = "";
    }

  if(client.flag_start){ //первый запуск
    report(state, 0, 0); //отправляем все
  } else {
    report(state, 1, 0); //отправляем только то, что изменилось
  }
}

void onConnection(){
  client.Subscribe("SEND_SMS", onMsgSendSMS);
  client.Subscribe("SEND_SMS_PDU", onMsgSendSMS_PDU);
  client.Subscribe("SEND_SMS_USSD", onMsgSendUSSD);
}

void OnLoad(){
}

void loop() {
  sim.loop();

  client.loop();

/*
  if(millis() - last_Check_gprs > interval_Check_gprs){
    if(!sim.IsGrpsConnected()){
      sim.gprsConnect("internet.mts.ru", "mts", "mts");
    }
    last_Check_gprs = millis( );
  }

  if(millis() - last_Check_dt > interval_get_dt ){
    stru_datetime dt = sim.GetTime( );
    if(dt.year != 0) {
      Serial.println(String(dt.year)+"_"+String(dt.month)+"_"+String(dt.day)+"_"+String(dt.hours)+"_"+String(dt.minutes)+"_"+String(dt.seconds));
    }
    last_Check_dt = millis( );
  }
*/
}

void NewMessage(String msgbody, String msgphone ){
  Serial.println("Get: " + msgphone + "=" + msgbody);
  client.Publish("input_sms", msgphone + "=" + msgbody);
  /*if (sim.is_MyPhone(msgphone)) { // Если телефон в белом списке, то...
    client.Publish("legal_input_sms", msgphone + "=" + msgbody);
  }*/
}

void ParseSms_01( String msg ){
  client.Publish("source_sms", msg);
}

void USSD_ANSWER( String msg ){
  client.Publish("USSD_ANSWER", msg);
}

void pub_response( String response ){
  //client.Publish("modem_response", response);
}