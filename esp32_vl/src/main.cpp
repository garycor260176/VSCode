#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_VL53L1X.h"

#include <VirtualButton.h>
VButton gest;

SFEVL53L1X distanceSensor;
static int time_budget_in_ms_long = 50;

// структура настроек
struct Data {
  bool state = 1;     // 0 выкл, 1 вкл
  byte mode = 0;      // 0 цвет, 1 теплота, 2 огонь
  byte bright[3] = {30, 30, 30};  // яркость
  byte value[3] = {0, 0, 0};      // параметр эффекта (цвет...)
};
Data data;

void setup() {
  Wire.begin();
  Wire.setClock(400000);

  Serial.begin(115200);                                         // Скорость обмена данными с компьютером

  if (distanceSensor.begin() != 0) {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while(1) ;
  }
  distanceSensor.setIntermeasurementPeriod(time_budget_in_ms_long);
  distanceSensor.setDistanceModeLong();  
  Serial.println("Sensor online!");
 }

int getDist() {
    distanceSensor.startRanging();
    int dist = distanceSensor.getDistance();
    distanceSensor.clearInterrupt();
    distanceSensor.stopRanging();
    return dist;
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

// экспоненциальный фильтр со сбросом снизу
#define ES_EXP 2L     // коэффициент плавности (больше - плавнее)
#define ES_MULT 16L   // мультипликатор повышения разрешения фильтра
int getFilterExp(int val) {
  static long filt;
  if (val) filt += (val * ES_MULT - filt) / ES_EXP;
  else filt = 0;  // если значение 0 - фильтр резко сбрасывается в 0
  // в нашем случае - чтобы применить заданную установку и не менять её вниз к нулю
  return filt / ES_MULT;
}

void pulse(){
  Serial.println("pulse");
}

void loop() {
  static uint32_t tmr;
  if (millis() - tmr >= 50) {
    tmr = millis();

    static uint32_t tout;   // таймаут настройки (удержание)
    static int offset_d;    // оффсеты для настроек
    static byte offset_v;

    int dist = getDist(); // получаем расстояние
    dist = getFilterMedian(dist);         // медиана
    dist = getFilterSkip(dist);           // пропускающий фильтр

    int dist_f = getFilterExp(dist);      // усреднение

    gest.poll((dist <= 100));                      // расстояние > 0 - это клик

// есть клики и прошло 2 секунды после настройки (удержание)
    if (gest.hasClicks() && millis() - tout > 2000) {
      switch (gest.clicks) {
        case 1:
          data.state = !data.state;  // вкл/выкл
          Serial.println("change state: " + String(data.state));
          break;
        case 2:
          if (data.state && ++data.mode >= 3) data.mode = 0;
          Serial.println("change mode: " + String(data.mode));
          break;
      }
    }

    if (gest.click()) {
      pulse( );
    }

// удержание (выполнится однократно)
    if (gest.held() && data.state) {
      pulse( );
      offset_d = dist_f;    // оффсет расстояния для дальнейшей настройки
      switch (gest.clicks) {
        case 0: offset_v = data.bright[data.mode]; 
          Serial.println("bright offset_v: " + String(offset_v));
          break;   // оффсет яркости
        case 1: offset_v = data.value[data.mode]; 
          Serial.println("value offset_v: " + String(offset_v));
        break;    // оффсет значения
      }
    }
    // удержание (выполнится пока удерживается)
    if (gest.hold() && data.state) {
      tout = millis();
      // смещение текущей настройки как оффсет + (текущее расстояние - расстояние начала)
      int shift = constrain(offset_v + (dist_f - offset_d), 0, 255);
      
      // применяем
      switch (gest.clicks) {
        case 0: data.bright[data.mode] = shift; 
          Serial.println("bright shift: " + String(shift));
        break;
        case 1: data.value[data.mode] = shift; 
          Serial.println("value shift: " + String(shift));
        break;
      }
    }
  }
}