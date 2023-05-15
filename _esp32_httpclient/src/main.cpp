#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <FastLED.h>

#define STASSID  "hacker"
#define STAPSK   "nthfgtdn260176"

const char* ssid = STASSID;
const char* pass = STAPSK;

#define PIN_LED               19
#define NUM_LEDS              4
CRGB leds[NUM_LEDS];
#define CURRENT_LIMIT 1000

WiFiClient wifi;

void flash_On( );
void flash_Off( );

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.println();  Serial.print("Connecting to ");  Serial.print(ssid);
  while ( WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("");
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");  Serial.println(ip);

  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  flash_Off( );
  FastLED.show();
}

void loop() {
  
  if ((WiFi.status() == WL_CONNECTED)) {
    flash_On();
    // создаем объект для работы с HTTP
    HTTPClient http;
    // подключаемся к веб-странице
    http.begin("http://192.168.2.1/create_photo");
    // делаем GET запрос
    int httpCode = http.GET();
    // проверяем успешность запроса
    if (httpCode > 0) {
      // выводим ответ сервера
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
    }
    else {
      Serial.println("Ошибка HTTP-запроса");
    }
    // освобождаем ресурсы микроконтроллера
    http.end();
    flash_Off();
  }
  delay(20000);
}

void flash_On( ){
  for (int i = 0 ; i < NUM_LEDS; i++ )
  {
    leds[i].setRGB( 255, 255, 255);
  }
  FastLED.setBrightness(255);
  FastLED.show();
}

void flash_Off( ){
  for (int i = 0 ; i < NUM_LEDS; i++ )
  {
    leds[i].setRGB( 0, 0, 0);
  }
  FastLED.setBrightness(0);
  FastLED.show();
}
