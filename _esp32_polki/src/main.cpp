#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <mqtt_ini.h>
#include <FastLED.h>

#define def_path "home/figures"

#define PIN_BTN     18

#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
#define delayFL 5

#define PIN_LED_01  19
#define NUM_LEDS_01 3
CRGB leds_01[NUM_LEDS_01];

#define PIN_LED_02  32
#define NUM_LEDS_02 3
CRGB leds_02[NUM_LEDS_02];

#define PIN_LED_03  14 //12, 14, 27, 26, 33, 25
#define NUM_LEDS_03 3
CRGB leds_03[NUM_LEDS_03];

uint32_t TimeSinceLastBtn = 0;
boolean flagSinceLastBtn = false;

mqtt_ini client( 
  "FIGURES",     // Client name that uniquely identify your device
   def_path);

struct s_led {
  int pin;

  uint8_t g; // = 0;
  uint8_t r; // = 0;
  uint8_t b; // = 255;
};

struct s_relay{
  int pin;
  int state;
};

struct s_state {
  s_led led_01;
  s_led led_02;
  s_led led_03;
  s_relay btn;

  boolean flag_change; // = false;
  boolean flag_on; // = false;
  uint8_t brightness; // = 100;
};

s_state cur_state;

s_state On( s_state state, boolean f_fl = true );
s_state Off( s_state state, boolean f_fl = true );
void Brightness(uint8_t from, uint8_t to, boolean f_fl = true);
s_led SetColorLed( s_led led);
s_state SetColor( s_state state);
s_state set_brightness( s_state state, uint8_t _brightness );
void report( s_state s_State, int mode );

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  cur_state.btn.pin = PIN_BTN; pinMode(cur_state.btn.pin, INPUT);

  cur_state.flag_on = false;
  cur_state.flag_change = true;
  cur_state.brightness = 100;

  cur_state.led_01.pin = PIN_LED_01;
  cur_state.led_01.g = 0;
  cur_state.led_01.r = 0;
  cur_state.led_01.b = 255;
  FastLED.addLeds<WS2812B, PIN_LED_01, GRB>(leds_01, NUM_LEDS_01);  // GRB ordering is assumed

  cur_state.led_02.pin = PIN_LED_02;
  cur_state.led_02.g = 0;
  cur_state.led_02.r = 0;
  cur_state.led_02.b = 255;
  FastLED.addLeds<WS2812B, PIN_LED_02, GRB>(leds_02, NUM_LEDS_02);  // GRB ordering is assumed

  cur_state.led_03.pin = PIN_LED_03;
  cur_state.led_03.g = 0;
  cur_state.led_03.r = 0;
  cur_state.led_03.b = 255;
  FastLED.addLeds<WS2812B, PIN_LED_03, GRB>(leds_03, NUM_LEDS_03);  // GRB ordering is assumed

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
void Msg_brightness( const String &message ){
  s_state cur = cur_state;

  if(message.length() > 0){
    cur.brightness = message.toInt();
    if(cur.brightness == cur_state.brightness) return;
    Serial.println("Msg_brightness: " + message);
    
    if(cur.brightness >= 0 && cur.brightness <= 255 ){
      cur = set_brightness( cur, cur.brightness);
    }

    report(cur, 1);
  }
}
void Msg_flag_change( const String &message ){
  Serial.println("Msg_flag_change: " + message);

  s_state ret = cur_state;

  if(message == "1"){
    ret.flag_change = true;
  } else {
    ret.flag_change = false;
  }

  report(ret, 1);
}

void Msg_led_red_01( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_01.r = message.toInt();
    if(cur.led_01.r >= 0 && cur.led_01.r <= 255 ){
      if(cur.led_01.r == cur_state.led_01.r) return;      
      Serial.println("Msg_led_red_01: " + message);
      if(cur.flag_on) { 
        cur.led_01 = SetColorLed(cur.led_01);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}
void Msg_led_green_01( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_01.g = message.toInt();
    if(cur.led_01.g >= 0 && cur.led_01.g <= 255 ){
      if(cur.led_01.g == cur_state.led_01.g) return;      
      Serial.println("Msg_led_green_01: " + message);
      if(cur.flag_on) { 
        cur.led_01 = SetColorLed(cur.led_01);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}
void Msg_led_blue_01( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_01.b = message.toInt();
    if(cur.led_01.b >= 0 && cur.led_01.b <= 255 ){
      if(cur.led_01.b == cur_state.led_01.b) return;      
      Serial.println("Msg_led_blue_01: " + message);
      if(cur.flag_on) { 
        cur.led_01 = SetColorLed(cur.led_01);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}

void Msg_led_red_02( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_02.r = message.toInt();
    if(cur.led_02.r >= 0 && cur.led_02.r <= 255 ){
      if(cur.led_02.r == cur_state.led_02.r) return;      
      Serial.println("Msg_led_red_02: " + message);
      if(cur.flag_on) { 
        cur.led_02 = SetColorLed(cur.led_02);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}
void Msg_led_green_02( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_02.g = message.toInt();
    if(cur.led_02.g >= 0 && cur.led_02.g <= 255 ){
      if(cur.led_02.g == cur_state.led_02.g) return;      
      Serial.println("Msg_led_green_02: " + message);
      if(cur.flag_on) { 
        cur.led_02 = SetColorLed(cur.led_02);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}
void Msg_led_blue_02( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_02.b = message.toInt();
    if(cur.led_02.b >= 0 && cur.led_02.b <= 255 ){
      if(cur.led_02.b == cur_state.led_02.b) return;      
      Serial.println("Msg_led_blue_02: " + message);
      if(cur.flag_on) { 
        cur.led_02 = SetColorLed(cur.led_02);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}

void Msg_led_red_03( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_03.r = message.toInt();
    if(cur.led_03.r >= 0 && cur.led_03.r <= 255 ){
      if(cur.led_03.r == cur_state.led_03.r) return;      
      Serial.println("Msg_led_red_03: " + message);
      if(cur.flag_on) { 
        cur.led_03 = SetColorLed(cur.led_03);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}
void Msg_led_green_03( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_03.g = message.toInt();
    if(cur.led_03.g >= 0 && cur.led_03.g <= 255 ){
      if(cur.led_03.g == cur_state.led_03.g) return;      
      Serial.println("Msg_led_green_03: " + message);
      if(cur.flag_on) { 
        cur.led_03 = SetColorLed(cur.led_03);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}
void Msg_led_blue_03( const String &message ){
  s_state cur = cur_state;
  if(message.length() > 0){
    cur.led_03.b = message.toInt();
    if(cur.led_03.b >= 0 && cur.led_03.b <= 255 ){
      if(cur.led_03.b == cur_state.led_03.b) return;      
      Serial.println("Msg_led_blue_03: " + message);
      if(cur.flag_on) { 
        cur.led_03 = SetColorLed(cur.led_03);
        FastLED.show();
      }
      report(cur, 1);
    }
  }
}

void Brightness(uint8_t from, uint8_t to, boolean f_fl){
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

  ret.brightness = _brightness;
  
  if(!ret.flag_on) return ret;

  Brightness(state.brightness, ret.brightness, ret.flag_change);

  return ret;
}
s_state Off( s_state state, boolean f_fl) {
  s_state ret = state;

  if (!ret.flag_on) return ret;

  Brightness(ret.brightness, 0, (ret.flag_change && f_fl));

  ret.flag_on = false;

  return ret;
}
s_state On( s_state state, boolean f_fl ) {
  s_state ret = state;

  if (ret.flag_on) return ret;

  ret = SetColor( ret );

  Brightness(0, ret.brightness, (ret.flag_change && f_fl));

  ret.flag_on = true;
  return ret;
}
s_led SetColorLed( s_led led){
  s_led ret = led;
  switch(led.pin){
    case PIN_LED_01: 
      for (int i = 0 ; i < NUM_LEDS_01; i++ )
      {
        leds_01[i].setRGB( ret.r, ret.g, ret.b);
      }
      break;
    case PIN_LED_02: 
      for (int i = 0 ; i < NUM_LEDS_02; i++ )
      {
        leds_02[i].setRGB( ret.r, ret.g, ret.b);
      }
      break;
    case PIN_LED_03: 
      for (int i = 0 ; i < NUM_LEDS_03; i++ )
      {
        leds_03[i].setRGB( ret.r, ret.g, ret.b);
      }
      break;
  }

  return ret;
}
s_state SetColor( s_state state){
  s_state ret = state;

  ret.led_01 = SetColorLed( ret.led_01 );
  ret.led_02 = SetColorLed( ret.led_02 );
  ret.led_03 = SetColorLed( ret.led_03 );

  return ret;
}

boolean Publish_flag_on( s_state sState ) {
  if (sState.flag_on) {
    return client.Publish("flag_on", "1");
  } else {
    return client.Publish("flag_on", "0");
  }
}

s_state Read_state( ) {
  s_state ret = cur_state;

  ret.btn.state = digitalRead(ret.btn.pin);
  return ret;
}

void report_Led(String s, s_led led_old, s_led led_new, int mode){
  if(mode == 0 || ( mode == 1 && led_new.r != led_old.r ) ){
    client.Publish("rgb_" + s + "/red", String(led_new.r));
  }
  if(mode == 0 || ( mode == 1 && led_new.g != led_old.g ) ){
    client.Publish("rgb_" + s + "/green", String(led_new.g));
  }
  if(mode == 0 || ( mode == 1 && led_new.b != led_old.b ) ){
    client.Publish("rgb_" + s + "/blue", String(led_new.b));
  }
}

void report( s_state s_State, int mode ) {
  s_state sState = s_State;

  if(mode == 0 || ( mode == 1 && sState.brightness != cur_state.brightness ) ){
    client.Publish("brightness", String(sState.brightness));
  }
  if(mode == 0 || ( mode == 1 && sState.flag_on != cur_state.flag_on ) ){
    Publish_flag_on( sState );
  }
  if(mode == 0 || ( mode == 1 && sState.flag_change != cur_state.flag_change ) ){
    if(sState.flag_change) {
      client.Publish("flag_change", "1");
    } else {
      client.Publish("flag_change", "0");
    }
  }

  report_Led("01", cur_state.led_01, sState.led_01, mode);
  report_Led("02", cur_state.led_02, sState.led_02, mode);
  report_Led("03", cur_state.led_03, sState.led_03, mode);

  cur_state = sState;
  client.flag_start = false;
}

void set_2( ){
  s_state state = cur_state;
  SetColor(state);
  int j;
  if(state.flag_on){
    j = 0;
  } else {
    j = state.brightness;
  }

  for(int i=0; i<=3; i++) {
    Brightness(0, j, false);
    delay(200);
    if(j == state.brightness) {
      j = 0;
    } else {
      j = state.brightness;
    }
  }
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
        state.btn.state = 2;
      } else {
        state.btn.state = 1;
      }
      client.Publish("btn/state", String(state.btn.state));
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

  client.Subscribe("rgb_01/red", Msg_led_red_01); 
  client.Subscribe("rgb_01/green", Msg_led_green_01); 
  client.Subscribe("rgb_01/blue", Msg_led_blue_01); 

  client.Subscribe("rgb_02/red", Msg_led_red_02); 
  client.Subscribe("rgb_02/green", Msg_led_green_02); 
  client.Subscribe("rgb_02/blue", Msg_led_blue_02); 

  client.Subscribe("rgb_03/red", Msg_led_red_03); 
  client.Subscribe("rgb_03/green", Msg_led_green_03); 
  client.Subscribe("rgb_03/blue", Msg_led_blue_03); 
}

void OnLoad(){
  client.flag_start = true;
}

void loop() {
  client.loop();
}
