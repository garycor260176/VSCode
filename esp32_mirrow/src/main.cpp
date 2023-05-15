#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <mqtt_ini.h>
#include <FastLED.h>
#include <BH1750FVI.h>

#define def_path "home/mirrow"

#define PIN_BTN               18
#define PIN_MOVE              19
#define PIN_LED               32
uint8_t ADDRESSPIN =          16;

#define NUM_LEDS              168
#define delayFL 5
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 3000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

uint32_t LastRead_bh1750fvi = 0; //запоминаем время последней публикации
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI bh1750fvi(ADDRESSPIN, DEVICEADDRESS, DEVICEMODE);

uint32_t TimeSinceLastBtn = 0;
boolean flagSinceLastBtn = false;

mqtt_ini client( 
  "MIRROW",     // Client name that uniquely identify your device
   def_path);

struct s_bh1750fvi{
  uint16_t lux;
  uint32_t interval_bh1750fvi = 500;
};

struct s_relay{
  int pin;
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

struct s_state {
  s_led led;
  s_bh1750fvi lux;
  s_relay btn;
  s_relay move;
};

s_state cur_state;

s_state SetColor( s_state state, uint8_t g, uint8_t r, uint8_t b);
s_state On( s_state state, boolean f_fl = true );
s_state Off( s_state state, boolean f_fl = true );
void report( s_state sState, int mode );
s_bh1750fvi Read_bh1750fvi( );

int ihue = 0;                //-HUE (0-255)
int thisdelay = 10;          //-FX LOOPS DELAY VAR
void new_rainbow_loop();

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  cur_state.btn.pin = PIN_BTN; pinMode(cur_state.btn.pin, INPUT);
  cur_state.move.pin = PIN_MOVE; pinMode(cur_state.move.pin, INPUT);

  cur_state.lux.interval_bh1750fvi = 500;

  cur_state.led.flag_on = false;
  cur_state.led.g = 255;
  cur_state.led.r = 255;
  cur_state.led.b = 255;
  cur_state.led.flag_change = true;
  cur_state.led.brightness = 100;
  cur_state.led.mode = 0;

  bh1750fvi.begin(); 

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  client.begin();
}

void onMsgCommand( const String &message ){
  s_state ret = cur_state;
  if (message == "command=on") {
    ret = On( ret );
  } else if (message == "command=off") {
    ret = Off( ret );
  } 
  report(ret, 1);
}

s_bh1750fvi Read_bh1750fvi( ) {
  s_bh1750fvi ret;
  ret = cur_state.lux;

  if (millis() - LastRead_bh1750fvi > ret.interval_bh1750fvi || client.flag_start) {
    ret.lux = bh1750fvi.GetLightIntensity();
    if(ret.lux == 54612){
      ret.lux = cur_state.lux.lux;
    }
    LastRead_bh1750fvi = millis();
  }
  return ret;
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

  ihue = 0;
  switch(ret.led.mode){
    case 1:
      fill_rainbow( leds, NUM_LEDS, ihue );
      break;
    default: 
      SetColor( ret, ret.led.g, ret.led.r, ret.led.b );
      break;
  }

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
      if(cur.led.flag_on && cur.led.mode == 0) { 
        cur = SetColor(cur, cur.led.g, cur.led.r, cur.led.b);
        FastLED.show();
      }
      report(cur, 1);
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
      if(cur.led.flag_on && cur.led.mode == 0) { 
        cur = SetColor(cur, cur.led.g, cur.led.r, cur.led.b);
        FastLED.show();
      }
      report(cur, 1);
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
      if(cur.led.flag_on && cur.led.mode == 0) { 
        cur = SetColor(cur, cur.led.g, cur.led.r, cur.led.b);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}
void Msg_mode( const String &message ){
  s_state cur = cur_state;

  if(!client.flag_Loaded) return;

  if(message.length() > 0){
    cur.led.mode = message.toInt();
    if(cur.led.mode >= 0 && cur.led.mode <= 1 ){
      if(cur.led.mode == cur_state.led.mode) return;      
      Serial.println("Msg_mode: " + message);
      cur = SetColor( cur, cur.led.g, cur.led.r, cur.led.b );
      if(cur.led.flag_on) { 
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}

boolean Publish_flag_on( s_state sState ) {
  if (sState.led.flag_on) {
    return client.Publish("flag_on", "1");
  } else {
    return client.Publish("flag_on", "0");
  }
}

s_state Read_movesens( s_state state ){
  s_state ret = state;

  ret.move.state = digitalRead(ret.move.pin);

  return ret;
}

void report( s_state sState, int mode ) {
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
  if(mode == 0 || ( mode == 1 && sState.led.mode != cur_state.led.mode ) ){
    client.Publish("mode_pic", String(sState.led.mode));
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

  if (mode == 0 || ( mode == 1 && sState.lux.lux != cur_state.lux.lux ) ) {
    client.Publish("BH1750FVI/lux", String(sState.lux.lux));
  }

  if (mode == 0 || ( mode == 1 && sState.move.state != cur_state.move.state ) ) {
    client.Publish("move/state", String(sState.move.state));
  }

  cur_state = sState;
  client.flag_start = false;
}

s_state Read_state( ) {
  s_state ret = cur_state;

  ret.lux = Read_bh1750fvi();
  ret = Read_movesens( ret );
  ret.btn.state = digitalRead(ret.btn.pin);
  return ret;
}

void set_2( ){
  s_state state = cur_state;
  SetColor(state, state.led.g, state.led.r, state.led.b);
  Brightness(0, state.led.brightness, false);
  delay(200);
  Brightness(0, 0, false);
  delay(200);
  Brightness(0, state.led.brightness, false);
  delay(200);
}

void OnCheckState(){
  s_state state = Read_state( );

  if(state.btn.state == HIGH){
    if(!flagSinceLastBtn){
      flagSinceLastBtn = true;
      TimeSinceLastBtn = millis( );
    }
  } else {
    if(flagSinceLastBtn){
      if(millis( ) - TimeSinceLastBtn >= 3000){
        set_2( );
        client.Publish("btn/state", "2");
      } else {
        client.Publish("btn/state", "1");
      }
      TimeSinceLastBtn = 0;
    }
    flagSinceLastBtn = false;
  }

  if (client.flag_start) { //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void onConnection(){
  client.Subscribe("brightness", Msg_brightness); 
  client.Subscribe("flag_change", Msg_flag_change); 
  client.Subscribe("rgb/red", Msg_led_red); 
  client.Subscribe("rgb/green", Msg_led_green); 
  client.Subscribe("rgb/blue", Msg_led_blue); 
  client.Subscribe("Constants/home/mirrow/mode_pic", Msg_mode, true); 
}

void OnLoad(){
}

void loop() {
  client.loop();

  if(cur_state.led.flag_on){
    switch (cur_state.led.mode)
    {
    case 1:
      new_rainbow_loop();
      break;
    
    default:
      break;
    }
  } 
}

void new_rainbow_loop() {                      //-m88-RAINBOW FADE FROM FAST_SPI2
  ihue -= 1;
  fill_rainbow( leds, NUM_LEDS, ihue );
  LEDS.show();
  delay(thisdelay);
}
