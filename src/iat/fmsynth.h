#ifndef FMSYNTH_H
#define FMSYNTH_H

#include <atomic>
#include "oscillator.h"
#include "adsr.h"
#include "metro.h"
#include "sequencer.h"
#include "output.h"
#include <ArduinoJson.h>
#include "utils.h"

namespace iat
{

class FmSynth : public Output
{
    Oscillator carrier;
    Oscillator modulator;
    ADSR carAdsr;
    ADSR modAdsr;
    float freq;
    std::atomic<float> modIndex;
    std::atomic<float> modRatio;
    std::atomic<float> level;
    float velocity;

  public:
    FmSynth(const char *idNum)
        : Output(120, 4, idNum), carrier(0.1, 440),
          modulator(0.1, 440),
          carAdsr(1),
          modAdsr(1),
          freq(220.0f),
          modIndex(0),
          modRatio(1),
          level(0.5f),
          velocity(0.5f)
    {
        carrier.initWaveTable();
        carAdsr.setParams(0.2 * noteDuration, 0.2 * noteDuration, 0.7, 0.2 * noteDuration);
        modAdsr.setParams(0.3, 0.5, 0.7, 0.1);
    }

    const double *process(bool click)
    {
        if (!seqConfig)
        {
            return nullptr;
        }
        seqTrig = getSubDivTrig(click);

        bool gate = sequencer.updateStepAndGate(seqTrig, seqConfig.load()->getSequencerActive());
        freq = midiNoteToFrequency(sequencer.getCurrentPitch()) + 0.5;
        if (freq < 4.0)
        {
            freq = 4.0;
        }
        velocity = sequencer.getCurrentVelocity();
        float modVal = modIndex * 2 * M_PI;

        return carrier.process(carAdsr.process(gate, velocity), level, freq,
                               modulator.process(modAdsr.process(gate, 1), modVal, freq * modRatio));
    }

    void setModIndex(float index)
    {
        if (index > 0 && index < 40)
        {
            modIndex = index;
        }
    }

    void setModRatio(float ratio)
    {
        if (ratio > 0)
        {
            modRatio = ratio;
        }
    }

    void setLevel(float value) { level = value; }
    float getModIndex() const { return modIndex; }
    float getModRatio() const { return modRatio; }
    float getLevel() const { return level; }
    void getCarEnv(float params[4]) { carAdsr.getParams(params); }
    void getModEnv(float params[4]) { modAdsr.getParams(params); }

    void setCarEnv(float atk, float dec, float sus, float rel)
    {
        carAdsr.setParams(atk, dec, sus, rel);
    }

    void setModEnv(float atk, float dec, float sus, float rel)
    {
        modAdsr.setParams(atk, dec, sus, rel);
    }

    const char *getType() const override { return "fmSynth"; }
};

}

#endif
