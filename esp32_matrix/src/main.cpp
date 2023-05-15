// (Heavily) adapted from https://github.com/G6EJD/ESP32-8266-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_02.ino
// Adjusted to allow brightness changes on press+hold, Auto-cycle for 3 button presses within 2 seconds
// Edited to add Neomatrix support for easier compatibility with different layouts.

#include <Arduino.h>
#include <FastLED_NeoMatrix.h>
#include <arduinoFFT.h>
#include <Wire.h>
#include <driver/adc.h>

#define NOISE           1000           // Used as a crude noise filter, values below this are ignored

#define STACK_SIZE  4096

//#define PIN_PEAK_DELAY  34
#define MAX_PEAK_DELAY  5000
uint64_t PEAK_DELAY = 300;

int cBAND = 0;
int aBANDS[] = {32, 16, 8};
int NUM_BANDS = aBANDS[cBAND];

#define INPUT_GAIN      1.5    // коэффициент усиления входного сигнала
#define SAMPLES         1024          // Must be a power of 2
#define SAMPLING_FREQ   40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE       1000          // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define AUDIO_IN_PIN    35            // Signal in on this pin
#define LED_PIN         19             // LED strip data
#define COLOR_ORDER     GRB           // If colours look wrong, play with this
#define CHIPSET         WS2812B       // LED strip type
#define MAX_MILLIAMPS   2000          // Careful with the amount of power here if running off USB port
const int BRIGHTNESS_SETTINGS[3] = {5, 70, 200};  // 3 Integer array for 3 brightness settings (based on pressing+holding BTN_PIN)
#define LED_VOLTS       5             // Usually 5 or 12

#define PIN_NUM_BANDS   4
boolean flagSinceLastBtn = false;
uint32_t TimeSinceLastBtn;

#define NOISE           200           // Used as a crude noise filter, values below this are ignored
const uint8_t kMatrixWidth = 32;                           // Matrix width
const uint8_t kMatrixHeight = 16;                         // Matrix height
#define NUM_LEDS       (kMatrixWidth * kMatrixHeight)     // Total number of LEDs
#define BAR_WIDTH      (kMatrixWidth  / (NUM_BANDS - 1))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define TOP            (kMatrixHeight - 0)                // Don't allow the bars to go offscreen
#define SERPENTINE     true                               // Set to false if you're LEDS are connected end to end, true if serpentine

// Sampling and FFT stuff
unsigned int sampling_period_us;
byte peak[] =           {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
uint32_t peak_delay[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int oldBarHeights[] =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bandValues[] =      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

// FastLED stuff
CRGB leds[NUM_LEDS];
DEFINE_GRADIENT_PALETTE( purple_gp ) {
  0,   0, 212, 255,   //blue
255, 179,   0, 255 }; //purple
DEFINE_GRADIENT_PALETTE( outrun_gp ) {
  0, 141,   0, 100,   //purple
127, 255, 192,   0,   //yellow
255,   0,   5, 255 };  //blue
DEFINE_GRADIENT_PALETTE( greenblue_gp ) {
  0,   0, 255,  60,   //green
 64,   0, 236, 255,   //cyan
128,   0,   5, 255,   //blue
192,   0, 236, 255,   //cyan
255,   0, 255,  60 }; //green
DEFINE_GRADIENT_PALETTE( redyellow_gp ) {
  0,   200, 200,  200,   //white
 64,   255, 218,    0,   //yellow
128,   231,   0,    0,   //red
192,   255, 218,    0,   //yellow
255,   200, 200,  200 }; //white
CRGBPalette16 purplePal = purple_gp;
CRGBPalette16 outrunPal = outrun_gp;
CRGBPalette16 greenbluePal = greenblue_gp;
CRGBPalette16 heatPal = redyellow_gp;
uint8_t colorTimer = 0;

// FastLED_NeoMaxtrix - see https://github.com/marcmerlin/FastLED_NeoMatrix for Tiled Matrixes, Zig-Zag and so forth
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, kMatrixWidth, kMatrixHeight,
  NEO_MATRIX_BOTTOM        + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS       + NEO_MATRIX_ZIGZAG +
  NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS);

void rainbowBars(int band, int barHeight);
void whitePeak(int band);

static int cutOffs32Band[32] = 
{
    10, 20, 25, 31, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000,
    1250, 1600, 2000, 2500, 3150, 4000, 5000, 6400, 8000, 10000, 12500, 16500, 20000
};

static int cutOffs24Band[24] = 
{
    40, 80, 150, 220, 270, 320, 380, 440, 540, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150,
    3800, 4200, 4800, 5400, 6200, 7400, 12500
};

static int cutOffs16Band[16] = 
{
    100, 250, 450, 565, 715, 900, 1125, 1400, 1750, 2250, 2800, 3150, 4000, 5000, 6400, 12500
};

static int cutOffs8Band[8] = 
{
    20, 150, 400, 750, 751, 752, 800, 1200
};

static int * BandCutoffTable(int bandCount){
    if(bandCount == 8) return cutOffs8Band;
    if(bandCount == 16) return cutOffs16Band;
    if(bandCount == 32) return cutOffs32Band;

    return cutOffs32Band;
}

int BucketFrequency(int iBucket) {
    if(iBucket <= 1) {
        return 0;
    }

    int iOffset = iBucket - 2;
    return iOffset * (SAMPLING_FREQ / 2) / (SAMPLES / 2);
}

void ScanInputs() {
    int btn_state = digitalRead(PIN_NUM_BANDS);
    if(btn_state == HIGH) {
      if(!flagSinceLastBtn){
        flagSinceLastBtn = true;
        TimeSinceLastBtn = millis( );
      }
    } else {
      if(flagSinceLastBtn){
        cBAND++;
        if(cBAND > 2) cBAND = 0;
        NUM_BANDS = aBANDS[cBAND];

        TimeSinceLastBtn = 0;
      }
      flagSinceLastBtn = false;
    }
//  peak delay
    int raw = adc1_get_raw(ADC1_CHANNEL_6);
    PEAK_DELAY = map(raw, 0, 4096, 0, MAX_PEAK_DELAY);
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS_SETTINGS[1]);
  FastLED.clear();

  pinMode(PIN_NUM_BANDS, INPUT);

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11); //GPIO35
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11); //GPIO34
  
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));
}

void AcquireSample() {
    // Sample the audio pin
    for (int i = 0; i < SAMPLES; i++) {
      newTime = micros();
      vReal[i] = adc1_get_raw(ADC1_CHANNEL_7);
      vImag[i] = 0;
      while ((micros() - newTime) < sampling_period_us) {  }
    }
}

void _FFT(){
    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude();  
    double peak = FFT.MajorPeak();
    //Serial.println(String(peak));
    //Serial.println("");
}

void ProcessPeaks(){
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }

/*
  for(int i=2; i<SAMPLES/2; i++){
      if(vReal[i] > NOISE) {
        int freq = BucketFrequency(i);
        //Serial.print(String(freq) + " - ");
        

        // усиляем сигнал
        //vReal[i] = vReal[i] * INPUT_GAIN;

        int iBand = 0;
        while(iBand < NUM_BANDS){
            if(freq < BandCutoffTable(NUM_BANDS)[iBand]) break;
            iBand++;
        }
        if(iBand > NUM_BANDS) iBand = NUM_BANDS;
        
        bandValues[iBand]  += (int)vReal[i];
      }
  }
*/  
  
  for (int i = 2; i < (SAMPLES/2); i++){       // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
      if (vReal[i] >= NOISE) {                    // Add a crude noise filter
        switch(NUM_BANDS){
          case 8:
            if (i<=3 )           bandValues[0]  += (int)vReal[i];
            if (i>3   && i<=6  ) bandValues[1]  += (int)vReal[i];
            if (i>6   && i<=13 ) bandValues[2]  += (int)vReal[i];
            if (i>13  && i<=27 ) bandValues[3]  += (int)vReal[i];
            if (i>27  && i<=55 ) bandValues[4]  += (int)vReal[i];
            if (i>55  && i<=112) bandValues[5]  += (int)vReal[i];
            if (i>112 && i<=229) bandValues[6]  += (int)vReal[i];
            if (i>229          ) bandValues[7]  += (int)vReal[i];
            break;

          case 16:          
            if (i<=2 )           bandValues[0]  += (int)vReal[i];
            if (i>2   && i<=3  ) bandValues[1]  += (int)vReal[i];
            if (i>3   && i<=5  ) bandValues[2]  += (int)vReal[i];
            if (i>5   && i<=7  ) bandValues[3]  += (int)vReal[i];
            if (i>7   && i<=9  ) bandValues[4]  += (int)vReal[i];
            if (i>9   && i<=13 ) bandValues[5]  += (int)vReal[i];
            if (i>13  && i<=18 ) bandValues[6]  += (int)vReal[i];
            if (i>18  && i<=25 ) bandValues[7]  += (int)vReal[i];
            if (i>25  && i<=36 ) bandValues[8]  += (int)vReal[i];
            if (i>36  && i<=50 ) bandValues[9]  += (int)vReal[i];
            if (i>50  && i<=69 ) bandValues[10] += (int)vReal[i];
            if (i>69  && i<=97 ) bandValues[11] += (int)vReal[i];
            if (i>97  && i<=135) bandValues[12] += (int)vReal[i];
            if (i>135 && i<=189) bandValues[13] += (int)vReal[i];
            if (i>189 && i<=264) bandValues[14] += (int)vReal[i];
            if (i>264          ) bandValues[15] += (int)vReal[i];
            break;

          default:
//32 bands, 280hz to 12kHz top band
            if (i <= 2 )           bandValues[0]  += (int)vReal[i];
            if (i > 2   && i <= 3  ) bandValues[1]  += (int)vReal[i];
            if (i > 3   && i <= 4  ) bandValues[2]  += (int)vReal[i];
            if (i > 4   && i <= 5 ) bandValues[3]  += (int)vReal[i];
            if (i > 5  && i <= 6 ) bandValues[4]  += (int)vReal[i];
            if (i > 6 && i <= 7 ) bandValues[5]  += (int)vReal[i];
            if (i > 7 && i <= 8) bandValues[6]  += (int)vReal[i];
            if (i > 8 && i <= 9) bandValues[7]  += (int)vReal[i];
            if (i > 9 && i <= 11) bandValues[8]  += (int)vReal[i];
            if (i > 11 && i <= 13 ) bandValues[9]  += (int)vReal[i];
            if (i > 13  && i <= 16 ) bandValues[10] += (int)vReal[i];
            if (i > 16  && i <= 18 ) bandValues[11] += (int)vReal[i];
            if (i > 18  && i <= 22) bandValues[12] += (int)vReal[i];
            if (i > 22 && i <= 25) bandValues[13] += (int)vReal[i];
            if (i > 25 && i <= 30) bandValues[14] += (int)vReal[i];
            if (i > 30   && i <= 36  ) bandValues[15]  += (int)vReal[i];
            if (i > 36   && i <= 43  ) bandValues[16]  += (int)vReal[i];
            if (i > 43   && i <= 50  ) bandValues[17]  += (int)vReal[i];
            if (i > 50   && i <= 59  ) bandValues[18]  += (int)vReal[i];
            if (i > 59   && i <= 69 ) bandValues[19]  += (int)vReal[i];
            if (i > 69  && i <= 80) bandValues[20]  += (int)vReal[i];
            if (i > 80  && i <= 97 ) bandValues[21]  += (int)vReal[i];
            if (i > 97  && i <= 114 ) bandValues[22]  += (int)vReal[i];
            if (i > 114  && i <= 135 ) bandValues[23]  += (int)vReal[i];
            if (i > 135  && i <= 160 ) bandValues[24] += (int)vReal[i];
            if (i > 160  && i <= 189 ) bandValues[25] += (int)vReal[i];
            if (i > 189  && i <= 225) bandValues[26] += (int)vReal[i];
            if (i > 225 && i <= 264) bandValues[27] += (int)vReal[i];
            if (i > 264 && i <= 330) bandValues[28] += (int)vReal[i];
            if (i > 330 &&  i <=  390) bandValues[29] += (int)vReal[i];
            if (i > 390 &&  i <=  450) bandValues[30] += (int)vReal[i];
            if (i > 450               ) bandValues[31] += (int)vReal[i];

            break;
        }
      }
    }   
}

void loop() {
  ScanInputs( );

    uint32_t mil = millis();

    FastLED.clear();

    AcquireSample();
    _FFT();
    ProcessPeaks();

// Process the FFT data into bar heights
    for (byte band = 0; band < NUM_BANDS; band++) {
      // Scale the bars for the display
      int barHeight = bandValues[band] / AMPLITUDE;
      if (barHeight > TOP) barHeight = TOP;

      // Small amount of averaging between frames
      barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

      // Move peak up
      if (barHeight > peak[band]) {
        peak[band] = min(TOP, barHeight);
        peak_delay[band] = mil;
      }

      rainbowBars(band, barHeight);
      whitePeak(band);

      // Save oldBarHeights for averaging later
      oldBarHeights[band] = barHeight;
    }

    // Decay peak
    EVERY_N_MILLISECONDS(60) {
      for (byte band = 0; band < NUM_BANDS; band++) {
        if(mil - peak_delay[band] > PEAK_DELAY ) {
          if (peak[band] > 0) peak[band] -= 1;
        }
      }
      colorTimer++;
    }

    // Used in some of the patterns
    EVERY_N_MILLISECONDS(10) {
      colorTimer++;
    }

    FastLED.show();
}


// PATTERNS BELOW //
void rainbowBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix->drawPixel(x, y, CHSV((x / BAR_WIDTH) * (255 / NUM_BANDS), 255, 255));
    }
  }
}

void whitePeak(int band) {
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix->drawPixel(x, peakHeight, CHSV(0,0,255));
  }
}
