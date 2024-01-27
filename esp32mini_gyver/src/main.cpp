/*
для 8266 нужна ерсия ядра 2.5.2 и версия FastLed 3.2.9. иначе нифига не работает
*/

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#include <mqtt_ini.h>
#include <FastLED.h>
#include <Preferences.h>

#define def_path "L_gyver"
#define PIN_LED   23

#define NUM_EFFECTS   34
#define WIDTH         16              // ширина матрицы
#define HEIGHT        16             // высота матрицы
#define NUM_LEDS      WIDTH * HEIGHT
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная

CRGB leds[NUM_LEDS];

#define SPEED_DEFAULT 30
#define SCALE_DEFAULT 40

struct s_mode {
  byte speed = SPEED_DEFAULT;
  byte scale = SCALE_DEFAULT;

  byte old_speed = SPEED_DEFAULT;
  byte old_scale = SCALE_DEFAULT;
} modes[NUM_EFFECTS];

#include "effects.h"

boolean started = false;

mqtt_ini client( 
  "L_gyver",     // Client name that uniquely identify your device
   def_path);

Preferences preferences; //хранение текущего состояния

struct s_def_effect {
  int r = 0;
  int g = 0;
  int b = 255;

  boolean done = false;
};

struct s_settings {
  int brightness;

  int dimmered; 
  
  int stepDimmer;

  int delayDimStep;

  int numEffect;

  s_def_effect e0;

  int demo_mode;
  uint32_t demo_mode_time;
};

struct s_state {
  s_settings ini;

  int onoff;
  int old_onoff;

  int cur_brightness;
  int  dir = 0;
  uint32_t dimmerTime;
  int oldEffect;
};

uint32_t last_change_mode;

s_state cur_state;
s_settings eeprom;

void report( s_state sState, int mode );

void read_eeprom(){
  cur_state.ini.e0.r = preferences.getInt("e0_r", 0);
  if(cur_state.ini.e0.r < 0 || cur_state.ini.e0.r > 255){
    cur_state.ini.e0.r = 50;
  }
  Serial.println("read e0.r = " + String(cur_state.ini.e0.r));

  cur_state.ini.e0.g = preferences.getInt("e0_g", 0);
  if(cur_state.ini.e0.g < 0 || cur_state.ini.e0.g > 255){
    cur_state.ini.e0.g = 50;
  }
  Serial.println("read e0.g = " + String(cur_state.ini.e0.g));

  cur_state.ini.e0.b = preferences.getInt("e0_b", 255);
  if(cur_state.ini.e0.b < 0 || cur_state.ini.e0.b > 255){
    cur_state.ini.e0.b = 50;
  }
  Serial.println("read e0.b = " + String(cur_state.ini.e0.b));

  cur_state.ini.brightness = preferences.getInt("brightness", 50);
  if(cur_state.ini.brightness < 0 || cur_state.ini.brightness > 255){
    cur_state.ini.brightness = 50;
  }
  Serial.println("read brightness = " + String(cur_state.ini.brightness));

  cur_state.ini.numEffect = preferences.getInt("numEffect", 0);
  if(cur_state.ini.numEffect < 0 || cur_state.ini.numEffect >= NUM_EFFECTS){
    cur_state.ini.numEffect = 0;
  }
  Serial.println("read numEffect = " + String(cur_state.ini.numEffect));  
  
  cur_state.ini.stepDimmer = preferences.getInt("stepDimmer", 1);
  if(cur_state.ini.stepDimmer < 0 || cur_state.ini.stepDimmer > 255){
    cur_state.ini.stepDimmer = 1;
  }
  Serial.println("read stepDimmer = " + String(cur_state.ini.stepDimmer));

  cur_state.ini.delayDimStep = preferences.getInt("delayDimStep", 5);
  if(cur_state.ini.delayDimStep < 0){
    cur_state.ini.delayDimStep = 5;
  }
  Serial.println("read delayDimStep = " + String(cur_state.ini.delayDimStep));

  cur_state.ini.dimmered = preferences.getInt("dimmered", 1);
  if(cur_state.ini.dimmered < 0) cur_state.ini.dimmered = 0;
  if(cur_state.ini.dimmered > 1) cur_state.ini.dimmered = 1;
  Serial.println("read dimmered = " + String(cur_state.ini.dimmered));

  cur_state.ini.demo_mode = preferences.getInt("demo_mode", 1);
  if(cur_state.ini.demo_mode < 0) cur_state.ini.demo_mode = 0;
  if(cur_state.ini.demo_mode > 1) cur_state.ini.demo_mode = 1;
  Serial.println("read demo_mode = " + String(cur_state.ini.demo_mode));

  cur_state.ini.demo_mode_time = preferences.getUInt("demo_modet", 10);
  if(cur_state.ini.demo_mode_time <= 0) cur_state.ini.demo_mode_time = 10;
  Serial.println("read demo_modet = " + String(cur_state.ini.demo_mode_time));

  String str;
  for(int i = 0; i < NUM_EFFECTS; i++) {
    str = "m" + String(i) + "scale";
    modes[i].scale = modes[i].old_scale = preferences.getInt(str.c_str(), SCALE_DEFAULT);
    str = "m" + String(i) + "speed";
    modes[i].speed = modes[i].old_speed = preferences.getInt(str.c_str(), SPEED_DEFAULT);

    Serial.println("read modes[" + String(i) + "] = [scale = " + String(modes[i].scale) + ", speed = " + String(modes[i].speed) + "]");
  }

  eeprom = cur_state.ini;
}
void write_eeprom(){
  if(cur_state.ini.e0.r != eeprom.e0.r)    {
    preferences.putInt("e0_r", cur_state.ini.e0.r);
    Serial.println("save e0.r = " + String(cur_state.ini.e0.r));
  }

  if(cur_state.ini.e0.g != eeprom.e0.g)    {
    preferences.putInt("e0_g", cur_state.ini.e0.g);
    Serial.println("save e0.g = " + String(cur_state.ini.e0.g));
  }

  if(cur_state.ini.e0.b != eeprom.e0.b)    {
    preferences.putInt("e0_b", cur_state.ini.e0.b);
    Serial.println("save e0.b = " + String(cur_state.ini.e0.b));
  }

  if(cur_state.ini.brightness != eeprom.brightness)    {
    preferences.putInt("brightness", cur_state.ini.brightness);
    Serial.println("save brightness = " + String(cur_state.ini.brightness));
  }

  if(cur_state.ini.numEffect != eeprom.numEffect)    {
    preferences.putInt("numEffect", cur_state.ini.numEffect);
    Serial.println("save numEffect = " + String(cur_state.ini.numEffect));
  }

  if(cur_state.ini.stepDimmer != eeprom.stepDimmer)    {
    preferences.putInt("stepDimmer", cur_state.ini.stepDimmer);
    Serial.println("save stepDimmer = " + String(cur_state.ini.stepDimmer));
  }

  if(cur_state.ini.delayDimStep != eeprom.delayDimStep)    {
    preferences.putInt("delayDimStep", cur_state.ini.delayDimStep);
    Serial.println("save delayDimStep = " + String(cur_state.ini.delayDimStep));
  }

  if(cur_state.ini.dimmered != eeprom.dimmered)    {
    preferences.putInt("dimmered", cur_state.ini.dimmered);
    Serial.println("save dimmered = " + String(cur_state.ini.dimmered));
  }

  if(cur_state.ini.demo_mode != eeprom.demo_mode)    {
    preferences.putInt("demo_mode", cur_state.ini.demo_mode);
    Serial.println("save demo_mode = " + String(cur_state.ini.demo_mode));
  }

  if(cur_state.ini.demo_mode_time != eeprom.demo_mode_time)    {
    preferences.putUInt("demo_modet", cur_state.ini.demo_mode_time);
    Serial.println("save demo_modet = " + String(cur_state.ini.demo_mode_time));
  }

  String str;
  for(int i = 0; i < NUM_EFFECTS; i++) {
    if(modes[i].scale != modes[i].old_scale) {
      str = "m" + String(i) + "scale";

      modes[i].old_scale = modes[i].scale;
      preferences.putInt(str.c_str( ), modes[i].scale);
 
      Serial.println("save modes[" + String(i) + "].scale = " + String(modes[i].scale));
    }
    if(modes[i].speed != modes[i].old_speed) {
      str = "m" + String(i) + "speed";

      modes[i].old_speed = modes[i].speed;
      preferences.putInt(str.c_str( ), modes[i].speed);
 
      Serial.println("save modes[" + String(i) + "].speed = " + String(modes[i].speed));
    }
  }  
  eeprom = cur_state.ini;
}

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  preferences.begin("settings", false);

  read_eeprom();
  cur_state.old_onoff = cur_state.onoff = 1;

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(0);
  FastLED.show();

  client.begin(true);
}

void onMsgCommand( const String &message ){

}

void report( s_state s_State, int mode ) {
  s_state sState = s_State;

  if(mode == 0 || ( mode == 1 && sState.onoff != sState.old_onoff )){
    client.Publish("onoff", String(sState.onoff));
  }

  if(mode == 0 || ( mode == 1 && sState.ini.brightness != eeprom.brightness )){
    client.Publish("settings/brightness", String(sState.ini.brightness));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.dimmered != eeprom.dimmered )){
    client.Publish("settings/dimmered", String(sState.ini.dimmered));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.demo_mode != eeprom.demo_mode )){
    client.Publish("settings/demo_mode", String(sState.ini.demo_mode));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.demo_mode_time != eeprom.demo_mode_time )){
    client.Publish("settings/demo_mode_time", String(sState.ini.demo_mode_time));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.stepDimmer != eeprom.stepDimmer )){
    client.Publish("settings/stepDimmer", String(sState.ini.stepDimmer));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.delayDimStep != eeprom.delayDimStep )){
    client.Publish("settings/delayDimStep", String(sState.ini.delayDimStep));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.numEffect != eeprom.numEffect )){
    client.Publish("settings/numEffect", String(sState.ini.numEffect));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.e0.r != eeprom.e0.r )){
    client.Publish("settings/color/r", String(sState.ini.e0.r));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.e0.g != eeprom.e0.g )){
    client.Publish("settings/color/g", String(sState.ini.e0.g));
  }
  if(mode == 0 || ( mode == 1 && sState.ini.e0.b != eeprom.e0.b )){
    client.Publish("settings/color/b", String(sState.ini.e0.b));
  }
  boolean changedNum = sState.ini.numEffect != eeprom.numEffect;
  if(mode == 0 || ( mode == 1 && ( modes[sState.ini.numEffect].scale != modes[sState.ini.numEffect].old_scale || changedNum))){
    client.Publish("settings/scale", String(modes[sState.ini.numEffect].scale));
  }
  if(mode == 0 || ( mode == 1 && ( modes[sState.ini.numEffect].speed != modes[sState.ini.numEffect].old_speed || changedNum))){
    client.Publish("settings/speed", String(modes[sState.ini.numEffect].speed));
  }

  cur_state = sState;

  write_eeprom( );

  cur_state.ini = eeprom;
  cur_state.old_onoff = cur_state.onoff;
  modes[sState.ini.numEffect].old_scale = modes[sState.ini.numEffect].scale;
  modes[sState.ini.numEffect].old_speed = modes[sState.ini.numEffect].speed;

  client.flag_start = false;
}

s_state Read_state( ) {
  s_state ret = cur_state;

  return ret;
}

void setColor(s_def_effect e){
  for(int i = 0; i < NUM_LEDS; i++) leds[i].setRGB(e.r, e.g, e.b);
}

s_state drawEffect(s_state state) {
  s_state ret = state;

  if(ret.ini.numEffect > 0 && ret.ini.numEffect < NUM_EFFECTS) {
    ret.ini.e0.done = false;
  } else {
    if(!ret.ini.e0.done) {
      ret.ini.e0.done = true;
      setColor(ret.ini.e0);
    }
    return ret;
  }

  showEffect( ret.ini.numEffect );

  return ret;
}

s_state OnOff(s_state state) {
  s_state ret = state;

  if(ret.onoff == 1) {
      if(ret.cur_brightness < ret.ini.brightness) {
        if(ret.dir != 1) {
          ret.dimmerTime = millis();
          ret.dir = 1;
        }
      } else if(ret.cur_brightness > ret.ini.brightness) {
        if(ret.dir != 2) {
          ret.dimmerTime = millis();
          ret.dir = 2;
        }
      } else {
        ret.dir = 0;
      }
  } else if(ret.onoff == 0 && ret.cur_brightness > 0) {
    if(ret.dir != 2) {
      ret.dimmerTime = millis();
      ret.dir = 2;
    } 
  } else {
    ret.dir = 0;
  }

  if(ret.onoff != ret.old_onoff) {
    last_change_mode = millis();
  }

  if(ret.onoff == 1) {
    if(ret.ini.demo_mode == 1) {
      if(millis() - last_change_mode > ret.ini.demo_mode_time * 1000) {
        ret.ini.numEffect++;
        if(ret.ini.numEffect >= NUM_EFFECTS) ret.ini.numEffect = 0;
        last_change_mode = millis();
      }
    } else {
      last_change_mode = millis( );
    }
  }

  if(ret.cur_brightness > 0) {
    if(ret.oldEffect != ret.ini.numEffect) {
      loadingFlag = true;
      FastLED.clear();
    }
    ret = drawEffect(ret);
    ret.oldEffect = ret.ini.numEffect;
  }

  if(ret.dir != 0) {
    int br = ret.cur_brightness;

    if(ret.ini.dimmered == 1) {
      if(millis() - ret.dimmerTime > ret.ini.delayDimStep) {
        if(ret.dir == 1) {
          br += ret.ini.stepDimmer;
          if(br > ret.ini.brightness) br = ret.ini.brightness;
        } else {
          br -= ret.ini.stepDimmer;
          if(br < 0) br = 0;
        }
        ret.dimmerTime = millis();
      }
    } else {
        br = (ret.dir == 1 ? ret.ini.brightness : 0);
    }
    if(br != ret.cur_brightness) {
        FastLED.setBrightness(br);
    }

    ret.cur_brightness = br;

    if(ret.cur_brightness == (ret.dir == 1 ? ret.ini.brightness : 0)) {
        ret.dir = 0;
    }
  }

  FastLED.show();

  return ret;
}

void OnCheckState(){
  s_state state = Read_state( );

  state = OnOff(state);

  if (client.flag_start) { //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void Msg_onoff( const String &message ){
  cur_state.onoff = message.toInt( );
  if(cur_state.onoff < 0 || cur_state.onoff > 1) cur_state.onoff = 0;
  if(cur_state.old_onoff == cur_state.onoff) return;
  cur_state.old_onoff = cur_state.onoff;
  Serial.println("onoff = " + String(cur_state.onoff));
}
void Msg_brightness( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 255) val = 255;
  if(cur_state.ini.brightness == val) return;
  cur_state.ini.brightness = val;
  Serial.println("brightness = " + String(cur_state.ini.brightness));
}
void Msg_dimmered( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 1) val = 1;
  if(cur_state.ini.dimmered == val) return;
  cur_state.ini.dimmered = val;
  Serial.println("dimmered = " + String(cur_state.ini.dimmered));
}
void Msg_stepDimmer( const String &message ){
  int val = message.toInt( );
  if(val < 1 ) val = 1;
  if(val > 255) val = 255;
  if(cur_state.ini.stepDimmer == val) return;
  cur_state.ini.stepDimmer = val;
  Serial.println("stepDimmer = " + String(cur_state.ini.stepDimmer));
}
void Msg_delayDimStep( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(cur_state.ini.delayDimStep == val) return;
  cur_state.ini.delayDimStep = val;
  Serial.println("delayDimStep = " + String(cur_state.ini.delayDimStep));
}
void Msg_numEffect( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val >= NUM_EFFECTS) val = NUM_EFFECTS - 1;
  if(cur_state.ini.numEffect == val) return;
  cur_state.ini.numEffect = val;
  Serial.println("numEffect = " + String(cur_state.ini.numEffect));
}
void Msg_r( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 255) val = 255;
  if(cur_state.ini.e0.r == val) return;
  cur_state.ini.e0.r = val;
  Serial.println("r = " + String(cur_state.ini.e0.r));
}
void Msg_g( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 255) val = 255;
  if(cur_state.ini.e0.g == val) return;
  cur_state.ini.e0.g = val;
  Serial.println("g = " + String(cur_state.ini.e0.g));
}
void Msg_b( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 255) val = 255;
  if(cur_state.ini.e0.b == val) return;
  cur_state.ini.e0.b = val;
  Serial.println("b = " + String(cur_state.ini.e0.b));
}
void Msg_settings_scale( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 255) val = 255;
  if(modes[cur_state.ini.numEffect].scale == val) return;
  modes[cur_state.ini.numEffect].scale = val;
  Serial.println("scale = " + String(modes[cur_state.ini.numEffect].scale));
}
void Msg_settings_speed( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 255) val = 255;
  if(modes[cur_state.ini.numEffect].speed == val) return;
  modes[cur_state.ini.numEffect].speed = val;
  Serial.println("speed = " + String(modes[cur_state.ini.numEffect].speed));
}
void Msg_demo_mode( const String &message ){
  int val = message.toInt( );
  if(val < 0 ) val = 0;
  if(val > 1) val = 1;
  if(cur_state.ini.demo_mode == val) return;
  cur_state.ini.demo_mode = val;
  Serial.println("demo_mode = " + String(cur_state.ini.demo_mode));
}
void Msg_demo_mode_time( const String &message ){
  int val = message.toInt( );
  if(val <= 0 ) val = 10;
  if(cur_state.ini.demo_mode_time == val) return;
  cur_state.ini.demo_mode_time = val;
  Serial.println("demo_mode_time = " + String(cur_state.ini.demo_mode_time));
}
void onConnection(){
  client.Subscribe("onoff", Msg_onoff); 

  client.Subscribe("settings/brightness", Msg_brightness); 
  client.Subscribe("settings/dimmered", Msg_dimmered); 
  client.Subscribe("settings/demo_mode", Msg_demo_mode); 
  client.Subscribe("settings/demo_mode_time", Msg_demo_mode_time); 
  client.Subscribe("settings/stepDimmer", Msg_stepDimmer); 
  client.Subscribe("settings/delayDimStep", Msg_delayDimStep); 
  client.Subscribe("settings/numEffect", Msg_numEffect); 
  client.Subscribe("settings/color/r", Msg_r); 
  client.Subscribe("settings/color/g", Msg_g); 
  client.Subscribe("settings/color/b", Msg_b); 

  client.Subscribe("onoff", Msg_onoff); 

  client.Subscribe("settings/scale", Msg_settings_scale); 
  client.Subscribe("settings/speed", Msg_settings_speed); 
}

void OnLoad(){
}

void loop() {
  if(!started) {
    FastLED.setBrightness(0);
    FastLED.clear( );
    FastLED.show( );
    started = true;
  }

  client.loop();
}