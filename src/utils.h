#ifndef UTILS_H
#define UTILS_H

#include "math.h"
#include <ArduinoJson.h>
#include <SD.h>

inline float midiNoteToFrequency(int midiNote)
{
    return pow(2.0, (midiNote - 69) / 12.0) * 440.0;
}

inline float scale(float value, float inMin, float inMax, float outMin, float outMax)
{
    value = value < inMin ? inMin : value;
    value = value > inMax ? inMax : value;
    return (value - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
}

#endif
