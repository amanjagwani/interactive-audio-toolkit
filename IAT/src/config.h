#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include <cstdint>

inline constexpr float AUDIO_SR = 44100.0f;
inline constexpr std::size_t AUDIO_BUFFER_SIZE = 256;
inline constexpr uint16_t WAVETABLE_SIZE = 1024;

#endif
