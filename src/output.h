#ifndef OUTPUT_H
#define OUTPUT_H

#include <ArduinoJson.h>
#include <sequencer.h>
#include <atomic>
#include "utils.h"
#include "sequencerConfig.h"

class Output_t
{
public:
    Sequencer sequencer;
    SequencerConfig_t *seqConfig;
    SequencerConfig_t *defConfig;
    uint16_t bpm;
    float noteDuration;
    bool seqTrig;
    unsigned long sequencerStartTime = 0;
    uint16_t counter;
    String configPath = "/config.json";

public:
    Output_t(uint16_t defaultBpm, uint8_t defaultSubDiv, SequencerConfig_t *defaultConfig) : bpm(defaultBpm),
                                                                                             sequencer(120),
                                                                                             counter(0), defConfig(defaultConfig),
                                                                                             seqConfig(defaultConfig)
    {
    }

    void setSeqConfig(SequencerConfig_t *config)
    {
        if (config != nullptr)
        {
            seqConfig = config;
            for (int i = 0; i < 32; i++)
            {
                sequencer.setPitch(i, seqConfig->pitches[i]);
                sequencer.setVelocity(i, seqConfig->velocities[i]);
            }
        }
    }

    void resetConfig()
    {
        setSeqConfig(defConfig);
    }

    void setMode(int mode)
    {
        sequencer.setMode(mode);
    }

    void setSteps(int numSteps)
    {
        sequencer.setSteps(numSteps);
    }

    void setPitch(int index, uint8_t value)
    {
        sequencer.setPitch(index, value);
    }

    void setVelocity(int index, float value)
    {
        sequencer.setVelocity(index, value);
    }

    SequencerConfig_t *getSeqConfig()
    {
        return seqConfig;
    }

    void setBpm(uint16_t globBpm)
    {
        bpm = globBpm;
        float bpmAtSubDiv;
        if (seqConfig && seqConfig->subDiv >= 1)
        {
            bpmAtSubDiv = bpm / seqConfig->subDiv;
        }
        else
        {
            bpmAtSubDiv = bpm;
        }
        sequencer.setBpm(bpmAtSubDiv);
        noteDuration = 60.0 / bpmAtSubDiv;
    }

    bool getSubDivTrig(bool click)
    {
        if (!seqConfig)
            return false;

        if (seqConfig->subDiv == 1)
        {
            seqTrig = click;
        }
        else if (click)
        {
            counter = (counter + 1) % 128;
            seqTrig = (click && counter % seqConfig->subDiv == 0);
        }
        else
        {
            seqTrig = false;
        }
        return seqTrig;
    }

    int getStep()
    {
        return sequencer.getStep();
    }

    virtual ~Output_t(){};
};

#endif