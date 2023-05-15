#include <Arduino.h>
#include <FastLED_NeoMatrix.h>
#include <arduinoFFT.h>
#include <Wire.h>

// -------------------------------- НАСТРОЙКИ --------------------------------
// матрица
#define WIDTH 16          // ширина матрицы (число диодов)
#define HEIGHT 16         // высота матрицы (число диодов)
#define BRIGHTNESS 15     // яркость (0 - 255)

// цвета высоты полос спектра. Длины полос задаются примерно в строке 95
#define COLOR1 CRGB::Green
#define COLOR2 CRGB::Yellow
#define COLOR3 CRGB::Orange
#define COLOR4 CRGB::Red

// сигнал
#define INPUT_GAIN 1.5    // коэффициент усиления входного сигнала
#define LOW_PASS 100      // нижний порог чувствительности шумов (нет скачков при отсутствии звука)
#define MAX_COEF 1.1      // коэффициент, который делает "максимальные" пики чуть меньше максимума, для более приятного восприятия
#define NORMALIZE 0       // нормализовать пики (столбики низких и высоких частот будут одинаковой длины при одинаковой громкости) (1 вкл, 0 выкл)

// анимация
#define SMOOTH 0.3        // плавность движения столбиков (0 - 1)
#define DELAY 4           // задержка между обновлениями матрицы (периодичность основного цикла), миллиисекунды

// громкость
#define DEF_GAIN 50       // максимальный порог по умолчанию (при MANUAL_GAIN или AUTO_GAIN игнорируется)
#define MANUAL_GAIN 0     // ручная настройка потенциометром на громкость (1 вкл, 0 выкл)
#define AUTO_GAIN 1       // автонастройка по громкости (экспериментальная функция) (1 вкл, 0 выкл)

// точки максимума
#define MAX_DOTS 1        // включить/выключить отрисовку точек максимума (1 вкл, 0 выкл)
#define MAX_COLOR CRGB::Red // цвет точек максимума
#define FALL_DELAY 50     // скорость падения точек максимума (задержка, миллисекунды)
#define FALL_PAUSE 700    // пауза перед падением точек максимума, миллисекунды

// массив тонов, расположены примерно по параболе. От 80 Гц до 16 кГц
byte posOffset[17] = {2, 3, 4, 6, 8, 10, 12, 14, 16, 20, 25, 30, 35, 60, 80, 100, 120};
// -------------------------------- НАСТРОЙКИ --------------------------------

// ---------------------- ПИНЫ ----------------------
// для увеличения точности уменьшаем опорное напряжение,
// выставив EXTERNAL и подключив Aref к выходу 3.3V на плате через делитель
// GND ---[10-20 кОм] --- REF --- [10 кОм] --- 3V3

#define AUDIO_IN 35         // пин, куда подключен звук
#define DIN_PIN 4        // пин Din ленты (через резистор!)
// ---------------------- ПИНЫ ----------------------

// --------------- ДЛЯ РАЗРАБОТЧИКОВ ---------------
#define NUM_LEDS WIDTH * HEIGHT
#define FHT_N 256         // ширина спектра х2
#define LOG_OUT 1
#define COLOR_ORDER     GRB           // If colours look wrong, play with this
#define CHIPSET         WS2812B       // LED strip type
#define MAX_MILLIAMPS   2000          // Careful with the amount of power here if running off USB port
const int BRIGHTNESS_SETTINGS[3] = {5, 70, 200};  // 3 Integer array for 3 brightness settings (based on pressing+holding BTN_PIN)
#define LED_VOLTS       5             // Usually 5 or 12
#define SAMPLING_FREQ   40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
unsigned int sampling_period_us;

#define SAMPLES         256 //1024          // Must be a power of 2
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
CRGB leds[NUM_LEDS];
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);


int gain = DEF_GAIN;   // усиление по умолчанию
unsigned long gainTimer, fallTimer;
byte maxValue;
float k = 0.05, maxValue_f = 0.0;
int maxLevel[16];
byte posLevel_old[16];
unsigned long timeLevel[16], mainDelay;
boolean fallFlag;
// --------------- ДЛЯ РАЗРАБОТЧИКОВ ---------------

void analyzeAudio();

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, DIN_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS_SETTINGS[1]);
  FastLED.clear();

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
}

void loop() {
  
  if (millis() - mainDelay > DELAY) {     // итерация главного цикла
    mainDelay = millis();

    analyzeAudio( );

    for (int i = 0; i < (SAMPLES/2); i ++) {
      // вот здесь сразу фильтруем весь спектр по минимальному LOW_PASS
      if (vReal[i] < LOW_PASS) vReal[i] = 0;

      // усиляем сигнал
      vReal[i] = (float)vReal[i] * INPUT_GAIN;

      // уменьшаем громкость высоких частот (пропорционально частоте) если включено
      if (NORMALIZE) vReal[i] = (float)vReal[i] / ((float)1 + (float)i / 128);
    }

    maxValue = 0;
    FastLED.clear();  // очистить матрицу
    for (byte pos = 0; pos < WIDTH; pos++) {    // для кажого столбца матрицы
      int posLevel = vReal[posOffset[pos]];
      byte linesBetween;
      if (pos > 0 && pos < WIDTH) {
        linesBetween = posOffset[pos] - posOffset[pos - 1];
        for (byte i = 0; i < linesBetween; i++) {  // от предыдущей полосы до текущей
          posLevel += (float) ((float)i / linesBetween) * vReal[posOffset[pos] - linesBetween + i];
        }
        linesBetween = posOffset[pos + 1] - posOffset[pos];
        for (byte i = 0; i < linesBetween; i++) {  // от предыдущей полосы до текущей
          posLevel += (float) ((float)i / linesBetween) * vReal[posOffset[pos] + linesBetween - i];
        }
      }

      // найти максимум из пачки тонов
      if (posLevel > maxValue) maxValue = posLevel;

      // фильтрация длины столбиков, для их плавного движения
      posLevel = posLevel * SMOOTH + posLevel_old[pos] * (1 - SMOOTH);
      posLevel_old[pos] = posLevel;

      // преобразовать значение величины спектра в диапазон 0..HEIGHT с учётом настроек
      posLevel = map(posLevel, LOW_PASS, gain, 0, HEIGHT);
      posLevel = constrain(posLevel, 0, HEIGHT);

      if (posLevel > 0) {
        for (int j = 0; j < posLevel; j++) {                 // столбцы
          uint32_t color;
          if (j < 5) color = COLOR1;
          else if (j < 10) color = COLOR2;
          else if (j < 13) color = COLOR3;
          else if (j < 15) color = COLOR4;

          if (pos % 2 != 0)                                 // если чётная строка
            leds[pos * WIDTH + j] = color;                  // заливаем в прямом порядке
          else                                              // если нечётная
            leds[pos * WIDTH + WIDTH - j - 1] = color;      // заливаем в обратном порядке
        }
      }

      if (posLevel > 0 && posLevel > maxLevel[pos]) {    // если для этой полосы есть максимум, который больше предыдущего
        maxLevel[pos] = posLevel;                        // запомнить его
        timeLevel[pos] = millis();                       // запомнить время
      }

      // если точка максимума выше нуля (или равна ему) - включить пиксель
      if (maxLevel[pos] >= 0 && MAX_DOTS) {
        if (pos % 2 != 0)                                 // если чётная строка
          leds[pos * WIDTH + maxLevel[pos]] = MAX_COLOR;                  // заливаем в прямом порядке
        else                                              // если нечётная
          leds[pos * WIDTH + WIDTH - maxLevel[pos] - 1] = MAX_COLOR;      // заливаем в обратном порядке
      }

      if (fallFlag) {                                           // если падаем на шаг
        if ((long)millis() - timeLevel[pos] > FALL_PAUSE) {     // если максимум держался на своей высоте дольше FALL_PAUSE
          if (maxLevel[pos] >= 0) maxLevel[pos]--;              // уменьшить высоту точки на 1
          // внимание! Принимает минимальное значение -1 !
        }
      }
    }
    FastLED.show();  // отправить на матрицу

    fallFlag = 0;                                 // сбросить флаг падения
    if (millis() - fallTimer > FALL_DELAY) {      // если настало время следующего падения
      fallFlag = 1;                               // поднять флаг
      fallTimer = millis();
    }

    // если разрешена авто настройка уровня громкости
    if (AUTO_GAIN) {
      if (millis() - gainTimer > 10) {      // каждые 10 мс
        maxValue_f = maxValue * k + maxValue_f * (1 - k);
        // если максимальное значение больше порога, взять его как максимум для отображения
        if (maxValue_f > LOW_PASS) gain = (float) MAX_COEF * maxValue_f;
        // если нет, то взять порог побольше, чтобы шумы вообще не проходили
        else gain = 100;
        gainTimer = millis();
      }
    }
  }
}

void analyzeAudio() {
  // Sample the audio pin
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN); // A conversion takes about 9.7uS on an ESP32
    
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */ }
  }

  // Compute FFT
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();
}
