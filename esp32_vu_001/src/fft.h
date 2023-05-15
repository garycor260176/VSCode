#pragma once
// Depending on how many bands have been defined, one of these tables will contain the frequency
// cutoffs for that "size" of a spectrum display. 
// Only one of the following should be 1, rest has to be 0 
// WARNING amke sure your math add up.. if you select 24 bands on a 32 wide matrix....you'll need to adjust code 
#define bands8  0
#define bands16 0
#define bands24 0
#define bands32 1
#define bands64 0  

int numBands = 16;
int CalibrationType=0;  //0=none, 1=White, 2=Pink     
double bndcounter[64];

//#if bands8
  static int BandCutoffTable_8[8] =
  {
   20, 150, 400, 550, 675, 780, 900, 1200
  };

  static int BandCutoffTable2_8[8] = 
  {

    3, 6, 13, 27, 55,  112,  229,  500
  };

  static double BandCalibration_Pink_8[8]=
  { 4.08, 1.56, 1.00, 2.80, 3.23, 3.63, 6.19, 1.65 };
  static double BandCalibration_White_8[8]=
  { 25.84, 3.43, 1.10, 2.95, 3.37, 3.39, 5.46, 1.00 };
  static double BandCalibration_Brown_8[8]=
  { 2.72, 1.17, 1.00, 3.34, 4.83, 5.57, 11.18, 3.08 };

//#if bands16
  static int BandCutoffTable_16[16] =
  {
   100, 250, 450, 565, 715, 900, 1125, 1400, 1750, 2250, 2800, 3150, 4000, 5000, 6400, 12500
  };

static int BandCutoffTable2_16[16] = 
  {

    2, 3, 5, 7, 9,  13,  18,  25, 36, 50, 69, 97, 135, 189, 264, 500
  };

  static double BandCalibration_Pink_16[16]=
{ 4.52, 5.48, 5.54, 6.06, 2.98, 1.72, 1.49, 1.36, 1.00, 1.49, 2.04, 1.71, 2.19, 2.68, 1.85, 5.69 };

  static double BandCalibration_White_16[16]=
{ 169.55, 148.58, 141.29, 124.24, 35.59, 10.76, 6.97, 5.18, 2.89, 4.19, 4.24, 1.99, 1.00, 1.60, 1.53, 5.48 }; 
    static double BandCalibration_Brown_16[16]=
{ 1.81, 2.17, 2.49, 2.89, 1.57, 1.00, 1.10, 1.30, 1.22, 3.74, 113.96, 774.90, 7.76, 645.75, 2583.00, 7749.00 };

//#if bands32
  static int BandCutoffTable_32[32] = 
  {

    10, 20, 25,  31,  40,  50,  63,  80,  100, 125, 160, 200, 250, 315, 400,  500, 
    630,  800, 1000,  1250, 1600, 2000, 2500, 3150, 4000,  5000,  6400,  8000, 10000, 12500, 16500, 20000
  };

  static int BandCutoffTable2_32[32] = 
  {

    3,  4,  5,  6,  7,  8,   9,   11,  13,  16,  18,  22,  25,  30,  36,  43,
    50, 59, 69, 80, 97, 114, 135, 150, 179, 215, 254, 320, 360, 450, 500, 600
  };


  static double BandCalibration_Pink_32[32]=
  { 1.17, 3.09, 3.07, 3.53, 3.95, 4.22, 4.43, 5.02, 5.59, 5.63, 6.27, 7.32, 1.90, 1.51, 1.66, 1.00, 1.31, 1.31, 2.58, 4.25, 1.78, 1.50, 2.75, 2.95, 2.34, 4.81, 2.41, 2.16, 2.08, 7.30, 9.01, 10.48 };
  static double BandCalibration_White_32[32]=
  { 22.21, 65.51, 62.36, 60.53, 57.51, 52.28, 48.14, 49.53, 49.06, 50.49, 58.15, 60.89, 9.73, 6.36, 6.01, 3.17, 3.67, 3.59, 6.76, 9.03, 2.86, 1.91, 2.09, 1.16, 1.00, 2.08, 1.64, 1.62, 1.73, 4.89, 6.98, 9.08 };
  static double BandCalibration_Brown_32[32]=
  { 1.00, 2.82, 3.26, 3.68, 4.21, 5.15, 5.57, 5.98, 7.06, 7.53, 9.94, 11.47, 3.22, 2.88, 3.53, 2.48, 3.88, 5.37, 43.13, 312.68, 224.49, 729.58, 2188.75, 2188.75, 12.35, 8755, 2188.75, 1250.71, 2918.33, 8755, 8755.00, 8755.00 };

static int * BandCutoffTable2(int bandCount){
    if(bandCount == 8) return BandCutoffTable2_8;
    if(bandCount == 16) return BandCutoffTable2_16;
    if(bandCount == 32) return BandCutoffTable2_32;

    return BandCutoffTable2_32;
}

static int * BandCutoffTable(int bandCount){
    if(bandCount == 8) return BandCutoffTable_8;
    if(bandCount == 16) return BandCutoffTable_16;
    if(bandCount == 32) return BandCutoffTable_32;

    return BandCutoffTable_32;
}

static double * BandCalibration_Pink(int bandCount){
    if(bandCount == 8) return BandCalibration_Pink_8;
    if(bandCount == 16) return BandCalibration_Pink_16;
    if(bandCount == 32) return BandCalibration_Pink_32;

    return BandCalibration_Pink_32;
}

static double * BandCalibration_White(int bandCount){
    if(bandCount == 8) return BandCalibration_White_8;
    if(bandCount == 16) return BandCalibration_White_16;
    if(bandCount == 32) return BandCalibration_White_32;

    return BandCalibration_White_32;
}

static double * BandCalibration_Brown(int bandCount){
    if(bandCount == 8) return BandCalibration_Brown_8;
    if(bandCount == 16) return BandCalibration_Brown_16;
    if(bandCount == 32) return BandCalibration_Brown_32;

    return BandCalibration_Brown_32;
}