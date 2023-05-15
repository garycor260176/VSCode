#pragma once

float _oldVU;

const size_t MAX_SAMPLES = 1024;
const size_t SAMPLING_FREQUENCY = 40000;

#define PRINT_PEAKS 0
#define SHOW_SAMPLE_TIMING 0
#define SHOW_FFT_TIMING 0

static int cutOffs32Band[32] = 
{
    10, 20, 25, 31, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000,
    1250, 1600, 2000, 2500, 3150, 4000, 5000, 6400, 8000, 10000, 12500, 16500, 20000,
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

class SampleBuffer {
    private:
        arduinoFFT _FFT;
        size_t _MaxSamples;
        size_t _SamplingFrequency;
        size_t _BandCount;
        float *_vPeaks;
        int _InputPin;
        //static float _oldVU;
        portMUX_TYPE _mutex; 

        int BucketFrequency(int iBucket) const {
            if(iBucket <= 1) {
                return 0;
            }

            int iOffset = iBucket - 2;
            return iOffset * (_SamplingFrequency / 2) / (_MaxSamples / 2);
        }

        static int * BandCutoffTable(int bandCount){
            if(bandCount == 8) return cutOffs8Band;
            if(bandCount == 16) return cutOffs16Band;
            if(bandCount == 24) return cutOffs24Band;
            if(bandCount == 32) return cutOffs32Band;

            return cutOffs32Band;
        }

    public:
        volatile int _cSamples;
        double *_vReal;
        double *_vImaginary;

        SampleBuffer(size_t MaxSamples, size_t BandCount, size_t SamplingFrequency, int InputPin)
        {
            _BandCount = BandCount;
            _SamplingFrequency = SamplingFrequency;
            _MaxSamples = MaxSamples;
            _InputPin = InputPin;

            _vReal = (double*)malloc(MaxSamples * sizeof(_vReal[0]));
            _vImaginary = (double*)malloc(MaxSamples * sizeof(_vImaginary[0]));
            _vPeaks = (float*)malloc(BandCount * sizeof(_vPeaks[0]));
            _oldVU = 0.0f;

            _mutex = portMUX_INITIALIZER_UNLOCKED;
            vPortCPUInitializeMutex(&_mutex);

            Reset();
        }

        ~SampleBuffer(){
            free(_vReal);
            free(_vImaginary);
            free(_vPeaks);
        }

        bool TryForimmediateLock(){
            return vPortCPUAcquireMutexTimeout(&_mutex, portMUX_TRY_LOCK);
        }

        void WaitForLock(){
            vPortCPUAcquireMutex(&_mutex);
        }

        void ReleaseLock(){
            vPortCPUReleaseMutex(&_mutex);
        }

        void Reset(){
            _cSamples = 0;
            for(int i=0; i<_MaxSamples; i++ ){
                _vReal[i] = 0;
                _vImaginary[i] = 0.0f;
            }
            for(int i=0; i<_BandCount; i++ ){
                _vPeaks[i] = 0;
            }
        }

        void FFT(){
            #if SHOW_FFT_TIMING
                unsigned long fftStart = millis();
            #endif

            _FFT.Windowing(_vReal, _MaxSamples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            _FFT.Compute(_vReal, _vImaginary, _MaxSamples, FFT_FORWARD);
            _FFT.ComplexToMagnitude(_vReal, _vImaginary, _MaxSamples);
            _FFT.MajorPeak(_vReal, _MaxSamples, _SamplingFrequency);

            #if SHOW_FFT_TIMING
                Serial.printf("FFT took %ld ms at %d FPS\n", millis() - fftStart, FPS(fftStart, millis()));
            #endif
        }

        void IRAM_ATTR AcquireSample(){
            g_cInterrupts++;

            if(TryForimmediateLock()){
                if(_cSamples < _MaxSamples) {
                    _vReal[_cSamples] = adc1_get_raw(ADC1_CHANNEL_0);
                    //_vReal[_cSamples] = analogRead(_InputPin);
                    _vImaginary[_cSamples] = 0;
                    _cSamples++;
                    g_cSamples++;
                }
                ReleaseLock();
            } else {
                g_cIRQMisses++;
            }
        }

        PeakData GetBandPeaks(){
            PeakData data;
            for(int i=0; i<_BandCount;i++){
                data.Peaks[i] = _vPeaks[i];
            }
            return data;
        }

        void ProcessPeaks(){
            static const int NOISE_CUTOFF = 10;

            float averageSum = 0.0f;
            double samplesPeak = 0.0f;
            for(int i=2; i<_MaxSamples/2; i++){
                averageSum += _vReal[i];
                if(_vReal[i] > samplesPeak) samplesPeak = _vReal[i];
            }

            float t = averageSum / (_MaxSamples/2);
            gVU = max(t, (_oldVU * 3 + t) / 4);
            _oldVU = gVU;

            for(int i=2; i<_MaxSamples/2; i++){
                if(_vReal[i] > powf(NOISE_CUTOFF, gLogScale)) {
                    int freq = BucketFrequency(i);

                    int iBand = 0;
                    while(iBand < _BandCount){
                        if(freq < BandCutoffTable(_BandCount)[iBand]) break;
                        iBand++;
                    }
                    if(iBand > _BandCount) iBand = _BandCount;

                    float scaledValue = _vReal[i];
                    if(scaledValue > _vPeaks[iBand]) _vPeaks[iBand] = scaledValue;
                }
            }

            #if PRINT_PEAKS
                Serial.print("Raws:  ");
                for(int i = 0; i < _BandCount; i++ ){
                    Serial.printf("%8.1f, ", _vPeaks[i]);
                }
                Serial.println("");
            #endif

            if(_BandCount == 16 && gVU > (MAX_VU / 8)) {
                _vPeaks[0] *= 0.3f;
                _vPeaks[1] *= 0.6f;
                _vPeaks[2] *= 0.8f;
                //...
                _vPeaks[9] *= 1.10f;
                _vPeaks[10] *= 1.25f;
                _vPeaks[11] *= 1.40f;
                _vPeaks[12] *= 1.60f;
                _vPeaks[13] *= 1.80f;
                _vPeaks[14] *= 1.90f;
                _vPeaks[15] *= 2.0f;
            }

            for(int i = 0; i < _BandCount; i++ ){
                _vPeaks[i] = powf(_vPeaks[i], gLogScale);
            }

            static float lastAllBandsPeak = 0.0f;
            float allBandsPeak = 0;
            for(int i = 0; i < _BandCount; i++ ){
                allBandsPeak = max( allBandsPeak, _vPeaks[i]);
            }

            if(allBandsPeak < 1) allBandsPeak = 1;

            allBandsPeak = max(allBandsPeak, ((lastAllBandsPeak * (GAIN_DAMPEN - 1)) + allBandsPeak) / GAIN_DAMPEN);
            lastAllBandsPeak = allBandsPeak;

            if(allBandsPeak < powf(2, 26)) {
                allBandsPeak = powf(2, 26);
            }

            for(int i = 0; i < _BandCount; i++ ){
                _vPeaks[i] /= (allBandsPeak * 1.1f);
            }

            gScaler = allBandsPeak;

            #if PRINT_PEAKS
                Serial.print("Aftr:  ");
                for(int i = 0; i < _BandCount; i++ ){
                    Serial.printf("%8.1f, ", _vPeaks[i]);
                }
                Serial.println("");
            #endif
        }
};

class SoundAnalyzer{
    private:
        hw_timer_t *_SamplerTimer = NULL;
        SampleBuffer _bufferA;
        SampleBuffer _bufferB;
        unsigned int _sampling_period_us = PERIOD_FROM_FREQ(SAMPLING_FREQUENCY);
        uint8_t _inputPin;

        static volatile SampleBuffer* _pIRQBuffer;

    public:
        SoundAnalyzer(uint8_t inputPin)
            : _bufferA(MAX_SAMPLES, BAND_COUNT, SAMPLING_FREQUENCY, INPUT_PIN),
              _bufferB(MAX_SAMPLES, BAND_COUNT, SAMPLING_FREQUENCY, INPUT_PIN),
              _sampling_period_us(PERIOD_FROM_FREQ(SAMPLING_FREQUENCY)),
              _inputPin(inputPin)
        {
            _pIRQBuffer = &_bufferA;
        }

        void StartInterrupts(){
            Serial.printf("Continual sampling every %d us for a sample rate of %d Hz. \n", _sampling_period_us, SAMPLING_FREQUENCY);
            _SamplerTimer = timerBegin(0, 80, true);
            timerAttachInterrupt(_SamplerTimer, &OnTimer, true);
            timerAlarmWrite(_SamplerTimer, _sampling_period_us, true);
            timerAlarmEnable(_SamplerTimer);
        }

        static void IRAM_ATTR OnTimer();

        void ScanInputs(){
/*
            //analogSetSamples(1);
            float raw = adc1_get_raw(ADC1_CHANNEL_6);
            //float raw = analogRead(BRIGHTNESS_PIN);
            raw = mapFloat(raw, 0, 4096, 1.5, 10);
            raw = roundf(raw);
            gBrightness = min((float)255, powf(raw, 2.52f));

            raw = adc1_get_raw(ADC1_CHANNEL_3);
            //raw = analogRead(COLOR_SPEED_PIN);
            raw = mapFloat(raw, 0, 4096, 0, MAX_COLOR_SPEED);
            gColorSpeed = raw;

            raw = adc1_get_raw(ADC1_CHANNEL_7);
            //raw = analogRead(PEAK_DECAY_PIN);
            raw = mapFloat(raw, 0, 4096, -0.5f, PEAK2_DECAY_PER_SECOND);
            gPeakDecay = raw;

            raw = adc1_get_raw(ADC1_CHANNEL_4);
            //raw = analogRead(COLOR_SCHEME_PIN);
            raw = mapFloat(raw, 0, 4096, 0, ARRAYSIZE(allPalettes));
            giColorScheme = raw;   

            //analogSetSamples(SUPERSAMPLES);
*/
        }

        PeakData RunSamplerPass(int bandCount){
            SampleBuffer *pBackBuffer = nullptr;
            for(;;){
                if(_bufferA._cSamples == MAX_SAMPLES){
                    portDISABLE_INTERRUPTS();
                    ScanInputs();
                    _bufferB.Reset();
                    _pIRQBuffer = &_bufferB;
                    pBackBuffer = &_bufferA;
                    portENABLE_INTERRUPTS();
                    break;
                }
                if(_bufferB._cSamples == MAX_SAMPLES){
                    portDISABLE_INTERRUPTS();
                    ScanInputs();
                    _bufferA.Reset();
                    _pIRQBuffer = &_bufferA;
                    pBackBuffer = &_bufferB;
                    portENABLE_INTERRUPTS();
                    break;
                }
                delay(0);
            }

            pBackBuffer->WaitForLock();
            pBackBuffer->FFT();
            pBackBuffer->ProcessPeaks();
            PeakData peaks = pBackBuffer->GetBandPeaks();
            pBackBuffer->Reset();
            pBackBuffer->ReleaseLock();

            return peaks;
        }
};

volatile SampleBuffer* SoundAnalyzer::_pIRQBuffer;

void IRAM_ATTR SoundAnalyzer::OnTimer(){
    ((SampleBuffer*)_pIRQBuffer)->AcquireSample();
}

SoundAnalyzer g_SoundAnalyzer(INPUT_PIN);