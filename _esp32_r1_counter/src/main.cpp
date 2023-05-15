#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include "DHT.h"
#include <BH1750FVI.h>
#include <mqtt_ini.h>

#define def_path "home/r1/lamp"

#define PIN_DHT22     19
#define PIN_LAMP      18
#define PIN_LED       32

#define NUM_LEDS      51 //117
#define delayFL       5
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 3000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

uint32_t LastRead_bh1750fvi = 0; //запоминаем время последней публикации
uint32_t LastRead_DHT22 = 0; //запоминаем время последней публикации

#define DHTTYPE DHT22 
DHT dht(PIN_DHT22, DHTTYPE);

uint8_t ADDRESSPIN = 16;
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI bh1750fvi(ADDRESSPIN, DEVICEADDRESS, DEVICEMODE);

mqtt_ini client( 
  "R1_LAMP",     // Client name that uniquely identify your device
   def_path);

struct s_dht22{
  float f_h;
  float f_t;

  String s_h;
  String s_t;
};

struct s_bh1750fvi{
  uint16_t lux;
};

struct s_relay{
  int state;
};

struct s_led {
  boolean flag_change; // = false;
  uint8_t g; // = 0;
  uint8_t r; // = 0;
  uint8_t b; // = 255;
  uint8_t brightness; // = 100;

  boolean flag_on; // = false;
  int mode;
};

struct s_state{
  uint32_t interval_DHT22;
  uint32_t interval_bh1750fvi;

  s_bh1750fvi bh1750fvi;
  s_dht22 dht22;
  s_relay lamp;
  s_led led;
};

s_state cur_state;

void report( s_state, int);
boolean PubInterval(String topic, uint32_t val);
s_state SetColor( s_state state, uint8_t g, uint8_t r, uint8_t b);
s_state set_brightness( s_state state, uint8_t _brightness );
s_state Off( s_state state, boolean f_fl = true);
s_state On( s_state state, boolean f_fl = true );

void setup() {
  Serial.begin(115200);

  cur_state.interval_DHT22 = 5000;
  cur_state.interval_bh1750fvi = 1000;

  pinMode(PIN_LAMP, OUTPUT); digitalWrite(PIN_LAMP, LOW);

  dht.begin();
  bh1750fvi.begin(); 

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

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

boolean Publish_flag_on( s_state sState ) {
  if (sState.led.flag_on) {
    return client.Publish("led/flag_on", "1");
  } else {
    return client.Publish("led/flag_on", "0");
  }
}
boolean Publish_flag_change( s_state sState ) {
  if (sState.led.flag_change) {
    return client.Publish("led/flag_change", "1");
  } else {
    return client.Publish("led/flag_change", "0");
  }
}

void onMsgCommand( const String &message ){
  if (message == "on_led") {
    cur_state = On(cur_state);
    Publish_flag_on( cur_state );
  }
  if (message == "off_led") {
    cur_state = Off(cur_state);
    Publish_flag_on( cur_state );
  }

  if (message == "on_lamp") {
    digitalWrite(PIN_LAMP, HIGH);
  }
  if (message == "off_lamp") {
    digitalWrite(PIN_LAMP, LOW);
  }
}

void Msg_r( const String &message ){
  Serial.println("Msg_r: " + message);

  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led.r = message.toInt();
    if(cur.led.r >= 0 && cur.led.r <= 255 ){
      if(cur.led.r == cur_state.led.r) return;      
      cur_state = SetColor(cur_state, cur_state.led.g, cur.led.r, cur_state.led.b);
      if(cur_state.led.flag_on) { 
        FastLED.show();
      }
    }
  }
  client.Publish("led/rgb/red", String(cur_state.led.r));
}
void Msg_g( const String &message ){
  Serial.println("Msg_g: " + message);

  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led.g = message.toInt();
    if(cur.led.g >= 0 && cur.led.g <= 255 ){
      if(cur.led.g == cur_state.led.g) return;      
      cur_state = SetColor(cur_state, cur.led.g, cur_state.led.r, cur_state.led.b);
      if(cur_state.led.flag_on) { 
        FastLED.show();
      }
    }
  }
  client.Publish("led/rgb/green", String(cur_state.led.g));
}
void Msg_b( const String &message ){
  Serial.println("Msg_b: " + message);

  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led.b = message.toInt();
    if(cur.led.b >= 0 && cur.led.b <= 255 ){
      if(cur.led.b == cur_state.led.b) return;      
      cur_state = SetColor(cur_state, cur_state.led.g, cur_state.led.r, cur.led.b);
      if(cur_state.led.flag_on) { 
        FastLED.show();
      }
    }
  }
  client.Publish("led/rgb/blue", String(cur_state.led.b));
}
void Msg_brightness( const String &message ){
  Serial.println("Msg_brightness: " + message);
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led.brightness = message.toInt();
    if(cur.led.brightness == cur_state.led.brightness) return;
    
    if(cur.led.brightness >= 0 && cur.led.brightness <= 255 ){
      cur_state = set_brightness( cur, cur.led.brightness);
    }
  }
  client.Publish("led/brightness", String(cur_state.led.brightness));
}
void Msg_flag_change( const String &message ){
  Serial.println("Msg_flag_change: " + message);

  if(message == "1"){
    cur_state.led.flag_change = true;
  } else {
    cur_state.led.flag_change = false;
  }
  Publish_flag_change(cur_state);
}

void onConnection()
{
  client.Subscribe("Constants/home/r1/interval_DHT22", Msg_interval_DHT22, true); 
  client.Subscribe("Constants/home/r1/interval_bh1750fvi", Msg_interval_bh1750fvi, true);  
  client.Subscribe("Constants/home/r1/led/r", Msg_r, true); 
  client.Subscribe("Constants/home/r1/led/g", Msg_g, true); 
  client.Subscribe("Constants/home/r1/led/b", Msg_b, true); 
  client.Subscribe("Constants/home/r1/led/brightness", Msg_brightness, true); 
  client.Subscribe("Constants/home/r1/led/flag_change", Msg_flag_change, true); 
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

s_bh1750fvi Read_bh1750fvi( ){
  s_bh1750fvi ret;
  
  ret.lux = bh1750fvi.GetLightIntensity();
  if(ret.lux == 54612){
    ret.lux = cur_state.bh1750fvi.lux;
  }
  return ret;
}

s_state Read_state( ){
  s_state ret = cur_state;

  ret.lamp.state = digitalRead(PIN_LAMP);

  if(client.flag_start || (millis( ) - LastRead_DHT22 > cur_state.interval_DHT22 && cur_state.interval_DHT22 >= 0 ) ) {
    ret.dht22 = Read_DHT22( );
    LastRead_DHT22 = millis();
  }
  if(client.flag_start || (millis( ) - LastRead_bh1750fvi > cur_state.interval_bh1750fvi && cur_state.interval_bh1750fvi >= 0 ) ) {
    ret.bh1750fvi = Read_bh1750fvi( );
    LastRead_bh1750fvi = millis();
  }

  return ret;
};

boolean PubInterval(String topic, uint32_t val){
  int intval = val / 1000;
  return client.Publish(topic, String(intval));
}

void Brightness(uint8_t from, uint8_t to, boolean f_fl = true){
  if(to >= from) {
    if(f_fl){
      for(int i = from; i <= to; i++){
        FastLED.setBrightness(i);
        FastLED.show();
        delay(delayFL);
      }
    } else {
        FastLED.setBrightness(to);
        FastLED.show();
    }
  } else {
    if(f_fl){
      for(int i = from; i >= to; i--){
        FastLED.setBrightness(i);
        FastLED.show();
        delay(delayFL);
      }
    } else {
        FastLED.setBrightness(to);
        FastLED.show();
    }
  }
}

s_state set_brightness( s_state state, uint8_t _brightness ){
  s_state ret = state;

  if(_brightness < 0 || _brightness > 255) return ret;

  ret.led.brightness = _brightness;
  
  if(!ret.led.flag_on) return ret;

  Brightness(state.led.brightness, ret.led.brightness, ret.led.flag_change);

  return ret;
}

s_state Off( s_state state, boolean f_fl) {
  s_state ret = state;

  if (!ret.led.flag_on) return ret;

  Brightness(ret.led.brightness, 0, (ret.led.flag_change && f_fl));

  ret.led.flag_on = false;

  return ret;
}

s_state On( s_state state, boolean f_fl ) {
  s_state ret = state;

  if (ret.led.flag_on) return ret;

  SetColor( ret, ret.led.g, ret.led.r, ret.led.b );

  Brightness(0, ret.led.brightness, (ret.led.flag_change && f_fl));

  ret.led.flag_on = true;
  
  return ret;
}

s_state SetColor( s_state state, uint8_t g, uint8_t r, uint8_t b){
  s_state ret = state;
  ret.led.g = g;  ret.led.r = r;  ret.led.b = b;
  for (int i = 0 ; i < NUM_LEDS; i++ )
  {
    leds[i].setRGB( r, g, b);
  }
  return ret;
}

void report( s_state sState, int mode){
  if(mode == 0 || ( mode == 1 && sState.interval_DHT22 != cur_state.interval_DHT22 ) ){
    PubInterval("dht22/interval", sState.interval_DHT22);
  }
  if(mode == 0 || ( mode == 1 && sState.dht22.s_t != cur_state.dht22.s_t ) ){
    client.Publish("dht22/t", sState.dht22.s_t);
  }
  if(mode == 0 || ( mode == 1 && sState.dht22.s_h != cur_state.dht22.s_h ) ){
    client.Publish("dht22/h", sState.dht22.s_h);
  }

  if(mode == 0 || ( mode == 1 && sState.interval_bh1750fvi != cur_state.interval_bh1750fvi ) ){
    PubInterval("BH1750FVI/interval", sState.interval_bh1750fvi);
  }
  if(mode == 0 || ( mode == 1 && sState.bh1750fvi.lux != cur_state.bh1750fvi.lux ) ){
    client.Publish("BH1750FVI/lux", String(sState.bh1750fvi.lux));
  }

  if(mode == 0 || ( mode == 1 && sState.led.flag_change != cur_state.led.flag_change ) ){
    Publish_flag_change(sState);
  }
  if(mode == 0 || ( mode == 1 && sState.led.r != cur_state.led.r ) ){
    client.Publish("led/rgb/red", String(sState.led.r));
  }
  if(mode == 0 || ( mode == 1 && sState.led.g != cur_state.led.g ) ){
    client.Publish("led/rgb/green", String(sState.led.g));
  }
  if(mode == 0 || ( mode == 1 && sState.led.b != cur_state.led.b ) ){
    client.Publish("led/rgb/blue", String(sState.led.b));
  }
  if(mode == 0 || ( mode == 1 && sState.led.brightness != cur_state.led.brightness ) ){
    client.Publish("led/brightness", String(sState.led.brightness));
  }
  if(mode == 0 || ( mode == 1 && sState.led.flag_on != cur_state.led.flag_on ) ){
    Publish_flag_on(sState);
  }

  if(mode == 0 || ( mode == 1 && sState.lamp.state != cur_state.lamp.state ) ){
    client.Publish("lamp/state", String(sState.lamp.state));
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