#ifndef OUTPUT_H
#define OUTPUT_H

#include <ArduinoJson.h>
#include <sequencer.h>
#include <atomic>
#include "utils.h"
#include "sequencerConfig.h"

class Output
{
protected:
    Sequencer sequencer;
    SequencerConfig *seqConfig;
    uint16_t bpm;
    float noteDuration;
    bool seqTrig;
    unsigned long sequencerStartTime = 0;
    uint16_t counter;
    String configPath = "/config.json";

public:
    Output(uint16_t defaultBpm, uint8_t defaultSubDiv) : bpm(defaultBpm),
                                                         sequencer(defaultBpm),
                                                         counter(0),
                                                         seqConfig(nullptr)
    {
    }

    void setSeqConfig(SequencerConfig *config)
    {
        if (config != nullptr)
        {
            seqConfig = config;
            for (int i = 0; i < 32; i++)
            {
                sequencer.setPitch(i, seqConfig->getPitch(i));
                sequencer.setVelocity(i, seqConfig->getVelocity(i));
            }

            sequencer.setMode(seqConfig->getMode());
            sequencer.setSteps(seqConfig->getNumSteps());
        }
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

    SequencerConfig *getSeqConfig()
    {
        return seqConfig;
    }

    void setBpm(uint16_t globBpm)
    {
        bpm = globBpm;
        float bpmAtSubDiv;
        if (seqConfig && seqConfig->getSubDiv() >= 1)
        {
            bpmAtSubDiv = bpm / seqConfig->getSubDiv();
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

        if (seqConfig->getSubDiv() == 1)
        {
            seqTrig = click;
        }
        else if (click)
        {
            counter = (counter + 1) % 128;
            seqTrig = (click && counter % seqConfig->getSubDiv() == 0);
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

    virtual ~Output(){};
};

#endif