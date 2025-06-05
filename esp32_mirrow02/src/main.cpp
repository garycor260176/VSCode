#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <mqtt_ini.h>
#include <FastLED.h>
#include <BH1750FVI.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <cl_bh1750fvi.h>

Preferences preferences; //хранение текущего состояния

#define def_path "mirrow1"

#define PIN_BTN               18
#define PIN_MOVE              19
#define PIN_LED               32
uint8_t PIN_BH1750FVI_ADDR  = 16;

#define NUM_LEDS              168
#define delayFL 5
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 3000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

mqtt_ini client( 
  "MIRROW",     // Client name that uniquely identify your device
   def_path);

#define def_subpath_bh1750fvi "bh1750fvi"
uint32_t LastRead_bh1750fvi = 0; //запоминаем время последней публикации
BH1750FVI::eDeviceAddress_t DEVICEADDRESS = BH1750FVI::k_DevAddress_H;
BH1750FVI::eDeviceMode_t DEVICEMODE = BH1750FVI::k_DevModeContHighRes2;
BH1750FVI _BH1750FVI(PIN_BH1750FVI_ADDR, DEVICEADDRESS, DEVICEMODE);
cl_bh1750fvi bh1750fvi(&_BH1750FVI, &client, String(def_subpath_bh1750fvi), 30000, false);

uint32_t TimeSinceLastBtn = 0;
boolean flagSinceLastBtn = false;

uint64_t move_save_time = 0;

int ihue = 0;                //-HUE (0-255)
int thisdelay = 10;          //-FX LOOPS DELAY VAR

struct s_bh1750fvi{
  uint16_t lux;
  uint32_t interval_bh1750fvi = 500;
};

struct s_relay{
  int pin;
  int state;
};

struct s_led {
  uint8_t g; // = 0;
  uint8_t r; // = 0;
  uint8_t b; // = 255;
  uint8_t brightness; // = 100;
  int flag_on; // = false;
};

struct s_settings{
  int flag_change; // = false;
  int mode; //0-авто, 1-on, 2-off
  int mode_pic; //0-просто цвет, 1-радуга
  uint32_t IntervalNoMove;
  uint32_t lux_off;
  uint32_t lux_on;
};

struct s_state {
  s_settings settings;

  s_led led;
  s_bh1750fvi lux;
  s_relay btn;
  s_relay move;

  uint64_t IntervalNoMove_ms;
  int move_state_time;

  boolean alarm;
};

s_state cur_state;
s_state eeprom;

void read_eeprom(){
  cur_state.settings.flag_change = preferences.getInt("flag_change", 1);
  if(cur_state.settings.flag_change < 0 || cur_state.settings.flag_change > 1){
    cur_state.settings.flag_change = 1;
  }
  Serial.println("read flag_change = " + String(cur_state.settings.flag_change));

  cur_state.settings.mode = preferences.getInt("mode", 0);
  if(cur_state.settings.mode < 0 || cur_state.settings.mode > 2) cur_state.settings.mode = 0;
  Serial.println("read mode = " + String(cur_state.settings.mode));

  cur_state.settings.mode_pic = preferences.getInt("mode_pic", 0);
  if(cur_state.settings.mode_pic < 0 || cur_state.settings.mode_pic > 1) cur_state.settings.mode_pic = 1;
  Serial.println("read mode_pic = " + String(cur_state.settings.mode_pic));

  cur_state.settings.IntervalNoMove = preferences.getUInt("IntervalNoMove", 10);
  if(cur_state.settings.IntervalNoMove < 0) cur_state.settings.IntervalNoMove = 10;
  cur_state.IntervalNoMove_ms = cur_state.settings.IntervalNoMove * 1000;
  Serial.println("read IntervalNoMove = " + String(cur_state.settings.IntervalNoMove));

  cur_state.settings.lux_on = preferences.getUInt("lux_on", 0);
  if(cur_state.settings.lux_on < 0) cur_state.settings.lux_on = 0;
  Serial.println("read lux_on = " + String(cur_state.settings.lux_on));

  cur_state.settings.lux_off = preferences.getUInt("lux_off", 5);
  if(cur_state.settings.lux_off < 0) cur_state.settings.lux_off = 0;
  Serial.println("read lux_off = " + String(cur_state.settings.lux_off));

  cur_state.settings.flag_change = preferences.getInt("flag_change", 5);
  if(cur_state.settings.flag_change < 0) cur_state.settings.flag_change = 0;
  Serial.println("read flag_change = " + String(cur_state.settings.flag_change));


  cur_state.led.brightness = preferences.getInt("brightness", 50);
  if(cur_state.led.brightness < 0 || cur_state.led.brightness > 255) cur_state.led.brightness = 50;
  Serial.println("read brightness = " + String(cur_state.led.brightness));

  cur_state.led.r = preferences.getInt("r", 0);
  if(cur_state.led.r < 0 || cur_state.led.r > 255) cur_state.led.r = 0;
  Serial.println("read r = " + String(cur_state.led.r));

  cur_state.led.g = preferences.getInt("g", 0);
  if(cur_state.led.g < 0 || cur_state.led.g > 255) cur_state.led.g = 0;
  Serial.println("read g = " + String(cur_state.led.g));

  cur_state.led.b = preferences.getInt("b", 255);
  if(cur_state.led.b < 0 || cur_state.led.b > 255) cur_state.led.b = 255;
  Serial.println("read b = " + String(cur_state.led.b));

  eeprom = cur_state;
}

void write_eeprom(){
  if(cur_state.settings.flag_change != eeprom.settings.flag_change)    {
    preferences.putInt("flag_change", cur_state.settings.flag_change);
    Serial.println("save flag_change = " + String(cur_state.settings.flag_change));
  }

  if(cur_state.settings.mode != eeprom.settings.mode)    {
    preferences.putInt("mode", cur_state.settings.mode);
    Serial.println("save mode = " + String(cur_state.settings.mode));
  }

  if(cur_state.settings.mode_pic != eeprom.settings.mode_pic)    {
    preferences.putInt("mode_pic", cur_state.settings.mode_pic);
    Serial.println("save mode_pic = " + String(cur_state.settings.mode_pic));
  }

  if(cur_state.settings.IntervalNoMove != eeprom.settings.IntervalNoMove)    {
    preferences.putUInt("IntervalNoMove", cur_state.settings.IntervalNoMove);
    Serial.println("save IntervalNoMove = " + String(cur_state.settings.IntervalNoMove));
  }

  if(cur_state.settings.lux_on != eeprom.settings.lux_on)    {
    preferences.putUInt("lux_on", cur_state.settings.lux_on);
    Serial.println("save lux_on = " + String(cur_state.settings.lux_on));
  }

  if(cur_state.settings.lux_off != eeprom.settings.lux_off)    {
    preferences.putUInt("lux_off", cur_state.settings.lux_off);
    Serial.println("save lux_off = " + String(cur_state.settings.lux_off));
  }

  if(cur_state.settings.flag_change != eeprom.settings.flag_change)    {
    preferences.putInt("flag_change", cur_state.settings.flag_change);
    Serial.println("save flag_change = " + String(cur_state.settings.flag_change));  
  }

  if(cur_state.led.brightness != eeprom.led.brightness)    {
    preferences.putInt("brightness", cur_state.led.brightness);
    Serial.println("save brightness = " + String(cur_state.led.brightness));
  }
  if(cur_state.led.r != eeprom.led.r)    {
    preferences.putInt("r", cur_state.led.r);
    Serial.println("save r = " + String(cur_state.led.r)); 
  }
  if(cur_state.led.g != eeprom.led.g)    {
    preferences.putInt("g", cur_state.led.g);
    Serial.println("save g = " + String(cur_state.led.g)); 
  }
  if(cur_state.led.b != eeprom.led.b)    {
    preferences.putInt("b", cur_state.led.b);
    Serial.println("save b = " + String(cur_state.led.b)); 
  }

  cur_state.IntervalNoMove_ms = cur_state.settings.IntervalNoMove * 1000;

  eeprom = cur_state;  
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  preferences.begin("settings", false);

  cur_state.btn.pin = PIN_BTN; pinMode(cur_state.btn.pin, INPUT);
  cur_state.move.pin = PIN_MOVE; pinMode(cur_state.move.pin, INPUT);
  cur_state.lux.interval_bh1750fvi = 500;

  cur_state.led.flag_on = false;
  cur_state.led.g = 255;
  cur_state.led.r = 255;
  cur_state.led.b = 255;
  cur_state.led.brightness = 100;

  cur_state.settings.flag_change = true;
  cur_state.settings.mode = 0;
  cur_state.settings.mode_pic = 1;

  bh1750fvi.begin();

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  read_eeprom();

  client.begin(true);
}

void onMsgCommand( const String &message ){
}

s_state Read_movesens( s_state state ){
  s_state ret = state;

  ret.move.state = digitalRead(ret.move.pin);

  if(ret.move.state == HIGH){
    ret.move_state_time = 1;
    move_save_time = millis( );
  } else {
    if(ret.move_state_time = 1) {
      if(millis() - move_save_time > ret.IntervalNoMove_ms) {
        ret.move_state_time = 0;
      }
    }
  }

  return ret;
}
s_state Read_state( ) {
  s_state ret = cur_state;

  ret.lux.lux =  bh1750fvi.get_last_value( );
  ret = Read_movesens( ret );
  ret.btn.state = digitalRead(ret.btn.pin);
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

  Brightness(state.led.brightness, ret.led.brightness, ret.settings.flag_change);

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
s_state Off( s_state state, boolean f_fl) {
  s_state ret = state;

  if (!ret.led.flag_on) return ret;

  Brightness(ret.led.brightness, 0, (ret.settings.flag_change && f_fl));

  ret.led.flag_on = false;

  return ret;
}
s_state On( s_state state, boolean f_fl ) {
  s_state ret = state;

  if (ret.led.flag_on) return ret;

  Brightness(0, ret.led.brightness, (ret.settings.flag_change && f_fl));

  ret.led.flag_on = true;
  return ret;
}

String get_settings( s_state State ) {
  String ret = "";
  DynamicJsonDocument doc(1024);

  doc["r"] = State.led.r;
  doc["g"] = State.led.g;
  doc["b"] = State.led.b;
  doc["brightness"] = State.led.brightness;
  doc["mode_pic"] = State.settings.mode_pic;
  doc["flag_change"] = State.settings.flag_change;
  doc["lux_off"] = State.settings.lux_off;
  doc["lux_on"] = State.settings.lux_on;
  doc["IntervalNoMove"] = State.settings.IntervalNoMove;

  serializeJson(doc, ret);
  return ret;
}
s_state read_settings(String msg){
  s_state ret = cur_state;

  if(msg.length( ) == 0) return ret;

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg);  

  ret.led.r = doc["r"];
  ret.led.g = doc["g"];
  ret.led.b = doc["b"];
  ret.led.brightness = doc["brightness"];
  ret.settings.mode_pic = doc["mode_pic"];
  ret.settings.flag_change = doc["flag_change"];
  ret.settings.lux_off = doc["lux_off"];
  ret.settings.lux_on = doc["lux_on"];
  ret.settings.IntervalNoMove = doc["IntervalNoMove"];
  ret.IntervalNoMove_ms = ret.settings.IntervalNoMove * 1000;

  return ret;
}

void report( s_state sState, int mode ) {
  if(mode == 0 || ( mode == 1 && sState.settings.mode != cur_state.settings.mode ) ){
    client.Publish("mode", String(sState.settings.mode));
  }
  if(mode == 0 || ( mode == 1 && sState.led.flag_on != cur_state.led.flag_on ) ){
    client.Publish("flag_on", String(sState.led.flag_on));
  }

  if(mode == 0 || ( mode == 1 && sState.move_state_time != cur_state.move_state_time ) ){
    client.Publish("move_state", String(sState.move_state_time));
  }

  if(mode == 0 || 
    ( mode == 1 &&
      ( sState.settings.flag_change != cur_state.settings.flag_change ||
        sState.settings.mode_pic != cur_state.settings.mode_pic ||
        sState.settings.lux_off != cur_state.settings.lux_off ||
        sState.settings.lux_on != cur_state.settings.lux_on ||
        sState.settings.IntervalNoMove != cur_state.settings.IntervalNoMove ||
        sState.led.brightness != cur_state.led.brightness ||
        sState.led.r != cur_state.led.r ||
        sState.led.g != cur_state.led.g ||
        sState.led.b != cur_state.led.b 
      )
    )
  ){
    client.Publish("settings", get_settings( sState ));
  }

  cur_state = sState;
  client.flag_start = false;
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
  bh1750fvi.loop( );

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
        state.move_state_time = 1;
        state.settings.mode = 0;
      } else {
        if(state.led.flag_on == 1) {
          state.settings.mode = 2;
        } else {
          state.settings.mode = 1;
        }
      }
      TimeSinceLastBtn = 0;
    }
    flagSinceLastBtn = false;
  }

  switch(state.settings.mode){
    case 1:
      state = On( state, state.settings.flag_change );
    break;
    
    case 2:
      state = Off( state, state.settings.flag_change );
    break;

    default:
      if(state.move_state_time == 1) {
        if(state.lux.lux <= state.settings.lux_on) {
          state = On( state, state.settings.flag_change );
        } else if(state.lux.lux > state.settings.lux_off) {
          state = Off( state, state.settings.flag_change );
        }
      } else {
        state = Off( state, state.settings.flag_change );
      }
  }

  if (client.flag_start) { //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void Msg_settings( const String &message ){
  cur_state = read_settings(message);
  write_eeprom();
}

void Msg_mode( const String &message ){
  cur_state.settings.mode = message.toInt( );
  if(cur_state.settings.mode < 0 || cur_state.settings.mode > 2) cur_state.settings.mode = 0;
  write_eeprom();
}

void Msg_alarm( const String &message ){
  cur_state.alarm = (message.toInt( ) != 0);
  if(!cur_state.alarm && cur_state.led.flag_on) {
    cur_state = Off(cur_state, false);
    cur_state = On(cur_state, false);
  }
}
void onConnection(){
  bh1750fvi.subscribe( ); //[def_path]/[def_subpath_bh1750fvi]/interval
  client.Subscribe("settings", Msg_settings); 
  client.Subscribe("mode", Msg_mode); 
  client.Subscribe("alarm", Msg_alarm); 
}

void new_rainbow_loop() {                      //-m88-RAINBOW FADE FROM FAST_SPI2
  ihue -= 1;
  fill_rainbow( leds, NUM_LEDS, ihue );
  LEDS.show();
  delay(thisdelay);
}

void OnLoad(){
}

byte baza = 0;
void rainbow_merz() {
  fill_rainbow( leds, NUM_LEDS, baza++, 7);
  if (random8() < 80) { leds[ random16(NUM_LEDS) ] += CRGB::White; }
  FastLED.setBrightness(cur_state.led.brightness);
  FastLED.show();
  delay(20);
}

uint8_t hue; 
void colorsRoutine(){
  hue += 10;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
  FastLED.show();
}

int AlarmMode;
void alarm(){
  if(!cur_state.led.flag_on) { 
    AlarmMode = cur_state.settings.mode;
    cur_state.settings.mode = 1;
  }
  colorsRoutine();
}

void loop() {
  client.loop();

  if(cur_state.alarm) {
    alarm();
  } else if(cur_state.led.flag_on){
    cur_state.settings.mode = AlarmMode;
    switch (cur_state.settings.mode_pic) {
      case 1:
        new_rainbow_loop();
        break;
      case 2:
        rainbow_merz();
        break;      
      default:
       SetColor(cur_state, cur_state.led.g, cur_state.led.r, cur_state.led.b);
       LEDS.show();
       break;
    }
  }
}