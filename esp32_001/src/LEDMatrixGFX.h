#pragma once

#include <Arduino.h>

class LEDMatrixGFX: public Adafruit_GFX
{
    private:
        CRGB * _pLEDs = nullptr;
        size_t _width;
        size_t _height;

    public:
        LEDMatrixGFX(size_t w, size_t h, int brightness = 255) 
            : Adafruit_GFX((int)w, (int)h),
            _width(w),
            _height(h)
        {
            _pLEDs = static_cast<CRGB*>(calloc(w*h, sizeof(CRGB)));
            FastLED.addLeds<WS2812B, LED_PIN, GRB>(_pLEDs, w*h);
            FastLED.setBrightness(brightness);
        }

        ~LEDMatrixGFX( ) {
            free( _pLEDs );
            _pLEDs = nullptr;
        }

        inline virtual void drawPixel(int16_t x, int16_t y, uint16_t color) {
            _pLEDs[getPixelIndex(x, y)] = from16Bit(color);
        }

        inline virtual void drawPixel(int16_t x, int16_t y, CRGB color) {
            _pLEDs[getPixelIndex(x, y)] = color;
        }

        static const byte gamma5[];
        static const byte gamma6[];

        inline static CRGB from16Bit(uint16_t color){
            byte r = gamma5[color >> 11];
            byte g = gamma6[(color >> 5) & 0x3F];
            byte b = gamma5[color & 0x1F];

            return CRGB(r, g, b);
        }

        static inline uint16_t to16bit(uint8_t r, uint8_t g, uint8_t b){
            return ((r / 8) << 1) | ((g / 4) << 5) | (b / 8);
        }

        static inline uint16_t to16bit(const CRGB rgb){
            return ((rgb.r / 8) << 1) | ((rgb.g / 4) << 5) | (rgb.b / 8);
        }

        static inline uint16_t to16bit(CRGB::HTMLColorCode code){
            return to16bit(CRGB(code));
        }

        inline uint16_t getPixelIndex(int16_t x, int16_t y) const
        {
            if(x & 0x01)
            {
                uint8_t reverseY = (_height - 1) - y;
                return (x * _height) + reverseY;
            } else {
                return (x * _height) + y;
            }
        }

        inline CRGB getPixel(int16_t x, int16_t y) const
        {
            return _pLEDs[getPixelIndex(x, y)];
        }

        void ShowMatrix() {
            FastLED.show();
        }

        void setBrightness(byte brightness){
            FastLED.setBrightness(brightness);
        }
};

const byte  LEDMatrixGFX::gamma5[] = 
{
  0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0b,
  0x0e, 0x11, 0x14, 0x18, 0x1d, 0x22, 0x28, 0x2e,
  0x36, 0x3d, 0x46, 0x4f, 0x59, 0x64, 0x6f, 0x7c,
  0x89, 0x97, 0xa6, 0xb6, 0xc7, 0xd9, 0xeb, 0xff
};

const byte  LEDMatrixGFX::gamma6[] = 
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
  0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x10, 0x12, 0x13,
  0x15, 0x17, 0x19, 0x1b, 0x1d, 0x20, 0x22, 0x25,
  0x27, 0x2a, 0x2d, 0x30, 0x33, 0x37, 0x3a, 0x3e,
  0x41, 0x45, 0x49, 0x4d, 0x52, 0x56, 0x5b, 0x5f,
  0x64, 0x69, 0x6e, 0x74, 0x79, 0x7f, 0x85, 0x8b,
  0x91, 0x97, 0x9d, 0xa4, 0xab, 0xb2, 0xb9, 0xc0,
  0xc7, 0xcf, 0xd6, 0xde, 0xe6, 0xee, 0xf7, 0xff
};