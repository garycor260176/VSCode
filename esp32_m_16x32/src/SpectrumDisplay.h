#pragma once

#define PEAK2_DECAY_PER_SECOND  2.2f
#define SHADE_BAND_EDGE         0

class PeakData{
    public:
        float Peaks[BAND_COUNT];

        PeakData(){
            for(int i=0; i<BAND_COUNT;i++){
                Peaks[i] = 0.0f;
            }
        }
};

class SpectrumDisplay{
    private:
        LEDMatrixGFX*   _pMatrix;
        byte            _numberOfBands;
        PeakData        _peaks;
        float           peak1Decay[BAND_COUNT] = {0};
        float           peak2Decay[BAND_COUNT] = {0};
        unsigned long   _lastPeak1Time[BAND_COUNT] = {0};

    public:
        SpectrumDisplay(LEDMatrixGFX* pgfx, byte numberOfBands){
            _pMatrix = pgfx;
            _numberOfBands = numberOfBands;
        }

        void SetPeaks(byte bands, PeakData peakData){
            for(int i = 0; i<bands;i++){
                if(peakData.Peaks[i] > peak1Decay[i]) {
                    peak1Decay[i] = peakData.Peaks[i];
                    _lastPeak1Time[i] = millis();
                }
                if(peakData.Peaks[i] > peak2Decay[i]) {
                    peak2Decay[i] = peakData.Peaks[i];
                }
            }
        }

        void Draw(int baseHue){

        }
};