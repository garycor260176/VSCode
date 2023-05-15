#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <BH1750FVI.h>
#include <FastLED.h>
#include <mqtt_ini.h>

#define def_path "home/L01"

#define PIN_MOVESENS          D5
#define PIN_LED               D6

#define NUM_LEDS              26
#define delayFL 5
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 1500    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define INTERVAL_CHECK_BH1750FVI  1000

uint32_t LAST_CHECK_BH1750FVI = 0;
uint32_t LAST_MOVESENS = 0;

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
  s_bh1750fvi bh1750fvi;
  s_movesens movesens;
};

s_state cur_state;

uint8_t ADDRESSPIN = 13;
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI bh1750fvi(ADDRESSPIN, DEVICEADDRESS, DEVICEMODE);

mqtt_ini client( 
  "ESP8266_L01", 
   def_path);

s_state SetColor( s_state state, uint8_t g, uint8_t r, uint8_t b);
s_state On( s_state state, boolean f_fl = true );
s_state Off( s_state state, boolean f_fl = true );
boolean Publish_flag_change( s_state sState );

void setup() {
  Serial.begin(115200);

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

  cur_state.movesens.state = LOW;
  pinMode(PIN_MOVESENS, INPUT);
  pinMode(PIN_LED, OUTPUT); digitalWrite(PIN_LED, LOW);

  bh1750fvi.begin();
  
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
  Publish_flag_change( cur_state );
}

void Msg_led_red( const String &message ){
  Serial.println("Msg_led_red: " + message);

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
  client.Publish("rgb/red", String(cur_state.led.r));
}
void Msg_led_green( const String &message ){
  Serial.println("Msg_led_green: " + message);
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
  client.Publish("rgb/green", String(cur_state.led.g));
}
void Msg_led_blue( const String &message ){
  Serial.println("Msg_led_blue: " + message);
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
  client.Publish("rgb/blue", String(cur_state.led.b));
}

boolean Publish_flag_change( s_state sState ) {
  if (sState.led.flag_change) {
    return client.Publish("flag_change", "1");
  } else {
    return client.Publish("flag_change", "0");
  }
}

boolean Publish_flag_on( s_state sState ) {
  if (sState.led.flag_on) {
    return client.Publish("flag_on", "1");
  } else {
    return client.Publish("flag_on", "0");
  }
}

void onMsgCommand( const String &message ) {
  if (message == "on") {
    cur_state = On(cur_state);
    Publish_flag_on( cur_state );
  }
  if (message == "off") {
    cur_state = Off(cur_state);
    Publish_flag_on( cur_state );
  }
}

void onConnection()
{
  client.Subscribe("Constants/home/L01/brightness", Msg_brightness, true); 
  client.Subscribe("Constants/home/L01/flag_change", Msg_flag_change, true); 
  client.Subscribe("Constants/home/L01/r", Msg_led_red, true); 
  client.Subscribe("Constants/home/L01/g", Msg_led_green, true); 
  client.Subscribe("Constants/home/L01/b", Msg_led_blue, true); 
}

s_bh1750fvi Read_bh1750fvi( ) {
  s_bh1750fvi ret;
  ret = cur_state.bh1750fvi;

  if (millis() - LAST_CHECK_BH1750FVI > INTERVAL_CHECK_BH1750FVI || client.flag_start) {
    ret.lux = bh1750fvi.GetLightIntensity();
    LAST_CHECK_BH1750FVI = millis();
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
  ret.movesens = Read_movesens( ret );
  return ret;
}

void report( s_state sState, int mode) {
  if (mode == 0 || ( mode == 1 && sState.bh1750fvi.lux != cur_state.bh1750fvi.lux ) ) {
    client.Publish("BH1750FVI/lux", String(sState.bh1750fvi.lux));
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
    Publish_flag_change( sState );
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