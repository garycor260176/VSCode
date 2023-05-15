#include <Arduino.h>
#include <FastLED_NeoMatrix.h>
#include <arduinoFFT.h>
#include <Wire.h>
#include <driver/adc.h>

#define LED_PIN       19
#define INPUT_PIN     35

#define STACK_SIZE  4096
#define MAX_VU  12000
#define GAIN_DAMPEN   2
#define BAND_COUNT    32
#define MAX_COLOR_SPEED  64
#define MATRIX_WIDTH  32
#define MATRIX_HEIGHT 16
#define MS_PER_SECOND 1000

volatile float gLogScale  = 2.0f;
volatile unsigned long g_cSamples  = 0;
volatile unsigned long g_cInterrupts  = 0;
volatile unsigned long g_cIRQMisses  = 0;
volatile float gScaler = 0.0f;
volatile int giColorScheme  = 0;
volatile float gPeakDecay  = 0.0;
volatile float gColorSpeed  = 128.0f;
volatile float gBrightness  = 64;
volatile float gVU  = 0;

#include "Utilities.h"
#include "LEDMatrixGFX.h"
#include "SpectrumDisplay.h"
#include "SoundAnalyzer.h"

LEDMatrixGFX gMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, 255);
SpectrumDisplay gDisplay(&gMatrix, BAND_COUNT);
SoundAnalyzer gAnalyzer(INPUT_PIN);

void SampleLoop(void *);
void MatrixLoop(void *);

void setup() {
  Serial.begin(115200);

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11); //GPIO35

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
  for(;;){
    PeakData peaks = gAnalyzer.RunSamplerPass(BAND_COUNT);
    gDisplay.SetPeaks(BAND_COUNT, peaks);

    delay(5);
  }
}

void MatrixLoop(void *){

}
