#pragma once
#include <Arduino.h>
#include <driver/i2s.h>
#include <driver/adc.h>

//Settings
const int samplingFrequency = 44100;
const int SAMPLEBLOCK = 1024;
const i2s_port_t I2S_PORT = I2S_NUM_0;

void setupI2S() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); //GPIO36 - audion input
//    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11); //GPIO35 - peakdelay
//    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); //GPIO32 - brightness
}