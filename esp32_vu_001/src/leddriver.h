#pragma once

#include "Settings.h"

#define TOP            (kMatrixHeight - 0)         // Don't allow the bars to go offscreen

#define CHIPSET         WS2812B                     // LED strip type
#define COLOR_ORDER     GRB                         // If colours look wrong, play with this
#define LED_VOLTS       5                           // Usually 5 or 12
#define MAX_MILLIAMPS   10000                        // Careful with the amount of power here if running off USB port
const uint8_t kMatrixWidth = 32;                    // Matrix width --> number of columns in your led matrix
const uint8_t kMatrixHeight = 32;                   // Matrix height --> number of leds per column
#define PANE_WIDTH kMatrixWidth

#define NUM_LEDS   (kMatrixWidth * kMatrixHeight)   // Total number of LEDs
int BAR_WIDTH;
//#define BAR_WIDTH  (kMatrixWidth /(numBands ))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define TOP            (kMatrixHeight - 0)         // Don't allow the bars to go offscreen

CRGB leds[NUM_LEDS];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, kMatrixWidth, kMatrixHeight,
                                //NEO_MATRIX_TOP        + NEO_MATRIX_LEFT +
                                NEO_MATRIX_BOTTOM        + NEO_MATRIX_LEFT +
                                NEO_MATRIX_COLUMNS       + NEO_MATRIX_ZIGZAG +
                                //NEO_MATRIX_ROWS       + NEO_MATRIX_ZIGZAG +
                                //NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS
                                NEO_TILE_TOP + NEO_TILE_RIGHT + NEO_TILE_ROWS
                                );

void SetupLEDSTRIP(void){
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESSMARK);
  FastLED.clear();
}