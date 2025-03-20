#include <mqtt_ini.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <Preferences.h>

#define def_path "switch/s_01"

#define LED_PIN        16
#define MAXNUMBUTTONS  4

#define NUMLEDS        4
CRGB leds[NUMLEDS];
#define CURRENT_LIMIT 1000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

Preferences preferences;

mqtt_ini *client;

enum MODES{
  AUTO = 0,
  ON = 1,
  OFF = 2,
  AUTO_ON = 3,
  AUTO_OFF = 4,
  UNKNOWN = 99,
};

int pins[MAXNUMBUTTONS] = {
5,
23,
19,
18,
};
 
struct stru_state{
  int brightness;
  int AutoSeconds;
};

stru_state curState;

void report(stru_state, int);
void onMsgCommand(const String &message);
void OnCheckState();
void onConnection();
void OnLoad();
stru_state read_eeprom();
stru_state write_eeprom(stru_state newState);
void createButtons();
void setBrightness(int brgth);

//***
class Button {
  public:
    Button(int _pin, int _num);
    String getName();
    void setMode(int _mode);
    void loop();

  private:
    int pin;
    int mode = UNKNOWN;
    int num;
    String name;

    uint32_t TimeSinceLastBtn = 0;
    boolean flagSinceLastBtn = false;
    
    int read_mode();
    int write_mode(int);
    void setColor(uint8_t g, uint8_t r, uint8_t b);
    void blinkColor();

    uint16_t delaybri = 100;
    uint32_t TimeDelaybri = 0;
    int step = 10;
};

void Button::loop(){
  String _name = name + "/pressed";

  int state = digitalRead(pin);
  if(state == HIGH){
    if(!flagSinceLastBtn){
      flagSinceLastBtn = true;
      TimeSinceLastBtn = millis( );
    }
  } else {
    if(flagSinceLastBtn) {
      uint32_t secs = curState.AutoSeconds * 1000;

      if(millis( ) - TimeSinceLastBtn >= secs) {
        client->Publish(_name.c_str(), String(2));
      } else {
        client->Publish(_name.c_str(), String(1));
      } 
      TimeSinceLastBtn = 0;
    }
    flagSinceLastBtn = false;
  }

  switch(mode){
    case AUTO: 
    case AUTO_ON: 
    case AUTO_OFF: blinkColor();           break;
    case ON:       setColor(255, 0, 0);    break;
    case OFF:      setColor(0, 255, 0);    break;
    case UNKNOWN:  setColor(255, 255, 0);    break;
    break;
  }
}

void Button::blinkColor(){
  if(millis() - TimeDelaybri > delaybri) {
    TimeDelaybri = millis();

    int color;

    switch(mode){
      case AUTO_ON:  color = leds[num].g;      break;
      case AUTO_OFF: color = leds[num].r;      break;
      default:       color = leds[num].b;      break;
    }
    color += step;
    if(color > 255 || color < 0) { 
      step = - step;
    }
    color = color + step;
    switch(mode){
      case AUTO_ON:  setColor(color, 0, 0);    break;
      case AUTO_OFF: setColor(0, color, 0);    break;
      default:       setColor(0, 0, color);    break;
    }
  }
}

void Button::setColor(uint8_t g, uint8_t r, uint8_t b){
  leds[num].setRGB( r, g, b);
  FastLED.show();
}

String Button::getName(){
  return name;
}

void Button::setMode(int _mode){
  int newMode = write_mode(_mode);
  if(newMode != mode) {
    mode = newMode;
    if(client != NULL && client->client->isConnected()){
      String _name = name + "/mode";
      client->Publish(_name.c_str(), String(mode));
    }
  }
  switch(mode){
    case 1:  setColor(255, 0, 0); break;
    case 2:  setColor(0, 255, 0); break;
    default: setColor(0, 0, 255); break;
  }
}

int Button::read_mode(){
/*  int ret = preferences.getInt(name.c_str(), 0);
  if(ret < 0) ret = 0;
  if(ret > 2) ret = 2;
  Serial.println("eeprom readed: " + name + " = " + String(ret));
  return ret;*/
  return mode;
}

int Button::write_mode(int _mode){
  int ret = _mode;
/*  if(ret < 0) ret = 0;
  if(ret > 2) ret = 2;
  if(ret != mode) {
    preferences.putInt(name.c_str(), ret);
    Serial.println("eeprom writed: " + name + " = " + String(ret));
  }*/
  return ret;
}

Button::Button(int _pin, int _num){
  pin = _pin;
  num = _num;
  name = "B" + String(num);
  pinMode(pin, INPUT);
  mode = UNKNOWN; // read_mode();
  setMode(mode);
}

Button* btns[MAXNUMBUTTONS];

stru_state read_eeprom(){
  stru_state ret = curState;

  ret.brightness = preferences.getInt("brgth", 127);
  if(ret.brightness < 0) ret.brightness = 0;
  if(ret.brightness > 255) ret.brightness = 255;
  Serial.println("eeprom readed: brgth = " + String(ret.brightness));

  ret.AutoSeconds = preferences.getInt("autosec", 2);
  if(ret.AutoSeconds < 2) ret.AutoSeconds = 2;
  if(ret.AutoSeconds > 10) ret.AutoSeconds = 10;
  Serial.println("eeprom readed: autosec = " + String(ret.AutoSeconds));

  return ret;
}

stru_state write_eeprom(stru_state newState){
  stru_state ret = newState;

  if(ret.brightness < 0) ret.brightness = 0;
  if(ret.brightness > 255) ret.brightness = 255;
  if(ret.brightness != curState.brightness) {
    preferences.putInt("brgth", ret.brightness);
    Serial.println("eeprom writed: brgth = " + String(ret.brightness));
  }

  if(ret.AutoSeconds < 2) ret.AutoSeconds = 2;
  if(ret.AutoSeconds > 255) ret.AutoSeconds = 255;
  if(ret.AutoSeconds != curState.AutoSeconds) {
    preferences.putInt("autosec", ret.AutoSeconds);
    Serial.println("eeprom writed: autosec = " + String(ret.AutoSeconds));
  }

  return ret;
}

//***
void onMsgCommand( const String &message ){
}

void report(stru_state newState, int mode){
  if (mode == 0 || ( mode == 1 && newState.brightness != curState.brightness ) ) {
    client->Publish("settings/brightness", String(newState.brightness));
  }

  if (mode == 0 || ( mode == 1 && newState.AutoSeconds != curState.AutoSeconds ) ) {
    client->Publish("settings/AutoSeconds", String(newState.AutoSeconds));
  }

  curState = newState;
  client->flag_start = false;
}

void OnCheckState(){
  stru_state newState = curState;

  if(client->flag_start){ //первый запуск
    report(newState, 0); //отправляем все
  } else {
    report(newState, 1); //отправляем только то, что изменилось
  }
}

void Msg_brightness(const String &message){
  stru_state newState = curState;
  newState.brightness = message.toInt();
  newState = write_eeprom(newState);
  setBrightness(newState.brightness);
  report(newState, 1);
}

void Msg_AutoSeconds(const String &message){
  stru_state newState = curState;
  newState.AutoSeconds = message.toInt();
  newState = write_eeprom(newState);
  report(newState, 1);
}

void Msg_btn0Mode(const String &message){
  btns[0]->setMode(message.toInt());
}
void Msg_btn1Mode(const String &message){
  btns[1]->setMode(message.toInt());
}
void Msg_btn2Mode(const String &message){
  btns[2]->setMode(message.toInt());
}
void Msg_btn3Mode(const String &message){
  btns[3]->setMode(message.toInt());
}

void onConnection()
{
  client->Subscribe("settings/brightness", Msg_brightness); 
  client->Subscribe("settings/AutoSeconds", Msg_AutoSeconds); 
  client->Subscribe(btns[0]->getName( ) + "/mode", Msg_btn0Mode); 
  client->Subscribe(btns[1]->getName( ) + "/mode", Msg_btn1Mode); 
  client->Subscribe(btns[2]->getName( ) + "/mode", Msg_btn2Mode); 
  client->Subscribe(btns[3]->getName( ) + "/mode", Msg_btn3Mode); 
}

void OnLoad(){
  client->flag_start = true;
}

void createButtons(){
  for(int i=0; i<MAXNUMBUTTONS;i++){
    btns[i] = new Button(pins[i], i);
  }
}

void setBrightness(int brgth){
  FastLED.setBrightness(brgth);
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start...");

  curState = read_eeprom();

  createButtons();

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUMLEDS);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  setBrightness(curState.brightness);

  client = new mqtt_ini( 
    "SWITCH01",
     def_path);

  client->begin(false);
}

void loop() {
  client->loop();

  for(int i=0; i<MAXNUMBUTTONS; i++){
    btns[i]->loop();
  }
}
