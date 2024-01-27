#pragma once

#include <Arduino.h>
#include <FastLED_NeoMatrix.h>

#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная

#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH    HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"
#endif

#define SPEED_DEFAULT 30
#define SCALE_DEFAULT 40
boolean loadingFlag = true;

struct s_mode {
  byte speed = SPEED_DEFAULT;
  byte scale = SCALE_DEFAULT;

  byte old_speed = SPEED_DEFAULT;
  byte old_scale = SCALE_DEFAULT;
} modes[1];

void showEffect(int scale);
void picassoSelector(s_mode mode);
void PicassoRoutine();
void PicassoRoutine2();
void PicassoRoutine3();
void PicassoGenerate(bool reset);
void PicassoPosition();
void DrawLine(int x1, int y1, int x2, int y2, CRGB color);
void drawPixelXY(int8_t x, int8_t y, CRGB color);
void blurScreen(fract8 blur_amount, CRGB *LEDarray = leds);
void dimAll(uint8_t value, CRGB *LEDarray = leds);
void drawCircleF(float x0, float y0, float radius, CRGB color);
void drawPixelXYF(float x, float y, CRGB color);
uint32_t getPixColorXY(int8_t x, int8_t y);
uint32_t getPixColor(int thisSegm);
uint16_t getPixelNumber(int8_t x, int8_t y);
void DrawLineF(float x1, float y1, float x2, float y2, CRGB color);
uint16_t XY(uint8_t x, uint8_t y);

uint32_t effTimer;
uint8_t enlargedObjectNUM;                                       // используемое в эффекте количество объектов
#define enlargedOBJECT_MAX_COUNT                     (WIDTH * 2) // максимальное количество сложных отслеживаемых объектов (меньше, чем trackingOBJECT_MAX_COUNT)
#define trackingOBJECT_MAX_COUNT                         (100U)  // максимальное количество отслеживаемых объектов (очень влияет на расход памяти)
float   trackingObjectPosX[trackingOBJECT_MAX_COUNT];
float   trackingObjectPosY[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectHue[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectState[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedY[trackingOBJECT_MAX_COUNT];
float   trackingObjectShift[trackingOBJECT_MAX_COUNT];
#define SEGMENTS 1

void showEffect(int scale) {
  //modes[0].scale = scale;
  if (millis() - effTimer >= 50 ) {
      effTimer = millis();
      picassoSelector(modes[0]);
  }
}

void picassoSelector(s_mode mode){
    if (loadingFlag)
    {
        if (mode.scale < 34U)           // если масштаб до 34
            enlargedObjectNUM = (mode.scale - 1U) / 32.0 * (enlargedOBJECT_MAX_COUNT - 3U) + 3U;
        else if (mode.scale >= 68U)      // если масштаб больше 67
            enlargedObjectNUM = (mode.scale - 68U) / 32.0 * (enlargedOBJECT_MAX_COUNT - 3U) + 3U;
        else                                          // для масштабов посередине
            enlargedObjectNUM = (mode.scale - 34U) / 33.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    }
    
    if (mode.scale < 34U)           // если масштаб до 34
        PicassoRoutine();
    else if (mode.scale > 67U)      // если масштаб больше 67
        PicassoRoutine3();
    else                                          // для масштабов посередине
        PicassoRoutine2();
}

void PicassoRoutine(){
  PicassoGenerate(false);
  PicassoPosition();

  for (uint8_t i = 0 ; i < enlargedObjectNUM - 2U ; i+=2) 
    DrawLine(trackingObjectPosX[i], trackingObjectPosY[i], trackingObjectPosX[i+1U], trackingObjectPosY[i+1U], CHSV(trackingObjectHue[i], 255U, 255U));

  EVERY_N_MILLIS(20000){
    PicassoGenerate(true);
  }

  blurScreen(80);
}

void PicassoGenerate(bool reset){
    if (loadingFlag)
    {
        loadingFlag = false;
        if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
        if (enlargedObjectNUM < 2U) enlargedObjectNUM = 2U;

        double minSpeed = 0.2, maxSpeed = 0.8;
        
        for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) { 
        trackingObjectPosX[i] = random8(WIDTH);
        trackingObjectPosY[i] = random8(HEIGHT);

        trackingObjectHue[i] = random8();

        trackingObjectSpeedY[i] = +((-maxSpeed / 3) + (maxSpeed * (float)random8(1, 100) / 100));
        trackingObjectSpeedY[i] += trackingObjectSpeedY[i] > 0 ? minSpeed : -minSpeed;

        trackingObjectShift[i] = +((-maxSpeed / 2) + (maxSpeed * (float)random8(1, 100) / 100));
        trackingObjectShift[i] += trackingObjectShift[i] > 0 ? minSpeed : -minSpeed;

        trackingObjectState[i] = trackingObjectHue[i];
        }
    }
    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
        if (reset) {
            trackingObjectState[i] = random8();
            trackingObjectSpeedX[i] = (trackingObjectState[i] - trackingObjectHue[i]) / 25;
        }
        if (trackingObjectState[i] != trackingObjectHue[i] && trackingObjectSpeedX[i]) {
            trackingObjectHue[i] += trackingObjectSpeedX[i];
        }
    }
}

void PicassoPosition(){
    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) { 
        if (trackingObjectPosX[i] + trackingObjectSpeedY[i] > WIDTH || trackingObjectPosX[i] + trackingObjectSpeedY[i] < 0) {
        trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
        }

        if (trackingObjectPosY[i] + trackingObjectShift[i] > HEIGHT || trackingObjectPosY[i] + trackingObjectShift[i] < 0) {
        trackingObjectShift[i] = -trackingObjectShift[i];
        }

        trackingObjectPosX[i] += trackingObjectSpeedY[i];
        trackingObjectPosY[i] += trackingObjectShift[i];
    };
}

void PicassoRoutine3(){
  PicassoGenerate(false);
  PicassoPosition();
  dimAll(180);

  for (uint8_t i = 0 ; i < enlargedObjectNUM - 2U ; i+=2) 
    drawCircleF(fabs(trackingObjectPosX[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosY[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosX[i] - trackingObjectPosY[i]), CHSV(trackingObjectHue[i], 255U, 255U));
    //drawCircleF(fabs(trackingObjectPosX[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosY[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosX[i] - trackingObjectPosY[i]), ColorFromPalette(*curPalette, trackingObjectHue[i]));
    
  EVERY_N_MILLIS(20000){
    PicassoGenerate(true);
  }

  blurScreen(80);
}

void PicassoRoutine2(){
  PicassoGenerate(false);
  PicassoPosition();
  dimAll(180);

  for (uint8_t i = 0 ; i < enlargedObjectNUM - 1U ; i++) 
    DrawLineF(trackingObjectPosX[i], trackingObjectPosY[i], trackingObjectPosX[i+1U], trackingObjectPosY[i+1U], CHSV(trackingObjectHue[i], 255U, 255U));

  EVERY_N_MILLIS(20000){
    PicassoGenerate(true);
  }

  blurScreen(80);
}

void DrawLine(int x1, int y1, int x2, int y2, CRGB color)
{
  int deltaX = abs(x2 - x1);
  int deltaY = abs(y2 - y1);
  int signX = x1 < x2 ? 1 : -1;
  int signY = y1 < y2 ? 1 : -1;
  int error = deltaX - deltaY;

  drawPixelXY(x2, y2, color);
  while (x1 != x2 || y1 != y2) {
      drawPixelXY(x1, y1, color);
      int error2 = error * 2;
      if (error2 > -deltaY) {
          error -= deltaY;
          x1 += signX;
      }
      if (error2 < deltaX) {
          error += deltaX;
          y1 += signY;
      }
  }
}

void drawPixelXY(int8_t x, int8_t y, CRGB color) {
  if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1) return;
  int thisPixel = getPixelNumber(x, y) * SEGMENTS;
  for (byte i = 0; i < SEGMENTS; i++) {
    leds[thisPixel + i] = color;
  }
}

void blurScreen(fract8 blur_amount, CRGB *LEDarray)
{
  blur2d(LEDarray, WIDTH, HEIGHT, blur_amount);
}

void dimAll(uint8_t value, CRGB *LEDarray) {
  nscale8(LEDarray, NUM_LEDS, value);
}

void drawCircleF(float x0, float y0, float radius, CRGB color){
  float x = 0, y = radius, error = 0;
  float delta = 1. - 2. * radius;

  while (y >= 0) {
    drawPixelXYF(fmod(x0 + x +WIDTH,WIDTH), y0 + y, color); // сделал, чтобы круги были бесшовными по оси х
    drawPixelXYF(fmod(x0 + x +WIDTH,WIDTH), y0 - y, color);
    drawPixelXYF(fmod(x0 - x +WIDTH,WIDTH), y0 + y, color);
    drawPixelXYF(fmod(x0 - x +WIDTH,WIDTH), y0 - y, color);
    error = 2. * (delta + y) - 1.;
    if (delta < 0 && error <= 0) {
      ++x;
      delta += 2. * x + 1.;
      continue;
    }
    error = 2. * (delta - x) - 1.;
    if (delta > 0 && error > 0) {
      --y;
      delta += 1. - 2. * y;
      continue;
    }
    ++x;
    delta += 2. * (x - y);
    --y;
  }
}

void drawPixelXYF(float x, float y, CRGB color) //, uint8_t darklevel = 0U)
{
//  if (x<0 || y<0) return; //не похоже, чтобы отрицательные значения хоть как-нибудь учитывались тут // зато с этой строчкой пропадает нижний ряд
  // extract the fractional parts and derive their inverses
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    CRGB clr = getPixColorXY(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
//if (darklevel) drawPixelXY(xn, yn, makeDarker(clr, darklevel));
//else
    drawPixelXY(xn, yn, clr);
  }
}

uint32_t getPixColorXY(int8_t x, int8_t y) {
  return getPixColor(getPixelNumber(x, y));
}

uint32_t getPixColor(int thisSegm) {
  int thisPixel = thisSegm * SEGMENTS;
  if (thisPixel < 0 || thisPixel > NUM_LEDS - 1) return 0;
  return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8 ) | (long)leds[thisPixel].b);
}

// получить номер пикселя в ленте по координатам
uint16_t getPixelNumber(int8_t x, int8_t y) {
  if ((THIS_Y % 2 == 0) || MATRIX_TYPE) {               // если чётная строка
    return (THIS_Y * _WIDTH + THIS_X);
  } else {                                              // если нечётная строка
    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);
  }
}

void DrawLineF(float x1, float y1, float x2, float y2, CRGB color){
    float deltaX = std::fabs(x2 - x1);
    float deltaY = std::fabs(y2 - y1);
    float error = deltaX - deltaY;

    float signX = x1 < x2 ? 0.5 : -0.5;
    float signY = y1 < y2 ? 0.5 : -0.5;

    while (x1 != x2 || y1 != y2) { // (true) - а я то думаю - "почему функция часто вызывает вылет по вачдогу?" А оно вон оно чё, Михалычь!
        if ((signX > 0 && x1 > x2+signX) || (signX < 0 && x1 < x2+signX)) break;
        if ((signY > 0 && y1 > y2+signY) || (signY < 0 && y1 < y2+signY)) break;
        drawPixelXYF(x1, y1, color); // интересно, почему тут было обычное drawPixelXY() ???
        float error2 = error;
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

uint16_t XY(uint8_t x, uint8_t y)
{
  if (!(THIS_Y & 0x01) || MATRIX_TYPE)               // Even rows run forwards
    return (THIS_Y * _WIDTH + THIS_X);
  else                                                  
    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);  // Odd rows run backwards
}
