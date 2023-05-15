#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <mqtt_ini.h>
#include <FastLED.h>
#include <Preferences.h>

#define def_path "lethlamp"

#define FLAG_SEND_LED         2

#define PIN_LED          D6
#define PIN_BTN           D7

#define NUM_LEDS              144
#define delayFL 5
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 1000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

struct s_ws2812b{
  int r = 0;
  int g = 0;
  int b = 255;
  int brightness = 100;
  boolean flag_change = true;
  boolean state;
  int step = 5;
};

struct s_state {
  s_ws2812b rgb;
  int alarm;
  int btn_state;
  boolean alarmed;
};

Preferences preferences; //хранение текущего состояния

mqtt_ini client( 
  "ESP8266_LETHLAMP",     // Client name that uniquely identify your device
   def_path);

void read_eeprom(struct s_state* state);
void write_eeprom(struct s_state* state);
void FastLedShow();
void SetColor( uint8_t g, uint8_t r, uint8_t b);

s_state cur_state;

void setup() {
  Serial.begin(115200);

  preferences.begin("ini"); // use "my-app" namespace

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.setBrightness(0);
  FastLED.show();

  pinMode(PIN_BTN, INPUT);

  read_eeprom(&cur_state);
  SetColor(cur_state.rgb.r, cur_state.rgb.g, cur_state.rgb.b);
  FastLED.show();

  client.begin(true);
}

void FastLedShow(){
  FastLED.show();  
}

void read_eeprom(struct s_state* state){
//RGB
  state->rgb.brightness = preferences.getInt("r_br", 50);
  if(state->rgb.brightness < 0 || state->rgb.brightness > 255) state->rgb.brightness = 50;
  Serial.println("read rgb_brightness = " + String(state->rgb.brightness));

  state->rgb.flag_change = preferences.getBool("r_fc", true);
  Serial.println("read rgb_flag_change = " + String(state->rgb.flag_change));

  state->rgb.step = preferences.getInt("r_step", 5);
  Serial.println("read rgb_step = " + String(state->rgb.step));

  state->rgb.r = preferences.getInt("r_r", 0);
  if(state->rgb.r < 0 || state->rgb.r > 255) state->rgb.r = 50;
  Serial.println("read rgb_red = " + String(state->rgb.r));

  state->rgb.g = preferences.getInt("r_g", 0);
  if(state->rgb.g < 0 || state->rgb.g > 255) state->rgb.g = 50;
  Serial.println("read rgb_green = " + String(state->rgb.g));

  state->rgb.b = preferences.getInt("r_b", 255);
  if(state->rgb.b < 0 || state->rgb.b > 255) state->rgb.b = 50;
  Serial.println("read rgb_blue = " + String(state->rgb.b));
}

void write_eeprom(struct s_state* state){
//RGB
  if(state->rgb.brightness != cur_state.rgb.brightness)    {
    preferences.putInt("r_br", state->rgb.brightness);
    Serial.println("save rgb_brightness = " + String(state->rgb.brightness));
  }
  if(state->rgb.flag_change != cur_state.rgb.flag_change)    {
    preferences.putBool("r_fc", state->rgb.flag_change);
    Serial.println("save rgb_flag_change = " + String(state->rgb.flag_change));
  }
  if(state->rgb.step != cur_state.rgb.step)    {
    preferences.putInt("r_step", state->rgb.step);
    Serial.println("save rgb_step = " + String(state->rgb.step));
  }
  if(state->rgb.r != cur_state.rgb.r)    {
    preferences.putInt("r_r", state->rgb.r);
    Serial.println("save rgb_red = " + String(state->rgb.r));
  }
  if(state->rgb.g != cur_state.rgb.g)    {
    preferences.putInt("r_g", state->rgb.g);
    Serial.println("save rgb_green = " + String(state->rgb.g));
  }
  if(state->rgb.b != cur_state.rgb.b)    {
    preferences.putInt("r_b", state->rgb.b);
    Serial.println("save rgb_blue = " + String(state->rgb.b));
  }
}

void onMsgCommand( const String &message ){}

void OnLoad(){}

void Msg_ini(const String &topicStr, const String &message) {
  s_state state = cur_state;

  Serial.println("topic = " + topicStr + ", msg = " + message);
  String topic = def_path; topic = topic + "/ini/rgb/";
  if(topic + "brightness" == topicStr) {
    state.rgb.brightness = message.toInt();
    if(state.rgb.brightness < 0) state.rgb.brightness = 0;
    if(state.rgb.brightness > 255) state.rgb.brightness = 255;
  }
  if(topic + "r" == topicStr) {
    state.rgb.r = message.toInt();
    if(state.rgb.r < 0) state.rgb.r = 0;
    if(state.rgb.r > 255) state.rgb.r = 255;
  }
  if(topic + "g" == topicStr) {
    state.rgb.g = message.toInt();
    if(state.rgb.g < 0) state.rgb.g = 0;
    if(state.rgb.g > 255) state.rgb.g = 255;
  }
  if(topic + "b" == topicStr) {
    state.rgb.b = message.toInt();
    if(state.rgb.b < 0) state.rgb.b = 0;
    if(state.rgb.b > 255) state.rgb.b = 255;
  }
  if(topic + "step" == topicStr) {
    state.rgb.step = message.toInt();
    if(state.rgb.step < 0) state.rgb.step = 0;
    if(state.rgb.step > 255) state.rgb.step = 255;
  }
  if(topic + "flag_change" == topicStr) {
    state.rgb.flag_change = (message.toInt() == 1);
  }

  write_eeprom(&state);
  cur_state = state;
  SetColor(cur_state.rgb.r, cur_state.rgb.g, cur_state.rgb.b);
  FastLedShow();
}

void Msg_rgb_state( const String &message ){
  cur_state.rgb.state = (message == "1");
}

void Msg_alarm( const String &message ){
  int alarm = cur_state.alarm;
  cur_state.alarm = message.toInt();
  cur_state.alarm = (cur_state.alarm == 1);

  if(alarm == cur_state.alarm) return;

  if(cur_state.alarm > 0){
    switch(cur_state.alarm) {
      case 1:
        FastLED.clear(true);
        SetColor(255, 0, 0);
        FastLED.setBrightness(255);
        FastLedShow();  
      break;
    }
  }
}

void onConnection(){
  client.Subscribe("ini/#", Msg_ini); 
  client.Subscribe("rgb_state", Msg_rgb_state); 
  client.Subscribe("alarm", Msg_alarm); 
}

boolean rgbSetBrightness(int from, int to, int step, boolean fc){
  boolean ret = false;

  const int rgb_t_delay = 30;
  static uint64_t rgb_m_delay;

  int br = FastLED.getBrightness();

  if(to >= from) {
    if(br >= to) {
      return ret;
    }
  } else {
    if(br <= to) {
      return ret;
    }
  }

  if(fc) {
    if(millis() - rgb_m_delay <= rgb_t_delay) return ret;
    rgb_m_delay = millis();

    if(from > to) step = -step;
    br += step;
    if(to > from) {
      if(br > to) br = to;
    } else {
      if(br < to) br = to;
    }
  } else {
    br = to;
  }
  if(br < 0) br = 0;
  if(br > 255) br = 255;

  FastLED.setBrightness(br);
  FastLED.show();  
  return ret;
}

void OnCheckState(){}

void check_btn(struct s_state* state) {
  state->btn_state = digitalRead(PIN_BTN);
}

void report(struct s_state* state, int mode) {
  if(mode == 0 || ( mode == 1 && state->rgb.state != cur_state.rgb.state ) ){
    client.Publish("rgb_state", (state->rgb.state ? "1": "0"));
  }

  if(mode == 0 || ( mode == 1 && state->rgb.brightness != cur_state.rgb.brightness ) ){
    client.Publish("ini/rgb/brightness", String(state->rgb.brightness));
  }
  if(mode == 0 || ( mode == 1 && state->rgb.flag_change != cur_state.rgb.flag_change ) ){
    client.Publish("ini/rgb/flag_change", (state->rgb.flag_change ? "1" : "0"));
  }
  if(mode == 0 || ( mode == 1 && state->rgb.step != cur_state.rgb.step ) ){
    client.Publish("ini/rgb/step", String(state->rgb.step));
  }
  if(mode == 0 || ( mode == 1 && state->rgb.r != cur_state.rgb.r ) ){
    client.Publish("ini/rgb/r", String(state->rgb.r));
  }
  if(mode == 0 || ( mode == 1 && state->rgb.g != cur_state.rgb.g ) ){
    client.Publish("ini/rgb/g", String(state->rgb.g));
  }
  if(mode == 0 || ( mode == 1 && state->rgb.b != cur_state.rgb.b ) ){
    client.Publish("ini/rgb/b", String(state->rgb.b));
  }

  client.flag_start = false;
}

void check_state( ) {
  s_state state = cur_state;

  static uint64_t TimeSinceLastBtn;
  static boolean flagSinceLastBtn;

  check_btn(&state);
  
  if(state.btn_state == HIGH){
    if(!flagSinceLastBtn){
      flagSinceLastBtn = true;
      TimeSinceLastBtn = millis( );
    }
  } else {
    if(flagSinceLastBtn){
      if( millis( ) - TimeSinceLastBtn >= 300){
        client.Publish("btn_clicked", "1");
        state.rgb.state = !state.rgb.state;
      }
      TimeSinceLastBtn = 0;
    }
    flagSinceLastBtn = false;
  }  

  if (client.flag_start) { //первый запуск
    report(&state, 0); //отправляем все
  } else {
    report(&state, 1); //отправляем все
  }  

  cur_state = state;
}

void SetColor( uint8_t r, uint8_t g, uint8_t b){
  for (int i = 0 ; i < NUM_LEDS; i++ )
  {
    leds[i].setRGB( r, g, b);
  }
}

uint8_t hue; 
void colorsRoutine(){
  hue += 10;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
  FastLED.show();
}

void loop() {
  client.loop();

  check_state();

  switch(cur_state.alarm) {
    case 1:
      cur_state.alarmed = true;
      colorsRoutine();
    break;

    default:
      if(cur_state.alarmed) {
        cur_state.alarmed = false;
        FastLED.clear(true);
        SetColor(cur_state.rgb.r, cur_state.rgb.g, cur_state.rgb.b);
        FastLED.setBrightness(0); 
        FastLedShow();
      }
      rgbSetBrightness(FastLED.getBrightness(), (cur_state.rgb.state ? cur_state.rgb.brightness : 0), cur_state.rgb.step, cur_state.rgb.flag_change);
    break;
  }
}