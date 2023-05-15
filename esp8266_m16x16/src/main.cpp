#define FASTLED_ESP8266_RAW_PIN_ORDER
#define CURRENT_LIMIT 1500    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
#include <FastLED.h>

#define PIN_LED               D5

#define WIDTH     16              // ширина матрицы
#define HEIGHT    16             // высота матрицы
#define NUM_LEDS  WIDTH * HEIGHT
CRGB leds[NUM_LEDS];

boolean started = false;
int cnt = 0;
boolean mode = false;

void setup() {
  Serial.begin(115200);                                         // Скорость обмена данными с компьютером
  Serial.println("");  Serial.println("Start!");

  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);  // GRB ordering is assumed
  FastLED.clearData();
  FastLED.clear();
  FastLED.setBrightness(50);
}

void loop() {
  if(!started) {
    FastLED.clear();
    FastLED.show();
    started = true;
    return;
  }

  if(mode) {
    leds[cnt].setRGB(0, 0, 0);
  } else {
    leds[cnt].setRGB(50, 0, 0);
  }
  FastLED.show();
  delay(100);

  cnt++; 
  if(cnt >= NUM_LEDS) {
    cnt = 0;
    mode = !mode;
  }
}
