#ifndef NOISE_H
#define NOISE_H

#include <Arduino.h>
#include "config.h"

class Noise
{
private:
    static constexpr float coeff = 4.6566129e-010f;
    float amp;
    int32_t randSeed;
    float out[AUDIO_BUFFER_SIZE];

public:
    Noise()
    {
        amp = 0.1f;
        randSeed = 1;
    };

    ~Noise() {};

    inline void setAmp(float a) { amp = a; }

    const float *process()
    {
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        {
            randSeed *= 16807;
            out[i] = (randSeed * coeff) * amp;
        }
        return out;
    }
};

#endif