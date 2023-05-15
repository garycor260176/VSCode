#pragma once

#include <Arduino.h>

#define ADC_INPUT       ADC1_CHANNEL_0
#define PIN_MODE        18
#define PIN_BAND        23
#define LED_PIN         19                          // LED strip data

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
int16_t samples[SAMPLEBLOCK];
double vReal[SAMPLEBLOCK];
double vImag[SAMPLEBLOCK];
uint16_t offset = (int)ADC_INPUT * 0x1000 + 0xFFF;
float FreqBins[65] = {0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int oldBarHeights[65] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};      // so they are set to 65
byte peak[65] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
int NoiseTresshold =        500;                   // this will effect the upper bands most.
volatile float         gVU       = 0;              // Instantaneous read of VU value
volatile float         oldVU     = 0;              // Previous read of VU value
#define DemoTreshold        500                     // this defines the treshold that will get the unit out of demo mode
#define GAIN_DAMPEN         2                       // Higher values cause auto gain to react more slowly
char PeakFlag[32];                           // the top peak delay needs to have a flag because it has different timing while floating compared to falling to the stack
int PeakTimer[32];                           // counter how many loops to stay floating before falling to stack
#define MAX_PEAKDELAY       100
int Peakdelay =             10;                     // Delay before peak falls down to stack. Overruled by PEAKDEALY Potmeter
#define BottomRowAlwaysOn   1                       // if set to 1, bottom row is always on. Setting only applies to LEDstrip not HUB75
int buttonPushCounter =     2;                      // This number defines what pattern to start after boot (0 to 12)
#define Fallingspeed        5                       // Falling down factor that effects the speed of falling tiles
bool autoChangePatterns =   false;                  // After boot, the pattern will not change automatically. 
#define SecToChangePattern  10                      // number of seconds that pattern changes when auto change mode is enabled
uint8_t colorTimer = 0;
#define MAX_VU              5000                    // How high our VU could max out at.  Arbitarily tuned.
#define ChangingBar_Color   y * (255 / kMatrixHeight) + colorTimer, 255, 255
#define BRIGHTNESSMAX       255                     // Max brightness of the leds...carefull...to bright might draw to much amps!
int BRIGHTNESSMARK= 50;                            // Default brightnetss, however, overruled by the Brightness potmeter
#define BRIGHTNESSMIN       5                              // Min brightness

#define TriBar_Color_Top      0 , 255, 255    // Red CHSV
#define TriBar_Color_Bottom   95 , 255, 255   // Green CHSV
#define TriBar_Color_Middle   45, 255, 255    // Yellow CHSV
#define TriBar_Color_Top_Peak      0 , 255, 255    // Red CHSV
#define TriBar_Color_Bottom_Peak   95 , 255, 255   // Green CHSV
#define TriBar_Color_Middle_Peak   45, 255, 255    // Yellow CHSV
#define PeakColor1  0, 0, 255       // white CHSV
#define RainbowBar_Color  (x / BAR_WIDTH) * (255 / numBands), 255, 255
#define PeakColor2  0, 0, 255       // white CHSV
DEFINE_GRADIENT_PALETTE( purple_gp ) {
  0,   0, 212, 255,   //blue
255, 179,   0, 255 }; //purple
CRGBPalette16 purplePal = purple_gp;
#define SameBar_Color1      0 , 255, 255      //red  CHSV
#define PeakColor3  160, 255, 255   // blue CHSV
#define SameBar_Color2      160 , 255, 255    //blue  CHSV
#define PeakColor4  0, 255, 255   // red CHSV
DEFINE_GRADIENT_PALETTE( redyellow_gp ) {  
  0,   200, 200,  200,   //white
 64,   255, 218,    0,   //yellow
128,   231,   0,    0,   //red
192,   255, 218,    0,   //yellow
255,   200, 200,  200 }; //white
CRGBPalette16 heatPal = redyellow_gp;
DEFINE_GRADIENT_PALETTE( outrun_gp ) {
  0, 141,   0, 100,   //purple
127, 255, 192,   0,   //yellow
255,   0,   5, 255 };  //blue
CRGBPalette16 outrunPal = outrun_gp;
DEFINE_GRADIENT_PALETTE( mark_gp2 ) {
  0,   255,   218,    0,   //Yellow
 64,   200, 200,    200,   //white
128,   141,   0, 100,   //pur
192,   200, 200,    200,   //white
255,   255,   218,    0,};   //Yellow
CRGBPalette16 markPal2 = mark_gp2;
DEFINE_GRADIENT_PALETTE( mark_gp ) {
  0,   231,   0,    0,   //red
 64,   200, 200,    200,   //white
128,   200, 200,    200,   //white
192,   200, 200,    200,   //white
255,   231, 0,  0,};   //red
CRGBPalette16 markPal = mark_gp;
#define PeakColor5  160, 255, 255   // blue CHSV
#define TriBar_Color_Top_Peak2      0 , 255, 255    // Red CHSV
#define TriBar_Color_Bottom_Peak2   95 , 255, 255   // Green CHSV
#define TriBar_Color_Middle_Peak2   45, 255, 255    // Yellow CHSV

#define FPS 25              /* Refresh rate 15 looks good*/
const uint32_t colors[] = {
  0x000000,
  0x100000,
  0x300000,
  0x600000,
  0x800000,
  0xA00000,
  0xC02000,
  0xC04000,
  0xC06000,
  0xC08000,
  0x807080
};
const uint8_t maxflare = 50;//4;     /* max number of simultaneous flares */
const uint8_t flaredecay = 14;  /* decay rate of flare radiation; 14 is good */
const uint8_t flarechance = 50; /* 50chance (%) of a new flare (if there's room) */
const uint8_t flarerows = 8;  //8  /* number of rows (from bottom) allowed to flare */
