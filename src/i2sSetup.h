#ifndef I2S_SETUP_H
#define I2S_SETUP_H

#include <driver/i2s.h>
#include "config.h"

const float AUDIO_SR = 44100.0f;
const std::size_t AUDIO_BUFFER_SIZE = 256;
const uint16_t WAVETABLE_SIZE = 1024;
const i2s_port_t i2sNum = I2S_NUM_0;

// change as required
const gpio_num_t I2S_DIN = GPIO_NUM_22;
const gpio_num_t I2S_LRCK = GPIO_NUM_25;
const gpio_num_t I2S_BCK = GPIO_NUM_26;

const i2s_config_t i2sConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = uint32_t(AUDIO_SR),
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = AUDIO_BUFFER_SIZE,
    .use_apll = 0,

};

const i2s_pin_config_t pinConfig = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_LRCK,
    .data_out_num = I2S_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE};

void i2sSetup()
{
    i2s_driver_install(i2sNum, &i2sConfig, 0, NULL);
    i2s_set_pin(i2sNum, &pinConfig);
}

void i2sAudioOut(const double *leftSamples, const double *rightSamples)
{
    int16_t intBuffer[AUDIO_BUFFER_SIZE * 2];
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        intBuffer[i * 2] = (leftSamples ? leftSamples[i] : 0.0) * 32000.0;
        intBuffer[(i * 2) + 1] = (rightSamples ? rightSamples[i] : 0.0) * 32000.0;
    }
    size_t bytesWritten;
    i2s_write(i2sNum, intBuffer, sizeof(intBuffer), &bytesWritten, portMAX_DELAY);
}

#endif
