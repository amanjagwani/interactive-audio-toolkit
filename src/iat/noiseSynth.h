#ifndef NOISESYNTH_H
#define NOISESYNTH_H

#include "noise.h"
#include "reson.h"
#include "randMod.h"
#include "config.h"
#include "output.h"
#include <atomic>

namespace iat
{

class NoiseSynth : public Output
{
  private:
    Noise noise;
    Reson reson;
    RandMod randMod;
    std::atomic<float> frequency;
    std::atomic<float> modDepth;
    float outputBuffer[AUDIO_BUFFER_SIZE];

  public:
    std::atomic<bool> manualEnabled;
    std::atomic<bool> triggerEnabled;
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
        const float baseFreq = frequency;
        const float depth = modDepth;

        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        {
            float mod = randMod.process() * depth;
            reson.setFrequency(baseFreq + mod);
            outputBuffer[i] = reson.processSample(noiseBuffer[i]);
        }

        return outputBuffer;
    }
};

}

#endif
