#ifndef RANDMOD_H
#define RANDMOD_H

#include <Arduino.h>
#include <cstdlib>
#include "config.h"

class RandMod
{
private:
    float sampleRate;
    float updateRate;
    float currentValue;
    float targetValue;
    float stepSize;
    int counter;

public:
    RandMod()
        : sampleRate(AUDIO_SR), updateRate(2.0f), currentValue(0.0f), targetValue(0.0f), stepSize(0.0f), counter(0)
    {
        generateNewTarget();
    }

    void setUpdateRate(float ur)
    {
        updateRate = ur;
        calculateStepSize();
    }

    float process()
    {
        if (counter++ >= sampleRate / updateRate)
        {
            counter = 0;
            generateNewTarget();
        }
        currentValue += stepSize;
        return currentValue;
    }

private:
    void generateNewTarget()
    {
        targetValue = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        stepSize = (targetValue - currentValue) / (sampleRate / updateRate);
    }

    void calculateStepSize()
    {
        stepSize = (targetValue - currentValue) / (sampleRate / updateRate);
    }
};

#endif
