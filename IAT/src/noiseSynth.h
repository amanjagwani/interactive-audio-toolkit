#ifndef NOISESYNTH_H
#define NOISESYNTH_H

#include "Noise.h"
#include "Reson.h"
#include "RandMod.h"
#include "config.h"
#include "output.h"

class NoiseSynth : public Output
{
private:
    Noise noise;
    Reson reson;
    RandMod randMod;
    float frequency;
    float modDepth;

public:
    bool manualEnabled;
    bool triggerEnabled;
    NoiseSynth(const char *id)
        : Output(120, 2, id), randMod(), modDepth(200.0f), manualEnabled(false), triggerEnabled(false) {}

    void setNoiseAmp(float amp) { noise.setAmp(amp); }
    void setResonFreq(float freq) { frequency = freq; }
    void setBandwidth(float bw) { reson.setBandwidth(bw); }
    void setModDepth(float depth) { modDepth = depth; }
    void setUpdateRate(float rate) { randMod.setUpdateRate(rate); }
    float getResonFreq() { return reson.getFc(); };

    const float *process()
    {
        const float *noiseBuffer = noise.process();
        static float outputBuffer[AUDIO_BUFFER_SIZE];

        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        {
            float mod = randMod.process() * modDepth;
            reson.setFrequency(frequency + mod);
            outputBuffer[i] = reson.processSample(noiseBuffer[i]);
        }

        return outputBuffer;
    }
};

#endif
