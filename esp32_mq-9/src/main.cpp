#include <Arduino.h>
#include <driver/adc.h>
#include <Preferences.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid     = "MQ-9";
const char* password = "MQ9sensor";

const char* http_username = "admin";
const char* http_password = "admin";

AsyncWebServer server(80);

#define PIN_LED   18
#define PIN_BEEP  19

int cnt = 0;

Preferences preferences; //хранение текущего состояния
LiquidCrystal_I2C lcd(0x27,16,2);

boolean delay_started = false;
uint64_t delay_start = 0;

struct s_settings{
  int start_alarm_value = 1000;
};

s_settings cur_settings;

s_settings read_eeprom(){
  s_settings ret;

  ret.start_alarm_value = preferences.getInt("sav", 1000);
  if(ret.start_alarm_value <= 0) ret.start_alarm_value = 1000;
  Serial.println("read start_alarm_value = " + String(ret.start_alarm_value));

  return ret;
}

void write_eeprom(s_settings set){
  if(set.start_alarm_value != cur_settings.start_alarm_value)    {
    preferences.putInt("sav", set.start_alarm_value);
    Serial.println("save start_alarm_value = " + String(set.start_alarm_value));
  }

  cur_settings = set;
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32/MQ-9</title>
</head>
<body>
  <form action="/get">
    alarm value: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200); 

  lcd.init();   
  lcd.backlight(); 
  lcd.setCursor(0,0); 
  lcd.print("init...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  preferences.begin("settings", false);
  cur_settings = read_eeprom();

  pinMode(PIN_LED, OUTPUT); digitalWrite(PIN_LED, LOW);
  pinMode(PIN_BEEP, OUTPUT); digitalWrite(PIN_BEEP, LOW);

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); //GPIO32

// Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam("input1")) {
      inputMessage = request->getParam("input1")->value();
      s_settings set = cur_settings;
      set.start_alarm_value = inputMessage.toInt( );
      write_eeprom( set );
      inputMessage = "Saved!";
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });

  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("init - ok!");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(IP);

  server.begin();

  delay_started = false;
}

void work(){
  if(delay_started  && millis() - delay_start > 1000) {
    delay_started = false;
  }

  if(!delay_started) {
    delay_started = true;

    float sensorValue = adc1_get_raw(ADC1_CHANNEL_4); 
    Serial.println("adc1_4 = " + String(sensorValue));
    lcd.setCursor(0, 1);    lcd.print("                ");
    lcd.setCursor(0, 1);    lcd.print("V = " + String(sensorValue));

    if (sensorValue > cur_settings.start_alarm_value) { digitalWrite(PIN_LED, HIGH); digitalWrite(PIN_BEEP, HIGH); }
    else                                              { digitalWrite(PIN_LED, LOW); digitalWrite(PIN_BEEP, LOW); }

    delay_start = millis( );
  }
}

void loop() {
  work( );
}