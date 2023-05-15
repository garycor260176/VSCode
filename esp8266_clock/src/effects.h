#pragma once
#include <Arduino.h>

boolean loadingFlag = true;

#include "utils.h"

uint8_t hue, hue2;                                 // постепенный сдвиг оттенка или какой-нибудь другой цикличный счётчик

#include "sparklesRoutine.h"
#include "fireRoutine.h"
#include "rainbow.h"
#include "colorsRoutine.h"
#include "noise.h"

uint32_t effTimer;

void showEffect(int num) {
    if (millis() - effTimer >= ((num < 6 || num > 14) ? modes[num].speed : 50) ) {
        effTimer = millis();

        switch(num) {
            case 1: sparklesRoutine(modes[num]); break;
            case 2: fireRoutine(modes[num]); break;
            case 3: rainbowVertical(modes[num]); break;
            case 4: rainbowHorizontal(modes[num]); break;
            case 5: madnessNoise(modes[num]); break;
            case 6: cloudNoise(modes[num]); break;
            case 7: lavaNoise(modes[num]); break;
            case 8: plasmaNoise(modes[num]); break;
            case 9: rainbowNoise(modes[num]); break;
            case 10: zebraNoise(modes[num]); break;
            case 11: forestNoise(modes[num]); break;
            case 12: oceanNoise(modes[num]); break;
            case 13: colorRoutine(modes[num]); break;
            case 14: snowRoutine(modes[num]); break;
            case 15: matrixRoutine(modes[num]); break;
            case 16: lightersRoutine(modes[num]); break;
            case 17: BBallsRoutine(modes[num]); break;
            case 18: spiroRoutine(modes[num]); break;
            case 19: PrismataRoutine(modes[num]); break;
            case 20: smokeballsRoutine(modes[num]); break;
            case 21: execStringsFlame(modes[num]); break;
            case 22: Fire2021Routine(modes[num]); break;
            case 23: DNARoutine(modes[num]); break;
            case 24: polarRoutine(modes[num]); break;
            case 25: newMatrixRoutine(modes[num]); break;
            case 26: fairyRoutine(modes[num]); break;
            case 27: LeapersRoutine(modes[num]); break;
            case 28: ballsRoutine(modes[num]); break;
            case 29: picassoSelector(modes[num]); break;
            case 30: lumenjerRoutine(modes[num]); break;
            case 31: lightBallsRoutine(modes[num]); break;
            case 32: nexusRoutine(modes[num]); break;
            case 33: Sinusoid3Routine(modes[num]); break;
/*
            case 10: rainbowStripeNoise(modes[num]); break;
            case 5: colorsRoutine(modes[num]); break;
            case 35: LiquidLampRoutine(modes[num], true); break;
            case 31: stormyRain(modes[num]); break;
            case 33: stormRoutine2(modes[num]); break;
            case 30: cube2dRoutine(modes[num]); break;
            case 31: ringsRoutine(modes[num]); break;
            case 37: WaveRoutine(modes[num]); break;
            case 39: MultipleStreamSmoke(modes[num], false); break;
            case 40: MultipleStream8(modes[num]); break;
            case 41: LLandRoutine(modes[num]); break;
            case 42: magmaRoutine(modes[num]); break;
            case 44: snakesRoutine(modes[num]); break;
*/
        }
    }
}
