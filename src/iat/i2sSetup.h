#ifndef I2S_SETUP_H
#define I2S_SETUP_H

#include <driver/i2s_std.h>
#include "config.h"

namespace iat
{

inline constexpr gpio_num_t I2S_DIN = static_cast<gpio_num_t>(IAT_I2S_DIN);
inline constexpr gpio_num_t I2S_LRCK = static_cast<gpio_num_t>(IAT_I2S_LRCK);
inline constexpr gpio_num_t I2S_BCK = static_cast<gpio_num_t>(IAT_I2S_BCK);

#if IAT_I2S_BIT_DEPTH == 32
using I2sSample = int32_t;
inline constexpr i2s_data_bit_width_t I2S_BIT_WIDTH = I2S_DATA_BIT_WIDTH_32BIT;
#else
using I2sSample = int16_t;
inline constexpr i2s_data_bit_width_t I2S_BIT_WIDTH = I2S_DATA_BIT_WIDTH_16BIT;
#endif
inline constexpr double I2S_SCALE = 32000.0 / 32768.0 * (1LL << (IAT_I2S_BIT_DEPTH - 1));

inline i2s_chan_handle_t i2sTx = nullptr;

inline void i2sSetup()
{
    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chanCfg.dma_desc_num = 4;
    chanCfg.dma_frame_num = AUDIO_BUFFER_SIZE;
    ESP_ERROR_CHECK(i2s_new_channel(&chanCfg, &i2sTx, nullptr));

    i2s_std_config_t stdCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(static_cast<uint32_t>(AUDIO_SR)),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_BIT_WIDTH, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK,
            .ws = I2S_LRCK,
            .dout = I2S_DIN,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {},
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(i2sTx, &stdCfg));
    ESP_ERROR_CHECK(i2s_channel_enable(i2sTx));
}

inline void i2sAudioOut(const double *leftSamples, const double *rightSamples)
{
    I2sSample intBuffer[AUDIO_BUFFER_SIZE * 2];
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        intBuffer[i * 2] = (leftSamples ? leftSamples[i] : 0.0) * I2S_SCALE;
        intBuffer[(i * 2) + 1] = (rightSamples ? rightSamples[i] : 0.0) * I2S_SCALE;
    }
    //  Serial.println(intBuffer[1]);
    size_t bytesWritten;
    i2s_channel_write(i2sTx, intBuffer, sizeof(intBuffer), &bytesWritten, portMAX_DELAY);
}

}

#endif
