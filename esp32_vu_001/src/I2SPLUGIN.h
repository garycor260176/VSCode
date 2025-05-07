#pragma once
#include <Arduino.h>
#include <driver/i2s.h>
#include <driver/adc.h>

//Settings
const int samplingFrequency = 40000; //44100;
const int SAMPLEBLOCK = 1024;
const i2s_port_t I2S_PORT = I2S_NUM_0;

void setupI2S() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); //GPIO36 - audion input
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11); //GPIO35 - peakdelay
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); //GPIO32 - brightness

/*
    Serial.println("Configuring I2S...");
    esp_err_t err;

  // The I2S config as per the example
    const i2s_config_t i2s_config = { 
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = samplingFrequency,                        
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // although the SEL config should be left, it seems to transmit on right
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 2,                           // number of buffers
      .dma_buf_len = SAMPLEBLOCK,                     // samples per buffer
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0
    };


  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.

    err = adc_gpio_init(ADC_UNIT_1, ADC_CHANNEL_0); //step 1
    if (err != ESP_OK) {
      Serial.printf("Failed setting up adc channel: %d\n", err);
      while (true);
    }

    err = adc_gpio_init(ADC_UNIT_1, ADC_CHANNEL_3); //step 1
    if (err != ESP_OK) {
        Serial.printf("Failed setting up adc channel: %d\n", err);
        while (true);
    }

    err = i2s_driver_install(I2S_NUM_0, &i2s_config,  0, NULL);  //step 2
    if (err != ESP_OK) {
        Serial.printf("Failed installing driver: %d\n", err);
        while (true);
    }


    err = i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_0);
        if (err != ESP_OK) {
        Serial.printf("Failed setting up adc mode: %d\n", err);
        while (true);
    }

    Serial.println("I2S driver installed.");
*/
}