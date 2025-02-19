#ifndef RESON_H
#define RESON_H

#include <Arduino.h>
#include <cmath>
#include "config.h"

class Reson
{
private:
    float fs;
    float fc;
    float bandwidth;
    float b1, b2, a;
    float y[2];

public:
    Reson()
        : fs(AUDIO_SR), fc(2000.0f), bandwidth(300.0f)
    {
        y[0] = y[1] = 0.0f;
        calculateCoefficients();
    }

    float getFc() { return fc; };

    void setFrequency(float centerFreq)
    {
        fc = centerFreq;
        calculateCoefficients();
    }
    void setBandwidth(float bw)
    {
        bandwidth = bw;
        calculateCoefficients();
    }

    void calculateCoefficients()
    {
        float R = 1 - (bandwidth * (PI / fs));
        float cosTheta = cos(2.0f * PI * fc / fs);
        b1 = -((4.0f * R * R) / (1 + (R * R))) * cosTheta;
        b2 = R * R;
        a = (1 - (R * R)) * sin(acos(b1 / 2 * R));
    }

    float processSample(const float x)
    {
        float y0 = a * x - b1 * y[0] - b2 * y[1];
        y[1] = y[0];
        y[0] = y0;
        return y0;
    }

    void reset()
    {
        y[0] = y[1] = 0.0f;
    }
};

#endif
