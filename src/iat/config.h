#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include <cstdint>
#include <sdkconfig.h>

namespace iat
{

inline constexpr float AUDIO_SR = 48000.0f;
inline constexpr std::size_t AUDIO_BUFFER_SIZE = 256;
inline constexpr uint16_t WAVETABLE_SIZE = 1024;

#if CONFIG_IDF_TARGET_ESP32S3
#define IAT_I2S_DEFAULT_BCK 4
#define IAT_I2S_DEFAULT_LRCK 5
#define IAT_I2S_DEFAULT_DIN 6
#else
#define IAT_I2S_DEFAULT_BCK 26
#define IAT_I2S_DEFAULT_LRCK 25
#define IAT_I2S_DEFAULT_DIN 22
#endif

#ifndef IAT_I2S_BCK
#define IAT_I2S_BCK IAT_I2S_DEFAULT_BCK
#endif
#ifndef IAT_I2S_LRCK
#define IAT_I2S_LRCK IAT_I2S_DEFAULT_LRCK
#endif
#ifndef IAT_I2S_DIN
#define IAT_I2S_DIN IAT_I2S_DEFAULT_DIN
#endif

#ifndef IAT_I2S_BIT_DEPTH
#if CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32P4
#define IAT_I2S_BIT_DEPTH 32
#else
#define IAT_I2S_BIT_DEPTH 16
#endif
#endif
static_assert(IAT_I2S_BIT_DEPTH == 16 || IAT_I2S_BIT_DEPTH == 32, "IAT_I2S_BIT_DEPTH must be 16 or 32");

}

#endif
