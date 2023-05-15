#include <BH1750FVI.h>
#include <mqtt_ini.h>

#define def_path "home/bed/sens"

#define PIN_MOVESENS          D5
#define INTERVAL_CHECK_BH1750FVI  1000

uint32_t LAST_CHECK_BH1750FVI = 0;
uint32_t LAST_MOVESENS = 0;

struct s_bh1750fvi{
  uint16_t lux;
};

struct s_movesens{
  int state;
};

struct s_state{
  s_bh1750fvi bh1750fvi;
  s_movesens movesens;
};

s_state cur_state;

void  report( s_state sState, int mode );

uint8_t ADDRESSPIN = 13;
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI bh1750fvi(ADDRESSPIN, DEVICEADDRESS, DEVICEMODE);

mqtt_ini client( 
  "ESP8266_BED_SENS",     // Client name that uniquely identify your device
   def_path);

void setup() { 
  Serial.begin(115200);

  cur_state.movesens.state = LOW;
  pinMode(PIN_MOVESENS, INPUT); 

  bh1750fvi.begin(); 

  client.begin();
}

void onMsgCommand( const String &message ){
}

void onConnection()
{
}

s_bh1750fvi Read_bh1750fvi( ){
  s_bh1750fvi ret;
  ret = cur_state.bh1750fvi;

  if(millis() - LAST_CHECK_BH1750FVI > INTERVAL_CHECK_BH1750FVI ){ 
    ret.lux = bh1750fvi.GetLightIntensity();
    LAST_CHECK_BH1750FVI = millis();
  }
  return ret;
}

s_state Read_movesens( s_state sState){
  s_state ret = sState;

  ret.movesens.state = digitalRead(PIN_MOVESENS);

  return ret;
}

s_state Read_state( ){
  s_state ret = cur_state;
  ret.bh1750fvi = Read_bh1750fvi();
  ret = Read_movesens(ret);  
  return ret;
}

void  report( s_state sState, int mode ){

  if(mode == 0 || ( mode == 1 && sState.bh1750fvi.lux != cur_state.bh1750fvi.lux ) ){
    client.Publish("BH1750FVI/lux", String(sState.bh1750fvi.lux));
  }

  if(mode == 0 || ( mode == 1 && sState.movesens.state != cur_state.movesens.state ) ){
    client.Publish("movesens/state", String(sState.movesens.state));
  }

  cur_state = sState;

  client.flag_start = false;
}

void OnCheckState(){
  s_state state = Read_state( );

  if(client.flag_start){ //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void OnLoad(){}

void loop() { 
  client.loop();
}