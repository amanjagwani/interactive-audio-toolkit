#ifndef FMSYNTH_H
#define FMSYNTH_H

#include <atomic>
#include "oscillator.h"
#include "adsr.h"
#include "metro.h"
#include "sequencer.h"
#include "output.h"
#include <FS.h>
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

    void setCarEnv(float atk, float dec, float sus, float rel)
    {
        carAdsr.setParams(atk, dec, sus, rel);
    }

    void setModEnv(float atk, float dec, float sus, float rel)
    {
        modAdsr.setParams(atk, dec, sus, rel);
    }

    void saveConfig(fs::FS &fs)
    {
        JsonDocument doc;
        auto configSection = doc["output"][id];
        configSection["modIndex"] = modIndex.load();
        configSection["level"] = level.load();
        configSection["modRatio"] = modRatio.load();
        configSection["ampAttack"] = carAdsr.m_attack.load();
        configSection["ampDecay"] = carAdsr.m_decay.load();
        configSection["ampSustain"] = carAdsr.m_sustainLevel.load();
        configSection["ampRelease"] = carAdsr.m_release.load();
        configSection["modAttack"] = modAdsr.m_attack.load();
        configSection["modDecay"] = modAdsr.m_decay.load();
        configSection["modSustain"] = modAdsr.m_sustainLevel.load();
        configSection["modRelease"] = modAdsr.m_release.load();

        File configFile = fs.open("/outputConfig.json", FILE_WRITE);
        if (!configFile)
        {
            Serial.println("Failed to open output config file for writing");
            return;
        }

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write output config file");
        }

        configFile.close();
    }

    void loadConfig(fs::FS &fs)
    {
        File configFile = fs.open("/outputConfig.json", FILE_READ);
        if (!configFile)
        {
            Serial.println("Failed to open config file for reading: FM Synth");
            return;
        }
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration: FM Synth");
        }
        else
        {
            auto configSection = doc["output"][id];
            modIndex = configSection["modIndex"] | modIndex.load();
            modRatio = configSection["modRatio"] | modRatio.load();
            level = configSection["level"] | level.load();
            carAdsr.m_attack = configSection["ampAttack"] | carAdsr.m_attack.load();
            carAdsr.m_decay = configSection["ampDecay"] | carAdsr.m_decay.load();
            carAdsr.m_sustainLevel = configSection["ampSustain"] | carAdsr.m_sustainLevel.load();
            carAdsr.m_release = configSection["ampRelease"] | carAdsr.m_release.load();
            modAdsr.m_attack = configSection["modAttack"] | modAdsr.m_attack.load();
            modAdsr.m_decay = configSection["modDecay"] | modAdsr.m_decay.load();
            modAdsr.m_sustainLevel = configSection["modSustain"] | modAdsr.m_sustainLevel.load();
            modAdsr.m_release = configSection["modRelease"] | modAdsr.m_release.load();

            carAdsr.setParams(carAdsr.m_attack, carAdsr.m_decay, carAdsr.m_sustainLevel, carAdsr.m_release);
            modAdsr.setParams(modAdsr.m_attack, modAdsr.m_decay, modAdsr.m_sustainLevel, modAdsr.m_release);
        }
        configFile.close();
    }
};

}

#endif
