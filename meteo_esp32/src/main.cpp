#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750FVI.h>
#include <Adafruit_Sensor.h>
#include <iarduino_Pressure_BMP.h>
#include <mqtt_ini.h>

#define def_path "outside/meteo"

#define DHTPIN          19
#define DS18B20PIN      18
#define PIN_RG11        14  
#define PIN_WILD_SPEED  34   
#define PIN_WILD_DIRECT 35

#define PIN_WATCHDOG_IN 5
#define PIN_WATCHDOG_OUT 4

uint32_t LastRead_bmp280 = 0; //запоминаем время последней публикации
uint32_t LastRead_bh1750fvi = 0; //запоминаем время последней публикации
uint32_t LastRead_WILD = 0; //запоминаем время последней публикации
uint32_t LastRead_DS18B20 = 0; //запоминаем время последней публикации
uint32_t LastRead_DHT22 = 0; //запоминаем время последней публикации
boolean ds18b20_readed = false;

uint32_t DS18b20_Start = 0;
boolean DS18b20_Started = false;

uint32_t disconnect_start = 0; 
boolean f_disconnect_start = false;
#define disconnect_time_reload 60000 //если в течении 60ти секунд нет связи

#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

OneWire oneWire(DS18B20PIN);
DallasTemperature DS18B20(&oneWire);

BH1750FVI bh1750fvi(BH1750FVI::k_DevModeContLowRes);

iarduino_Pressure_BMP bmp(0x76);

mqtt_ini client( 
  "ESP32_METEO",     // Client name that uniquely identify your device
   def_path);

//датчик дождя
//boolean Flag_Rain = false; //признак что сработало прерывание
unsigned long ContactTime;// Timer to manage any contact bounce in interrupt routine 
//#define IntervalRainData 180000 //интервал обновления состояния датчика дождя в сек (если в течении этого времени датчик дождя не срабатывал, то считаем, что дождь кончился)
//uint32_t LastRain;// Timer to manage any contact bounce in interrupt routine 

struct s_bmp280{
  float bmpTemperature = 0; 
  float bmpPressure = 0;
  float bmpAltitude = 0;

  String Pressure;
  String Temperature;
  String Altitude;
};

struct s_dht22{
  float f_h;
  float f_t;

  String s_h;
  String s_t;
};

struct s_ds18b20{
  float f_t;
  String s_t;
};

struct s_bh1750fvi{
  uint16_t lux;
};

struct s_RG11{
  int rain;
};

struct s_wild{
  float speed_voltage;
  float speed;
  String s_speed;
  
  float direct_voltage;
  String s_direct;
};

struct s_state{
  uint32_t interval_DHT22;
  uint32_t interval_DS18B20;
  uint32_t interval_WILD;
  uint32_t interval_bh1750fvi;
  uint32_t interval_bmp280;

  s_RG11 rg11;
  s_bh1750fvi bh1750fvi;
  s_ds18b20 ds18b20;
  s_dht22 dht22;
  s_wild wild;
  s_bmp280 bmp280;
};

s_state cur_state;

void report( s_state, int);
boolean PubInterval(String topic, uint32_t val);
s_wild Read_Wild();

/*
//ICACHE_RAM_ATTR 
void IRAM_ATTR rgisr() { 
  if((millis() - ContactTime) > 70 ) { // debounce of sensor signal 
    Flag_Rain = true;
    ContactTime = millis(); 
  } 
} 
*/

void setup() {
  Serial.begin(115200);

  cur_state.interval_DHT22 = 5000;
  cur_state.interval_DS18B20 = 5000;
  cur_state.interval_WILD = 1000;
  cur_state.interval_bh1750fvi = 1000;
  cur_state.interval_bmp280 = 300000;

  dht.begin();
  DS18B20.begin();
  bh1750fvi.begin(); 
  bmp.begin(118);

  pinMode(PIN_WATCHDOG_IN, INPUT);
  pinMode(PIN_WATCHDOG_OUT, OUTPUT); digitalWrite(PIN_WATCHDOG_OUT, LOW);

//датчик дождя
  //Flag_Rain = false; 
  //LastRain = 0;      
  pinMode(PIN_RG11, INPUT); 
  //attachInterrupt(PIN_RG11, rgisr, FALLING); 

  client.begin();
  delay(200);
}

void Msg_interval_DHT22( const String &message ){
  Serial.println("Msg_interval_DHT22: " + message);

  if(message.length() > 0){
    cur_state.interval_DHT22 = message.toInt() * 1000;
    if(cur_state.interval_DHT22 <= 1000){
      cur_state.interval_DHT22 = 1000;
    }
    PubInterval("dht22/interval", cur_state.interval_DHT22);
  }
}

void Msg_interval_DS18B20( const String &message ){
  Serial.println("Msg_interval_DS18B20: " + message);

  if(message.length() > 0){
    cur_state.interval_DS18B20 = message.toInt() * 1000;
    if(cur_state.interval_DS18B20 <= 1000){
      cur_state.interval_DS18B20 = 1000;
    }
    PubInterval("ds18b20/interval", cur_state.interval_DS18B20);
  }
}

void Msg_interval_bh1750fvi( const String &message ){
  Serial.println("Msg_interval_bh1750fvi: " + message);

  if(message.length() > 0){
    cur_state.interval_bh1750fvi = message.toInt() * 1000;
    if(cur_state.interval_bh1750fvi <= 1000){
      cur_state.interval_bh1750fvi = 1000;
    }
    PubInterval("BH1750FVI/interval", cur_state.interval_bh1750fvi);
  }
}

void Msg_interval_bmp280( const String &message ){
  Serial.println("Msg_interval_bmp280: " + message);

  if(message.length() > 0){
    cur_state.interval_bmp280 = message.toInt() * 1000;
    if(cur_state.interval_bmp280 <= 1000){
      cur_state.interval_bmp280 = 1000;
    }
    PubInterval("bmp280/interval", cur_state.interval_bmp280);
  }
}

void Msg_interval_WILD( const String &message ){
  Serial.println("Msg_interval_WILD: " + message);

  if(message.length() > 0){
    cur_state.interval_WILD = message.toInt() * 1000;
    if(cur_state.interval_WILD <= 1000){
      cur_state.interval_WILD = 1000;
    }
    PubInterval("wild/interval", cur_state.interval_WILD);
  }
}

void onMsgCommand( const String &message ){}

void onConnection()
{
  client.Subscribe("Constants/outside/meteo/interval_DHT22", Msg_interval_DHT22, true); 
  client.Subscribe("Constants/outside/meteo/interval_DS18B20", Msg_interval_DS18B20, true); 
  client.Subscribe("Constants/outside/meteo/interval_WILD", Msg_interval_WILD, true); 
  client.Subscribe("Constants/outside/meteo/interval_bh1750fvi", Msg_interval_bh1750fvi, true); 
  client.Subscribe("Constants/outside/meteo/interval_bmp280", Msg_interval_bmp280, true); 
}

s_dht22 Read_DHT22(){
  s_dht22 ret;
  
  delay(200);
  ret.f_t = dht.readTemperature();
  ret.f_h = dht.readHumidity();
  if (isnan(ret.f_h) || isnan(ret.f_t)) { //при сбое вернем предыдущее значение
    ret.f_t = cur_state.dht22.f_t;
    ret.f_h = cur_state.dht22.f_h;
    ret.s_t = cur_state.dht22.s_t;
    ret.s_h = cur_state.dht22.s_h;
  } else {
    ret.s_t = String(ret.f_t);
    ret.s_h = String(ret.f_h);
  }
  return ret;
}

s_ds18b20 Read_ds18b20(){
  s_ds18b20 ret;

  DS18B20.requestTemperatures();
  ret.f_t = DS18B20.getTempCByIndex(0);

//в первй раз пропускаем. почему-то всегда выдает 25градусов
  if(DS18b20_Started){
    if (isnan(ret.f_t) || ret.f_t == -127 ) { //при сбое вернем предыдущее значение
      ret.f_t = cur_state.ds18b20.f_t;
      ret.s_t = cur_state.ds18b20.s_t;
    } else {
      ret.s_t = String(ret.f_t);
      ds18b20_readed = true;
    }
  } else {
    DS18b20_Started = true;
  }

  return ret;
}

s_bh1750fvi Read_bh1750fvi( ){
  s_bh1750fvi ret = cur_state.bh1750fvi;
  
  ret.lux = bh1750fvi.GetLightIntensity();
  if(ret.lux == 54612 or ret.lux == 27519) {
    ret.lux = cur_state.bh1750fvi.lux;
  }
  return ret;
}

s_RG11 Read_RG11(){
  s_RG11 ret = cur_state.rg11;

static int old_state;

  int state = digitalRead(PIN_RG11);

  if(old_state != state){
    Serial.println(String(state));
  }
  old_state = state;


  if(state == LOW) { //датчик сработал
    ret.rain = 1; //идет дождь
    ContactTime = millis(); //запоминаем время последнего срабатывания

  } else { //датчик сбросился
    if(cur_state.rg11.rain == 1) { //уже взводился
      if(millis() - ContactTime >= 60000) { //последний раз взводился больше 60сек
        ret.rain = 0;
      }
    }
  }
/*
  if(Flag_Rain) { //сработало прерывание
    Flag_Rain = false; //сбрасываем признак срабатывания прерывания
    LastRain = millis(); //запоминаем время
    ret.rain = 1;
  } else {
    if(cur_state.rg11.rain != 0){ //признак дождя выставлен
      if(millis() - LastRain > IntervalRainData ){ //сколько прошло времени с момента последнего прерывания
        ret.rain = 0;
      }
    }
  }
*/
  return ret;
}

s_bmp280 Read_bmp280( ){
  s_bmp280 ret = cur_state.bmp280;

  if(bmp.read(1)) {
    ret.bmpTemperature = bmp.temperature;
    ret.bmpPressure = bmp.pressure;
    ret.bmpAltitude = bmp.altitude;
  }
  if(ret.bmpPressure<=0) ret.bmpPressure = cur_state.bmp280.bmpPressure;
  ret.Pressure = String(ret.bmpPressure);
  ret.Temperature = String(ret.bmpTemperature);
  ret.Altitude = String(ret.bmpAltitude);
  return ret;
}

s_state Read_state( ){
  s_state ret = cur_state;

  if(client.flag_start || (millis( ) - LastRead_DHT22 > cur_state.interval_DHT22 && cur_state.interval_DHT22 >= 0 ) ) {
    ret.dht22 = Read_DHT22( );
    LastRead_DHT22 = millis();
  }
  if(client.flag_start || (millis( ) - LastRead_DS18B20 > cur_state.interval_DS18B20 && cur_state.interval_DS18B20 >= 0 ) ) {
    ret.ds18b20 = Read_ds18b20( );
    LastRead_DS18B20 = millis();
  }
  if(client.flag_start || (millis( ) - LastRead_bh1750fvi > cur_state.interval_bh1750fvi && cur_state.interval_bh1750fvi >= 0 ) ) {
    ret.bh1750fvi = Read_bh1750fvi( );
    LastRead_bh1750fvi = millis();
  }
  if(client.flag_start || (millis( ) - LastRead_bmp280 > cur_state.interval_bmp280 && cur_state.interval_bmp280 >= 0 ) ) {
    ret.bmp280 = Read_bmp280( );
    LastRead_bmp280 = millis();
  }

  ret.rg11 = Read_RG11( );

  if(client.flag_start || (millis( ) - LastRead_WILD > cur_state.interval_WILD && cur_state.interval_WILD >= 0 ) ) {
    ret.wild = Read_Wild( );
    LastRead_WILD = millis();
  }
  return ret;
};

s_wild Read_Wild(){
  s_wild ret;

  float max_V = 2.96; //максимальное напряжение делителя с 5В на резистортах (4,7кОм и 6,8кОм)
  float koef = 30 / max_V;
  float sensor = analogRead(PIN_WILD_SPEED);
  ret.speed_voltage = sensor * (3.3 / 4095.0); //сколько вольт на входе
  //переводим В в м/с
  ret.speed = koef * ret.speed_voltage; 
  ret.s_speed = String(ret.speed);

  max_V = 2.88;
  float koef_div = ( 4.95 / max_V ); //коэффициент делителя
  sensor = analogRead(PIN_WILD_DIRECT);
  ret.direct_voltage = sensor * (3.3 / 4095.0); //сколько вольт на входе

  if(ret.direct_voltage > 0.05 && ret.direct_voltage <= 0.40) {
    ret.s_direct = "N/E";
  } else if(ret.direct_voltage > 0.40 && ret.direct_voltage <= 0.76) {
    ret.s_direct = "E";
  } else if(ret.direct_voltage > 0.76 && ret.direct_voltage <= 1.12) {
    ret.s_direct = "S/E";
  } else if(ret.direct_voltage > 1.12 && ret.direct_voltage <= 1.49) {
    ret.s_direct = "S";
  } else if(ret.direct_voltage > 1.49 && ret.direct_voltage <= 1.86) {
    ret.s_direct = "S/W";
  } else if(ret.direct_voltage > 1.86 && ret.direct_voltage <= 2.21) {
    ret.s_direct = "W";
  } else if(ret.direct_voltage > 2.21 && ret.direct_voltage <= 2.63) {
    ret.s_direct = "N/W";
  } else {
    ret.s_direct = "N";
  }

  return ret;
}

boolean PubInterval(String topic, uint32_t val){
  int intval = val / 1000;
  return client.Publish(topic, String(intval));
}

void report( s_state sState, int mode){
  if(mode == 0 || ( mode == 1 && sState.interval_DHT22 != cur_state.interval_DHT22 ) ){
    PubInterval("dht22/interval", sState.interval_DHT22);
  }
  if(mode == 0 || ( mode == 1 && sState.interval_DS18B20 != cur_state.interval_DS18B20 ) ){
    PubInterval("ds18b20/interval", sState.interval_DS18B20);
  }
  if(mode == 0 || ( mode == 1 && sState.interval_bh1750fvi != cur_state.interval_bh1750fvi ) ){
    PubInterval("BH1750FVI/interval", sState.interval_bh1750fvi);
  }
  if(mode == 0 || ( mode == 1 && sState.interval_bmp280 != cur_state.interval_bmp280 ) ){
    PubInterval("bmp280/interval", sState.interval_bmp280);
  }
  if(mode == 0 || ( mode == 1 && sState.interval_WILD != cur_state.interval_WILD ) ){
    PubInterval("wild/interval", sState.interval_WILD);
  }

  if(ds18b20_readed && (mode == 0 || ( mode == 1 && sState.ds18b20.s_t != cur_state.ds18b20.s_t )) ){
    client.Publish("ds18b20/t", sState.ds18b20.s_t);
  }

  if(mode == 0 || ( mode == 1 && sState.dht22.s_t != cur_state.dht22.s_t ) ){
    client.Publish("dht22/t", sState.dht22.s_t);
  }
  if(mode == 0 || ( mode == 1 && sState.dht22.s_h != cur_state.dht22.s_h ) ){
    client.Publish("dht22/h", sState.dht22.s_h);
  }

  if(mode == 0 || ( mode == 1 && sState.bh1750fvi.lux != cur_state.bh1750fvi.lux ) ){
    client.Publish("BH1750FVI/lux", String(sState.bh1750fvi.lux));
  }

  if(mode == 0 || ( mode == 1 && sState.rg11.rain != cur_state.rg11.rain ) ){
    client.Publish("rain", String(sState.rg11.rain));
  }

  if(mode == 0 || ( mode == 1 && sState.wild.s_speed != cur_state.wild.s_speed ) ){
    client.Publish("wild/speed", sState.wild.s_speed);
  }
  if(mode == 0 || ( mode == 1 && sState.wild.s_direct != cur_state.wild.s_direct ) ){
    client.Publish("wild/direct", sState.wild.s_direct);
  }

  if(mode == 0 || ( mode == 1 && sState.bmp280.Pressure != cur_state.bmp280.Pressure ) ){
    client.Publish("bmp280/Pressure", sState.bmp280.Pressure);
  }
  if(mode == 0 || ( mode == 1 && sState.bmp280.Temperature != cur_state.bmp280.Temperature ) ){
    client.Publish("bmp280/Temperature", sState.bmp280.Temperature);
  }
  if(mode == 0 || ( mode == 1 && sState.bmp280.Altitude != cur_state.bmp280.Altitude ) ){
    client.Publish("bmp280/Altitude", sState.bmp280.Altitude);
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
//выдаем то, что нам выставил сторожевой таймер
  digitalWrite(PIN_WATCHDOG_OUT, digitalRead(PIN_WATCHDOG_IN));
  
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