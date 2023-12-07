/*
для 8266 нужна ерсия ядра 2.5.2 и версия FastLed 3.2.9. иначе нифига не работает
*/

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#include <mqtt_ini.h>
#include <FastLED.h>
#include <Preferences.h>
#include <ArduinoJson.h>

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

uint32_t b_timer;
boolean b_started_change = false;
int b_old;
int b_value;

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
  uint8_t brightness; // = 100;
  boolean dimmered = true;;
  int stepDimmer = 1;
  int delayDimStep = 5;

  int numEffect = 0;

  s_def_effect e0;
};

struct s_state {
  s_settings ini;

  int onoff = 0;
  int cur_brightness;
  int  dir = 0;
  uint32_t dimmerTime;

  int oldEffect;
};

s_state cur_state;
s_settings eeprom;

//button btn_mode(PIN_MODE); // указываем пин

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

  cur_state.ini.dimmered = preferences.getBool("dimmered", true);
  Serial.println("read dimmered = " + String(cur_state.ini.dimmered));

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
    preferences.putBool("dimmered", cur_state.ini.dimmered);
    Serial.println("save dimmered = " + String(cur_state.ini.dimmered));
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

  b_value = cur_state.ini.brightness;

  preferences.begin("settings", false);

  read_eeprom();
  cur_state.onoff = 1;

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(0);
  FastLED.show();

  client.begin(false);
}

void onMsgCommand( const String &message ){

}

String Settings2String( s_settings State ) {
  String ret = "";
  DynamicJsonDocument doc(1024);

  doc["brightness"] = State.brightness;
  doc["dimmered"] = State.dimmered;
  doc["stepDimmer"] = State.stepDimmer;
  doc["delayDimStep"] = State.delayDimStep;
  doc["numEffect"] = State.numEffect;
  
  doc["e0"][0] = State.e0.r;
  doc["e0"][1] = State.e0.g;
  doc["e0"][2] = State.e0.b;

  doc["mode"][0] = modes[State.numEffect].speed;
  doc["mode"][1] = modes[State.numEffect].scale;

  serializeJson(doc, ret);
  return ret;
}

String Mode2String( s_mode mode ) {
  String ret = "";
  DynamicJsonDocument doc(1024);

  doc["speed"] = mode.speed;
  doc["scale"] = mode.scale;

  serializeJson(doc, ret);
  return ret;
}

s_settings String2Settings(String msg){
  s_settings ret = cur_state.ini;
  if(msg.length( ) == 0) return ret;

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg);  

  ret.brightness = doc["brightness"];
  ret.dimmered = doc["dimmered"];
  ret.stepDimmer = doc["stepDimmer"];
  ret.delayDimStep = doc["delayDimStep"];
  ret.numEffect = doc["numEffect"];
  if(ret.numEffect < 0 || ret.numEffect >= NUM_EFFECTS) {
    ret.numEffect = 0;
  }
  ret.e0.r = doc["e0"][0];
  ret.e0.g = doc["e0"][1];
  ret.e0.b = doc["e0"][2];
  
  return ret;
}

void String2Mode(String msg){
  if(msg.length( ) == 0) return;

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg);  

  modes[cur_state.ini.numEffect].speed = doc["speed"];
  modes[cur_state.ini.numEffect].scale = doc["scale"];
}

void reportMode(s_mode mode) {
    client.Publish("mode", Mode2String( mode));
}

void report( s_state s_State, int mode ) {
  s_state sState = s_State;

  if(mode == 0 || 
    ( mode == 1 &&
      ( sState.ini.brightness != cur_state.ini.brightness ||
       sState.ini.dimmered != cur_state.ini.dimmered ||
       sState.ini.stepDimmer != cur_state.ini.stepDimmer ||
       sState.ini.delayDimStep != cur_state.ini.delayDimStep ||
       sState.ini.numEffect != cur_state.ini.numEffect ||
       modes[sState.ini.numEffect].scale != modes[sState.ini.numEffect].old_scale ||
       modes[sState.ini.numEffect].speed != modes[sState.ini.numEffect].old_speed
      )
    )
  ){
    client.Publish("settings", Settings2String( sState.ini));
    reportMode(modes[sState.ini.numEffect]);
  }

  if(mode == 0 || 
    ( mode == 1 && sState.onoff != cur_state.onoff )
  ){
    client.Publish("onoff", String( sState.onoff ));
  }

  if(mode == 0 || 
    ( mode == 1 && sState.dir != cur_state.dir )
  ){
    client.Publish("dir", String( sState.dir ));
  }

  cur_state = sState;
  client.flag_start = false;
}

uint64_t bt;

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

    if(ret.ini.dimmered) {
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

  if(state.ini.numEffect < 0 || state.ini.numEffect >= NUM_EFFECTS) {
    state.ini.numEffect = 0;
  }  

  state = OnOff(state);

  if (client.flag_start) { //первый запуск
    report(state, 0); //отправляем все
  } else {
    report(state, 1); //отправляем только то, что изменилось
  }
}

void Msg_settings( const String &message ){
  int numEffect = cur_state.ini.numEffect;
  s_settings ini = ini = String2Settings(message);
  if(ini.e0.r != cur_state.ini.e0.r || ini.e0.g != cur_state.ini.e0.g || ini.e0.b != cur_state.ini.e0.b ) {
    ini.e0.done = false;
  }

  cur_state.ini = ini;
  write_eeprom();  
  if(numEffect != cur_state.ini.numEffect) {
      reportMode(modes[cur_state.ini.numEffect]);
  }
}

void Msg_settings_mode( const String &message ){
  String2Mode(message);
  reportMode(modes[cur_state.ini.numEffect]);
  write_eeprom();  
}

void Msg_onoff( const String &message ){
  cur_state.onoff = message.toInt( );
  Serial.println("onoff = " + String(cur_state.onoff));
}

void onConnection(){
  client.Subscribe("settings", Msg_settings); 
  client.Subscribe("mode", Msg_settings_mode); 
  client.Subscribe("onoff", Msg_onoff); 
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