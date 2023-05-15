#define FASTLED_INTERNAL
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Arduino_GFX_Library.h>
#include <arduinoFFT.h>
#include <Fastled.h>
#include <pixeltypes.h>

#define LED_PIN       19
#define INPUT_PIN     36

/*
#define COLOR_SPEED_PIN  39
#define BRIGHTNESS_PIN  34
#define PEAK_DECAY_PIN  35
#define COLOR_SCHEME_PIN  32
*/

#define BAND_COUNT    32
#define MATRIX_WIDTH  48
#define MATRIX_HEIGHT 16
#define GAIN_DAMPEN   2
#define MAX_COLOR_SPEED  64
#define SUPERSAMPLES  2
#define SAMPLE_BITS 12
#define MAX_ANALOG_PIN  ((1<<SAMPLE_BITS)*SUPERSAMPLES)
#define MAX_VU  12000
#define ONSCREEN_FPS  0
#define MS_PER_SECOND 1000
#define STACK_SIZE  4096

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

volatile float gScaler = 0.0f;
volatile size_t gFPS  = 0;
volatile size_t mFPS  = 0;
volatile float gLogScale  = 2.0f;
volatile float gBrightness  = 64;
volatile float gPeakDecay  = 0.0;
volatile float gColorSpeed  = 128.0f;
volatile float gVU  = 0;
volatile int giColorScheme  = 0;

volatile unsigned long g_cSamples  = 0;
volatile unsigned long g_cInterrupts  = 0;
volatile unsigned long g_cIRQMisses  = 0;

#include "Utilities.h"
#include "LEDMatrixGFX.h"
#include "Palettes.h"
#include "SpectrumDisplay.h"
#include "SoundAnalyzer.h"

//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R2, 15, 4, 16);
LEDMatrixGFX gMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, 255);
SpectrumDisplay gDisplay(&gMatrix, BAND_COUNT);
SoundAnalyzer gAnalyzer(INPUT_PIN);

void SampleLoop(void *);
void TFTUpdateLoop(void* );
void MatrixLoop(void *);

void setup() {
  Serial.begin(115200);
/*
  Serial.println("Initializing TFT display...");
  u8g2.begin();
  u8g2.clear();
*/

  Serial.println("Configure input pins...");
/*
  pinMode(BRIGHTNESS_PIN, INPUT);
  pinMode(COLOR_SPEED_PIN, INPUT);
  pinMode(PEAK_DECAY_PIN, INPUT);
*/

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11); //GPIO36
  /*
  adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_11); //GPIO39
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11); //GPIO34
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11); //GPIO34
  adc1_config_channel_atten(ADC1_CHANNEL_4,ADC_ATTEN_DB_11); //GPIO32
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11); //GPIO35
  */

/*
  analogReadResolution(12);
  analogSetWidth(12);
  //analogSetCycles(32);
  //analogSetSamples(SUPERSAMPLES);
  analogSetClockDiv(1);
  analogSetAttenuation(ADC_11db);
  analogSetPinAttenuation(INPUT_PIN, ADC_2_5db);
*/
  Serial.println("Scheduling CPU Cores...");
  TaskHandle_t samplerTask;
  TaskHandle_t matrixTask;
  xTaskCreatePinnedToCore(SampleLoop, "Sampler_Loop", STACK_SIZE, nullptr, 1, &samplerTask, 0);
  xTaskCreatePinnedToCore(MatrixLoop, "Matrix_Loop", STACK_SIZE, nullptr, 1, &matrixTask, 1);

  Serial.println("Audio Sampler Launching...");
  Serial.printf("   FFT Size: %d bytes\n", MAX_SAMPLES);
  g_SoundAnalyzer.StartInterrupts( );
  Serial.println("Sampler Started! System is OPERATIONAL");
}

void loop() {
  delay(portMAX_DELAY);
}

void SampleLoop(void *){
  unsigned long lastFrame = 0;
  for(;;){
    gFPS = FPS(lastFrame, millis());
    lastFrame = millis();

    PeakData peaks = gAnalyzer.RunSamplerPass(BAND_COUNT);
    gDisplay.SetPeaks(BAND_COUNT, peaks);

    delay(5);
  }
}

void MatrixLoop(void *){
  static float colorShift = 0;
  unsigned long lastTime = 0;
  for(;;){
    mFPS = FPS(lastTime, millis());
    float secondElapsed = (millis() - lastTime) / (float)MS_PER_SECOND;
    lastTime = millis();

    gMatrix.fillScreen(BLACK);

    if(gColorSpeed < 2) {
      gDisplay.Draw(0);
    } else {
      colorShift += gColorSpeed * secondElapsed;
      while(colorShift >= 256) {
        colorShift -= 256;
      }

      gDisplay.Draw((byte)colorShift);
    }

    #if ONSCREEN_FPS
      gMatrix.setTextColor(RED16);
      gMatrix.setCursor(20, 0);
      gMatrix.print(gFPS);

      gMatrix.setTextColor(BLUE16);
      gMatrix.setCursor(0, 0);
      gMatrix.print(mFPS);
    #endif

    gMatrix.setBrightness(gBrightness);
    gMatrix.ShowMatrix();
    
    yield();
  }
}