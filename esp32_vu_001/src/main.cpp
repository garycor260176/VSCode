#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <arduinoFFT.h>
#include <FastLED_NeoMatrix.h>
#include <driver/adc.h>
#include "I2SPLUGIN.h"
#include "fft.h"
#include "Settings.h"
#include "leddriver.h"
#include "patterns.h"
#include "fire.h"
#include "buttons.h"

int skip=true;
unsigned long newTime;
unsigned int sampling_period_us;

arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLEBLOCK, samplingFrequency);
long LastDoNothingTime = 0;                       // only needed for screensaver
#define up  1
#define down 0
int PeakDirection=0;

button btn_mode(PIN_MODE); // указываем пин
button btn_band(PIN_BAND); // указываем пин

void setup() {
  Serial.begin(115200);

  pinMode(PIN_MODE, INPUT);

  Serial.println("Setting up Audio Input I2S");
  sampling_period_us = round(1000000 * (1.0 / samplingFrequency));
  setupI2S();
  Serial.println("Audio input setup completed");
  delay(1000);

  SetupLEDSTRIP();
}

// BucketFrequency
//
// Return the frequency corresponding to the Nth sample bucket.  Skips the first two 
// buckets which are overall amplitude and something else.
int BucketFrequency(int iBucket){
  if (iBucket <= 1) return 0;
  int iOffset = iBucket - 2;
  return iOffset * (samplingFrequency / 2) / (SAMPLEBLOCK / 2);
}

void DrawVUPixels(int i, int yVU, int fadeBy = 0){ 
  CRGB VUC;
  if (i>(PANE_WIDTH/3)){
    VUC.r=255;
    VUC.g=0;
    VUC.b=0 ;
  }
  else if (i>(PANE_WIDTH/5)){
    VUC.r=255;
    VUC.g=255;
    VUC.b=0;
  }
  else{ // green
    VUC.r=0;
    VUC.g=255;
    VUC.b=0;
  }
  
  int xHalf = matrix->width()/2;
  matrix->drawPixel(xHalf-i-1, yVU, CRGB(VUC.r,VUC.g,VUC.b).fadeToBlackBy(fadeBy));
  matrix->drawPixel(xHalf+i,   yVU, CRGB(VUC.r,VUC.g,VUC.b).fadeToBlackBy(fadeBy));
}

void DrawVUMeter(int yVU){
  static int iPeakVUy = 0;        // size (in LED pixels) of the VU peak
  static unsigned long msPeakVU = 0;       // timestamp in ms when that peak happened so we know how old it is
  const int MAX_FADE = 256;

  matrix->fillRect(0, yVU, matrix->width(), 1, 0x0000);

  if (iPeakVUy > 1){
    int fade = MAX_FADE * (millis() - msPeakVU) / (float) 1000;
    DrawVUPixels(iPeakVUy,   yVU, fade);
  }
  int xHalf = (PANE_WIDTH/2)-1;
  int bars  = map(gVU, 0, MAX_VU, 1, xHalf);
  bars = min(bars, xHalf);
  if(bars > iPeakVUy){
    msPeakVU = millis();
    iPeakVUy = bars;
  }
  else if (millis() - msPeakVU > 1000)iPeakVUy = 0;
  for (int i = 0; i < bars; i++)DrawVUPixels(i, yVU);
}

void make_fire() {
  uint16_t i, j;
  
  if (t > millis()) return;
  t = millis() + (1000 / FPS);

  // First, move all existing heat points up the display and fade
 
  for (i =  rows - 1; i > 0; --i) {
    for (j = 0; j < cols; ++j) {
      uint8_t n = 0;
      if (pix[i - 1][j] > 0)
        n = pix[i - 1][j] - 1;
      pix[i][j] = n;
    }
  }

  // Heat the bottom row
  for (j = 0; j < cols; ++j) {
    i = pix[0][j];
    if (i > 0) {
      pix[0][j] = random(NCOLORS - 6, NCOLORS - 2);
    }
  }

  // flare
  for (i = 0; i < nflare; ++i) {
    int x = flare[i] & 0xff;
    int y = (flare[i] >> 8) & 0xff;
    int z = (flare[i] >> 16) & 0xff;
    glow(x, y, z);
    if (z > 1) {
      flare[i] = (flare[i] & 0xffff) | ((z - 1) << 16);
    } else {
      // This flare is out
      for (int j = i + 1; j < nflare; ++j) {
        flare[j - 1] = flare[j];
      }
      --nflare;
    }
  }
  newflare();

  // Set and draw
  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
     // matrix -> drawPixel(j, rows - i, colors[pix[i][j]]);
     CRGB COlsplit=colors[pix[i][j]];
     matrix -> drawPixel(j, rows - i, colors[pix[i][j]]);
    }
  }
}

void ScanInputs( ){
  if (btn_mode.click()){
    buttonPushCounter++;
    if(buttonPushCounter > 12) {
      buttonPushCounter = 0;
    }
  }

  if (btn_band.click()){
    switch(numBands){
      case 8:   numBands = 16; break;
      case 16:  numBands = 32; break;
      case 32:  numBands = 8; break;
      default:  numBands = 32; break;
    } 
    Serial.println(String(numBands));
  }

  float raw = adc1_get_raw(ADC1_CHANNEL_7);
  raw = 4095 - raw;
  if(raw < 0) raw = 0;
  Peakdelay =  map(raw,0,4095,1,100);
  Peakdelay = 10;

  raw = adc1_get_raw(ADC1_CHANNEL_4);
  raw = 4095 - raw;
  if(raw < 0) raw = 0;
  BRIGHTNESSMARK = map(raw,0,4095,BRIGHTNESSMIN, BRIGHTNESSMAX);
//!!! потом убрать
  BRIGHTNESSMARK = 100;
  FastLED.setBrightness(BRIGHTNESSMARK);
}

void loop() {
  size_t bytesRead = 0;
  int TempADC=0;

  ScanInputs( );
  
  //############ Step 1: read samples from the I2S Buffer ##################
    for (int i = 0; i < SAMPLEBLOCK; i++) {
      newTime = micros();
      vReal[i] = adc1_get_raw(ADC1_CHANNEL_0);
      vImag[i] = 0;
      while ((micros() - newTime) < sampling_period_us) {  }
    }

  BAR_WIDTH  = kMatrixWidth /(numBands );

 //############ Step 3: Do FFT on the VReal array  ############
  // compute FFT
  FFT.DCRemoval();
  FFT.Windowing(vReal, SAMPLEBLOCK, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLEBLOCK, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLEBLOCK);
  FFT.MajorPeak(vReal, SAMPLEBLOCK, samplingFrequency); 
  for (int i = 0; i < numBands; i++) {
    FreqBins[i] = 0;
  }  

  //############ Step 4: Fill the frequency bins with the FFT Samples ############
  float averageSum = 0.0f;
  for (int i = 2; i < SAMPLEBLOCK / 2; i++){ 
    if (vReal[i] > NoiseTresshold){
      averageSum+=vReal[i];
      int freq = i; //BucketFrequency(i);
      int iBand = 0;
      while (iBand < numBands){
        //if (freq < BandCutoffTable[iBand])break;
        if (freq < BandCutoffTable2(numBands)[iBand])break;
        iBand++;
      }
      if (iBand > numBands) iBand = numBands;
      FreqBins[iBand]+= vReal[i]; 
    }
  }

    //############ Step 5: Determine the VU value  and mingle in the readout...( cheating the bands ) ############ Step 
  float t = averageSum / (SAMPLEBLOCK / 2);
  gVU = max(t, (oldVU * 3 + t) / 4);
  oldVU = gVU; 
  if(gVU>DemoTreshold)LastDoNothingTime = millis(); // if there is signal in any off the bands[>2] then no demo mode

  for(int j=0;j<numBands;j++){
    if (CalibrationType==1) FreqBins[j] *= BandCalibration_Pink(numBands)[j];
    else if (CalibrationType==2) FreqBins[j] *= BandCalibration_White(numBands)[j];
    else if (CalibrationType==3) FreqBins[j] *= BandCalibration_Brown(numBands)[j];

  }

//############ Step 6: Averaging and making it all fit on screen 
  static float lastAllBandsPeak = 0.0f;
  float allBandsPeak = 0;
  for (int i = 0; i < numBands; i++){
    if (FreqBins[i] > allBandsPeak){
      allBandsPeak = FreqBins[i];
    }
  }   
  if (allBandsPeak < 1)allBandsPeak = 1;
  //  The followinf picks allBandsPeak if it's gone up.  If it's gone down, it "averages" it by faking a running average of GAIN_DAMPEN past peaks
  allBandsPeak = max(allBandsPeak, ((lastAllBandsPeak * (GAIN_DAMPEN-1)) + allBandsPeak) / GAIN_DAMPEN);  // Dampen rate of change a little bit on way down
  lastAllBandsPeak = allBandsPeak;


  if (allBandsPeak < 80000) allBandsPeak = 80000;
  for (int i = 0; i < numBands; i++){ 
    FreqBins[i] /= (allBandsPeak * 1.0f);
  }

 // Process the FFT data into bar heights
  for (int band = 0; band < numBands; band++) {
    int barHeight = FreqBins[band] * kMatrixHeight - 1;  //(AMPLITUDE);
    if (barHeight > TOP-2) barHeight = TOP-2;
    
    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(TOP, barHeight);
      PeakFlag[band]=1;
    }
    bndcounter[band] += barHeight; // ten behoeve calibratie

    #if BottomRowAlwaysOn
      if (barHeight==0)barHeight=1; // make sure there is always one bar that lights up
    #endif

    switch (buttonPushCounter) {
      case 0:
        changingBarsLS(band, barHeight);
        break;
      case 1:
        TriBarLS(band, barHeight);
        TriPeakLS(band);        
        break;
      case 2:
        rainbowBarsLS(band, barHeight);
        NormalPeakLS(band, PeakColor1);
        break;
      case 3:
        purpleBarsLS(band, barHeight);
        NormalPeakLS(band, PeakColor2);
        break;
      case 4:
        SameBarLS(band, barHeight); 
        NormalPeakLS(band, PeakColor3);
        break;
      case 5:
        SameBar2LS(band, barHeight); 
        NormalPeakLS(band, PeakColor3);
        break;
      case 6:
        centerBarsLS(band, barHeight);
        break;
      case 7:
        centerBars2LS(band, barHeight);
        break;
      case 8:
        centerBars3LS(band, barHeight);
        break;
      case 9:
        BlackBarLS(band, barHeight);
        outrunPeakLS(band);
        break;
      case 10:
        BlackBarLS(band, barHeight);
        NormalPeakLS(band, PeakColor5);
        break;
      case 11:
        BlackBarLS(band, barHeight);
        TriPeak2LS(band);
        break;
      case 12:
        matrix->fillRect(0, 0, matrix->width(), 1, 0x0000); // delete the VU meter
        make_fire();
        break;
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
  }

  if (buttonPushCounter!=12) DrawVUMeter(0); // Draw it when not in screensaver mode

   // Decay peak
  EVERY_N_MILLISECONDS(Fallingspeed){
    for (byte band = 0; band < numBands; band++){
      if(PeakFlag[band]==1){
        PeakTimer[band]++;
        if (PeakTimer[band]> Peakdelay){PeakTimer[band]=0;PeakFlag[band]=0;}
      }
      else if ((peak[band] > 0) && (PeakDirection==up)){ 
        peak[band] += 1;
        if (peak[band]>(kMatrixHeight+10))peak[band]=0;
        } // when to far off screen then reset peak height
      else if ((peak[band] > 0)&&(PeakDirection==down)){ peak[band] -= 1;}
    }   
    colorTimer++;
  }

  EVERY_N_MILLISECONDS(10)colorTimer++; // Used in some of the patterns
  
  EVERY_N_SECONDS(SecToChangePattern) {
    if (autoChangePatterns){ 
      buttonPushCounter = (buttonPushCounter + 1) % 12;
    }
  }

  delay(1); // needed to give fastled a minimum recovery time
  FastLED.show();

}

