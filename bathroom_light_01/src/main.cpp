#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "DHT.h"
#include <BH1750FVI.h>
#include <FastLED.h>
#include <mqtt_ini.h>

#define def_path "home/bath/lamp"

#define FLAG_SEND_BH1750FVI   1
#define FLAG_SEND_MOVESENS    2
#define FLAG_SEND_LED         4
#define FLAG_SEND_DHT22       8

#define PIN_MOVESENS          D6
#define PIN_LED               D5
#define DHTPIN                D7

#define NUM_LEDS              88
#define delayFL 5
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define INTERVAL_CHECK_BH1750FVI  500
#define DEFAULT_led_on_lux        0
#define DEFAULT_led_off_lux       8

uint32_t LAST_CHECK_BH1750FVI = 0;
uint32_t LAST_CHECK_DHT22 = 0; //запоминаем время последней публикации

struct s_dht22 {
  float f_h;
  float f_t;

  uint32_t intervalRead; //в минутах

  String s_h;
  String s_t;
};

struct s_led {
  boolean flag_change; // = false;
  uint8_t g; // = 0;
  uint8_t r; // = 0;
  uint8_t b; // = 255;
  uint8_t brightness; // = 100;

  boolean flag_on; // = false;
};

struct s_bh1750fvi {
  uint16_t lux;
};

struct s_movesens {
  int state;
};

struct s_state {
  s_led led;
  s_dht22 dht22;
  s_bh1750fvi bh1750fvi;
  s_movesens movesens;
};

s_state cur_state;

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

uint8_t ADDRESSPIN = 13;
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI bh1750fvi(ADDRESSPIN, DEVICEADDRESS, DEVICEMODE);

mqtt_ini client( 
  "ESP8266_BATHROOM_LED", 
   def_path);

s_state SetColor( s_state state, uint8_t g, uint8_t r, uint8_t b);
s_state On( s_state state, boolean f_fl = true );
s_state Off( s_state state, boolean f_fl = true );
void report( s_state sState, int mode);

void setup() {
  Serial.begin(115200);

  cur_state.dht22.intervalRead = 30000;
  cur_state.led.flag_on = false;
  cur_state.led.g = 255;
  cur_state.led.r = 255;
  cur_state.led.b = 255;
  cur_state.led.flag_change = true;
  cur_state.led.brightness = 50;

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  pinMode(PIN_MOVESENS, INPUT);

  bh1750fvi.begin();
  dht.begin();

  client.begin();
  delay(100);
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

void Msg_brightness( const String &message ){
  s_state cur = cur_state;

  if(message.length() > 0){
    cur.led.brightness = message.toInt();
    if(cur.led.brightness == cur_state.led.brightness) return;
  Serial.println("Msg_brightness: " + message);
    
    if(cur.led.brightness >= 0 && cur.led.brightness <= 255 ){
      cur_state = set_brightness( cur, cur.led.brightness);
    }
  }
}

void Msg_flag_change( const String &message ){
  Serial.println("Msg_flag_change: " + message);

  if(message == "1"){
    cur_state.led.flag_change = true;
  } else {
    cur_state.led.flag_change = false;
  }
}

void Msg_led_red( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led.r = message.toInt();
    if(cur.led.r >= 0 && cur.led.r <= 255 ){
      if(cur.led.r == cur_state.led.r) return;      
  Serial.println("Msg_led_red: " + message);
      cur_state = SetColor(cur_state, cur_state.led.g, cur.led.r, cur_state.led.b);
      if(cur_state.led.flag_on) { 
        FastLED.show();
      }
    }
  }
}
void Msg_led_green( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led.g = message.toInt();
    if(cur.led.g >= 0 && cur.led.g <= 255 ){
      if(cur.led.g == cur_state.led.g) return;      
  Serial.println("Msg_led_green: " + message);
      cur_state = SetColor(cur_state, cur.led.g, cur_state.led.r, cur_state.led.b);
      if(cur_state.led.flag_on) { 
        FastLED.show();
      }
    }
  }
}
void Msg_led_blue( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led.b = message.toInt();
    if(cur.led.b >= 0 && cur.led.b <= 255 ){
      if(cur.led.b == cur_state.led.b) return;      
  Serial.println("Msg_led_blue: " + message);
      cur_state = SetColor(cur_state, cur_state.led.g, cur_state.led.r, cur.led.b);
      if(cur_state.led.flag_on) { 
        FastLED.show();
      }
    }
  }
}

void onMsgCommand( const String &message ) {
  s_state ret = cur_state;
  if (message == "command=on") {
    ret = On( ret );
  }
  if (message == "command=off") {
    ret = Off( ret );
  }
  report(ret, 1);
}

void onConnection()
{
  client.Subscribe("brightness", Msg_brightness); 
  client.Subscribe("flag_change", Msg_flag_change); 
  client.Subscribe("rgb/red", Msg_led_red); 
  client.Subscribe("rgb/green", Msg_led_green); 
  client.Subscribe("rgb/blue", Msg_led_blue); 
}

s_bh1750fvi Read_bh1750fvi( ) {
  s_bh1750fvi ret;
  ret = cur_state.bh1750fvi;

  if ((millis() - LAST_CHECK_BH1750FVI) > INTERVAL_CHECK_BH1750FVI || client.flag_start) {
    ret.lux = bh1750fvi.GetLightIntensity();
    if(ret.lux == 54612){
      ret.lux = cur_state.bh1750fvi.lux;
    }
    LAST_CHECK_BH1750FVI = millis();
  }
  return ret;
}

s_dht22 Read_DHT22() {
  s_dht22 ret = cur_state.dht22;

  if (millis() - LAST_CHECK_DHT22 > ret.intervalRead || client.flag_start) {
    ret.f_t = dht.readTemperature();
    ret.f_h = dht.readHumidity();
    if (isnan(ret.f_h) || isnan(ret.f_t)) {

    } else {
      ret.s_t = String(ret.f_t);
      ret.s_h = String(ret.f_h);
    }
    LAST_CHECK_DHT22 = millis();
  }

  return ret;
}

s_movesens Read_movesens( s_state cur) {
  s_movesens ret = cur_state.movesens;
  ret.state = digitalRead(PIN_MOVESENS);
  return ret;
}

s_state Read_state( ) {
  s_state ret = cur_state;
  ret.bh1750fvi = Read_bh1750fvi();
  ret.dht22 = Read_DHT22();
  ret.movesens = Read_movesens( ret );
  return ret;
}

boolean Publish_flag_on( s_state sState ) {
  if (sState.led.flag_on) {
    return client.Publish("flag_on", "1");
  } else {
    return client.Publish("flag_on", "0");
  }
}

void report( s_state sState, int mode) {
  if (mode == 0 || ( mode == 1 && sState.bh1750fvi.lux != cur_state.bh1750fvi.lux ) ) {
    client.Publish("BH1750FVI/lux", String(sState.bh1750fvi.lux));
  }

  if (mode == 0 || ( mode == 1 && sState.dht22.s_t != cur_state.dht22.s_t ) ) {
    client.Publish("DHT22/t", sState.dht22.s_t);
  }
  if (mode == 0 || ( mode == 1 && sState.dht22.s_h != cur_state.dht22.s_h ) ) {
    client.Publish("DHT22/h", sState.dht22.s_h);
  }

  if (mode == 0 || ( mode == 1 && sState.movesens.state != cur_state.movesens.state ) ) {
    client.Publish("movesens/state", String(sState.movesens.state));
  }

  if(mode == 0 || ( mode == 1 && sState.led.r != cur_state.led.r ) ){
    client.Publish("rgb/red", String(sState.led.r));
  }
  if(mode == 0 || ( mode == 1 && sState.led.g != cur_state.led.g ) ){
    client.Publish("rgb/green", String(sState.led.g));
  }
  if(mode == 0 || ( mode == 1 && sState.led.b != cur_state.led.b ) ){
    client.Publish("rgb/blue", String(sState.led.b));
  }
  if(mode == 0 || ( mode == 1 && sState.led.brightness != cur_state.led.brightness ) ){
    client.Publish("brightness", String(sState.led.brightness));
  }
  if(mode == 0 || ( mode == 1 && sState.led.flag_change != cur_state.led.flag_change ) ){
    if(sState.led.flag_change) {
      client.Publish("flag_change", "1");
    } else {
      client.Publish("flag_change", "0");
    }
  }
  if(mode == 0 || ( mode == 1 && sState.led.flag_on != cur_state.led.flag_on ) ){
    Publish_flag_on( sState );
  }

  cur_state = sState;
  client.flag_start = false;
}

void OnCheckState() {
  s_state state = Read_state( );

  if (client.flag_start) { //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void OnLoad(){

}

void loop() {
  client.loop();
}