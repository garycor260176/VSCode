#pragma once

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#define PERIOD_FROM_FREQ(f) (round(1000000 * (1.0 / f)))
#define FREQ_FROM_PERIOD(p) (1.0 / p * 1000000)

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max){
    return ( x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int FPS(unsigned long start, unsigned long end){
    unsigned long msDuration = end - start;
    float fpsf = 1.0f / msDuration / (float)MS_PER_SECOND;
    int FPS = (int)fpsf;
    if(FPS > 999) FPS = 999;
    return FPS;
}