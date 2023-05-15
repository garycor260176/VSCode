#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <Wire.h>
#include <mqtt_ini.h>
#include <FastLED.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "SparkFun_VL53L1X.h"

Preferences preferences; //хранение текущего состояния

#define def_path "kitchen"

#define PIN_LED               19

#define NUM_LEDS              216
#define delayFL 5
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 25000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define CNT_MODES 1

mqtt_ini client( 
  "kitchen",     // Client name that uniquely identify your device
   def_path);

struct s_led {
  uint8_t g; // = 0;
  uint8_t r; // = 0;
  uint8_t b; // = 255;
  uint8_t brightness; // = 100;
  boolean flag_on; // = false;
};

struct s_settings{
  boolean flag_change; // = false;
  int mode; //0-авто, 1-on, 2-off
  int DISTANCE_PUSH;
};


struct s_state {
  s_settings settings;

  int btn_state;

  boolean alarm;
  boolean on_by_alarm;
  boolean setted_color;

  s_led led;
};

#define INTERVAL_MEASURE  100
SFEVL53L1X distanceSensor;
static int time_budget_in_ms_long = 50;

s_state cur_state;
s_state eeprom;

int getDist(byte trig, byte echo);
void applay_mode(s_state state);

void read_eeprom(){
  cur_state.settings.flag_change = preferences.getBool("flag_change", true);
  Serial.println("read flag_change = " + String((cur_state.settings.flag_change ? 1 : 0)));

  cur_state.settings.mode = 2;

  cur_state.settings.DISTANCE_PUSH = preferences.getInt("DISTANCE_PUSH", 80);
  if(cur_state.settings.DISTANCE_PUSH <= 0 || cur_state.settings.DISTANCE_PUSH > 2000) cur_state.settings.DISTANCE_PUSH = 80;
  Serial.println("read DISTANCE_PUSH = " + String(cur_state.settings.DISTANCE_PUSH));

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
    preferences.putBool("flag_change", cur_state.settings.flag_change);
    Serial.println("save flag_change = " + String((cur_state.settings.flag_change ? 1 : 0)));
  }

  if(cur_state.settings.DISTANCE_PUSH != eeprom.settings.DISTANCE_PUSH)    {
    preferences.putInt("DISTANCE_PUSH", cur_state.settings.DISTANCE_PUSH);
    Serial.println("save DISTANCE_PUSH = " + String(cur_state.settings.DISTANCE_PUSH));
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

  eeprom = cur_state;  
}

void setup() {
  Wire.begin();
  Wire.setClock(400000);

  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  if (distanceSensor.begin() != 0) {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while(1) ;
  }
  distanceSensor.setIntermeasurementPeriod(time_budget_in_ms_long);
  distanceSensor.setDistanceModeLong();  
  Serial.println("Sensor online!");
  
  preferences.begin("settings", false);

  cur_state.led.flag_on = false;
  cur_state.led.g = 255;
  cur_state.led.r = 255;
  cur_state.led.b = 255;
  cur_state.led.brightness = 50;

  cur_state.settings.flag_change = true;
  cur_state.settings.mode = 2;

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  read_eeprom();

  client.begin(true);
}

void onMsgCommand( const String &message ){}

void Brightness(uint8_t from, uint8_t to, boolean f_fl = true){
  int f = from;
  int t = to;
  int step = 1;
  if(f_fl){
    if(from <= to) {
      for(int i = from; i <= to; i += step){
        FastLED.setBrightness(i);
        FastLED.show();
        delay(delayFL);
      }
    } else {
      for(int i = to; i <= from; i -= step){
        FastLED.setBrightness(i);
        FastLED.show();
        delay(delayFL);
      }
    }
  } else {
    FastLED.setBrightness(t);
    FastLED.show();
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

void SetCol(int r, int g, int b){
  for (int i = 0 ; i < NUM_LEDS; i++ )
  {
    leds[i].setRGB( r, g, b);
  }
  LEDS.show();
}

s_state SetColor( s_state state, uint8_t g, uint8_t r, uint8_t b){
  s_state ret = state;
  ret.setted_color = true;
  ret.led.g = g;  ret.led.r = r;  ret.led.b = b;
  SetCol(ret.led.r, ret.led.g, ret.led.b);
  return ret;
}

s_state Off( s_state state, boolean f_fl) {
  s_state ret = state;

  if (!ret.led.flag_on) return ret;

  FastLED.clear();
  FastLED.show();
//  Brightness(ret.led.brightness, 0, (ret.settings.flag_change && f_fl));
  ret.led.flag_on = false;
  return ret;
}
s_state On( s_state state, boolean f_fl ) {
  s_state ret = state;

  if(ret.led.flag_on) return ret;

  Brightness(0, ret.led.brightness, (ret.settings.flag_change && f_fl));

  ret.led.flag_on = true;
  return ret;
}

int getDist() {
    distanceSensor.startRanging();
    int dist = distanceSensor.getDistance();
    distanceSensor.clearInterrupt();
    distanceSensor.stopRanging();
    return dist;
}

String get_settings( s_state State ) {
  String ret = "";
  DynamicJsonDocument doc(1024);

  doc["r"] = State.led.r;
  doc["g"] = State.led.g;
  doc["b"] = State.led.b;
  doc["brightness"] = State.led.brightness;
  doc["DISTANCE_PUSH"] = State.settings.DISTANCE_PUSH;
  doc["flag_change"] = State.settings.flag_change ? 1 : 0;

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
  ret.settings.flag_change = doc["flag_change"] == 1 ? true : false;
  
  ret.settings.DISTANCE_PUSH = doc["DISTANCE_PUSH"];
  if(ret.settings.DISTANCE_PUSH <= 0 || ret.settings.DISTANCE_PUSH > 2000) ret.settings.DISTANCE_PUSH = 80;

  return ret;
}

void report( s_state sState, int mode ) {
  if(mode == 0 || ( mode == 1 && sState.settings.mode != cur_state.settings.mode ) ){
    client.Publish("mode", String(sState.settings.mode));
  }
  if(mode == 0 || ( mode == 1 && sState.led.flag_on != cur_state.led.flag_on ) ){
    client.Publish("flag_on", String((sState.led.flag_on ? 1 : 0)));
  }

  if(mode == 0 || 
    ( mode == 1 &&
      ( sState.settings.flag_change != cur_state.settings.flag_change ||
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

void Msg_settings( const String &message ){
  s_state state = read_settings(message);
  applay_mode(state);
}

void applay_mode(s_state state){
  if(!state.alarm && state.led.flag_on) {
    state = Off(state, false);
    state = On(state, false);
  }
  cur_state = state;
  write_eeprom();
}

void Msg_mode( const String &message ){
  s_state state = cur_state;
  state.settings.mode = message.toInt( );
  applay_mode(state);
}

void Msg_alarm( const String &message ){
  cur_state.alarm = (message.toInt( ) != 0);
}

void onConnection(){
  client.Subscribe("settings", Msg_settings); 
  client.Subscribe("mode", Msg_mode); 
  client.Subscribe("alarm", Msg_alarm); 
}

void OnLoad(){
}

uint8_t hue; 
void colorsRoutine(){
  hue += 10;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
  FastLED.show();
}

void alarm(){
  colorsRoutine();
}

// медианный фильтр
int getFilterMedian(int newVal) {
  static int buf[3];
  static byte count = 0;
  buf[count] = newVal;
  if (++count >= 3) count = 0;
  return (max(buf[0], buf[1]) == max(buf[1], buf[2])) ? max(buf[0], buf[2]) : max(buf[1], min(buf[0], buf[2]));
}
// пропускающий фильтр
#define FS_WINDOW 7   // количество измерений, в течение которого значение не будет меняться
#define FS_DIFF 80    // разница измерений, с которой начинается пропуск
int getFilterSkip(int val) {
  static int prev;
  static byte count;
  if (!prev && val) prev = val;   // предыдущее значение 0, а текущее нет. Обновляем предыдущее
  // позволит фильтру резко срабатывать на появление руки
  // разница больше указанной ИЛИ значение равно 0 (цель пропала)
  if (abs(prev - val) > FS_DIFF || !val) {
    count++;
    // счётчик потенциально неправильных измерений
    if (count > FS_WINDOW) {
      prev = val;
      count = 0;
    } else val = prev;
  } else count = 0;   // сброс счётчика
  prev = val;
  
  return val;
}

s_state Read_state( ) {
  s_state ret = cur_state;

  static uint64_t t;
  if(millis( ) - t > INTERVAL_MEASURE) {
    int dist = getDist( );
    //dist = getFilterMedian(dist);         // медиана
    //dist = getFilterSkip(dist);           // пропускающий фильтр
    ret.btn_state = ( dist > 0 && dist <= ret.settings.DISTANCE_PUSH ? 1 : 0 );
    if(ret.btn_state != cur_state.btn_state) {
client.Publish("dist", String(dist));
    }
    t = millis();
  }

  return ret;
}

void OnCheckState(){
  s_state state = Read_state();

  static uint64_t TimeSinceLastBtn;
  static boolean flagSinceLastBtn;

  if(state.btn_state == HIGH){
    if(!flagSinceLastBtn){
      flagSinceLastBtn = true;
      TimeSinceLastBtn = millis( );
    }
  } else {
    if(flagSinceLastBtn){
      if( millis( ) - TimeSinceLastBtn >= 300){
client.Publish("btn", "1");
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
    
    default:
      state = Off( state, state.settings.flag_change );
    break;
  }  

  if (client.flag_start) { //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем все
  }
}

void loop() {
  client.loop();

  if(cur_state.alarm) {
    if(!cur_state.led.flag_on) {
      cur_state = On(cur_state, false);
      cur_state.on_by_alarm = true;
    }
    alarm();    
  } else {
    if(cur_state.on_by_alarm) {
      cur_state.on_by_alarm = false;
      cur_state = Off(cur_state, false);
    }

    if(cur_state.led.flag_on) {
      if(cur_state.settings.mode == 1) {
        if(!cur_state.setted_color) SetColor(cur_state, cur_state.led.g, cur_state.led.r, cur_state.led.b);
      }
    }
  }
}
