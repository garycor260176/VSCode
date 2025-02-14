#include <mqtt_ini.h>
//#include <FastLED.h>

#define DATA_PIN 23
#define NUM_LEDS 1

//CRGB leds[NUM_LEDS];

#define def_path "TEST"

mqtt_ini client( 
  "ESP32_TEST",     // Client name that uniquely identify your device
   def_path);

void setup() {
  Serial.begin(115200);                                         
  Serial.println("");  Serial.println("Start!");

  client.begin();

  //FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
}

void onMsgCommand( const String &message ){}
void OnLoad(){}
void OnCheckState(){}
void onConnection(){
}

void report(int mode ){
  client.Publish("states/test", String(millis()));
  client.flag_start = false;
}

void loop() {
  client.loop();

  report(0); //отправляем все

  delay(1000);

/*/
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);

  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
*/
}