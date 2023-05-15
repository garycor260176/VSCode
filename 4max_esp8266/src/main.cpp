#include <mqtt_ini.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#define def_path "home/4max"

#define FLAG_SEND_STATE     1

#define PIN_LED             D4
#define PIN_BTN             D5
#define PIN_RELAY           D6

#define NUMPIXELS 24 // Popular NeoPixel ring size

uint32_t TimeSinceLastBtn = 0;
boolean flagSinceLastBtn = false;

#define NUM_LEDS 24
#define delayFL 5
CRGB leds[NUM_LEDS];

#define CURRENT_LIMIT 1000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

mqtt_ini client( 
  "ESP8266_4MAX",     // Client name that uniquely identify your device
   def_path);

struct stru_led{
  boolean flag_change; // = false;
  uint8_t g; // = 0;
  uint8_t r; // = 0;
  uint8_t b; // = 255;
  boolean flag_on; // = false;
  uint8_t brightness; // = 100;
};

struct stru_btn{
  int pin;
  int state;
};

struct s_state{
  stru_led led;
  stru_btn btn;
  stru_btn relay;
};

s_state cur_state; //предыдущее отправленное состояние 

void report( s_state, int);

s_state SetColor( s_state state, uint8_t g, uint8_t r, uint8_t b);
s_state On( s_state state, boolean f_fl = true );
s_state Off( s_state state, boolean f_fl = true );

void setup() {

  Serial.begin(115200);
  Serial.println("Start...");

  cur_state.btn.pin = PIN_BTN;
  cur_state.relay.pin = PIN_RELAY;

  cur_state.led.g = 255;
  cur_state.led.r = 255;
  cur_state.led.b = 255;
  cur_state.led.flag_on = false;
  cur_state.led.flag_change = true;
  cur_state.led.brightness = 50;  
  
  pinMode(cur_state.btn.pin, INPUT);  
  pinMode(cur_state.relay.pin, OUTPUT);  digitalWrite(cur_state.relay.pin, LOW);

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  client.begin();
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

s_state Off( s_state state, boolean f_fl ){
  s_state ret = state;

  if(!ret.led.flag_on) return ret;
  Brightness(ret.led.brightness, 0, (ret.led.flag_change && f_fl));

  ret.led.flag_on = false;

  return ret;
}

s_state On( s_state state, boolean f_fl ){
  s_state ret = state;

  if(ret.led.flag_on) return ret;
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

boolean Publish_flag_on( s_state sState ) {
  if(sState.led.flag_on) {
    return client.Publish("state", "1");
  } else {
    return client.Publish("state", "0");
  }
}

void onMsgCommand( const String &message ){
  if (message == "command=on") {
    cur_state = On( cur_state);
    Publish_flag_on(cur_state);
  }
  if (message == "command=off") {
    cur_state = Off( cur_state );
    Publish_flag_on(cur_state);
  }
  if (message == "command=onPrinter") {
    digitalWrite(cur_state.relay.pin, HIGH);
  }
  if (message == "command=offPrinter") {
    digitalWrite(cur_state.relay.pin, LOW);
  }
}

void report( s_state sState1, int mode){
  s_state sState = sState1;
  
  if(mode == 0 || ( mode == 1 && sState.relay.state != cur_state.relay.state) ){
    client.Publish("relay_state", String(sState.relay.state));
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

s_state Read_state( ){
  s_state ret = cur_state;
  ret.btn.state = digitalRead( ret.btn.pin );
  ret.relay.state = digitalRead( ret.relay.pin );

  return ret;
};

void OnCheckState(){
  s_state state = Read_state( );

  if(state.btn.state == HIGH){
    if(!flagSinceLastBtn){
      flagSinceLastBtn = true;
      TimeSinceLastBtn = millis( );
    }
  } else {
    if(flagSinceLastBtn){
      if(millis( ) - TimeSinceLastBtn >= 5000){
        if(state.relay.state == LOW) {
          state.relay.state = HIGH;
        } else {
          state.relay.state = LOW;
        }
        digitalWrite(cur_state.relay.pin, state.relay.state);
      } else {
        if( state.led.flag_on ){
          state = Off( state );
        } else {
          state = On( state );
        }
      }
      TimeSinceLastBtn = 0;
    }
    flagSinceLastBtn = false;
  }
  
  if(client.flag_start){ //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void onConnection()
{
  client.Subscribe("brightness", Msg_brightness); 
  client.Subscribe("flag_change", Msg_flag_change); 
  client.Subscribe("rgb/red", Msg_led_red); 
  client.Subscribe("rgb/green", Msg_led_green); 
  client.Subscribe("rgb/blue", Msg_led_blue); 
}

void OnLoad(){
  client.flag_start = true;
}

void loop() {
  client.loop();
}