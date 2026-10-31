#ifndef OUTPUT_H
#define OUTPUT_H

#include <ArduinoJson.h>
#include "sequencer.h"
#include <atomic>
#include "utils.h"
#include "sequencerConfig.h"

namespace iat
{

class Output
{
  protected:
    Sequencer sequencer;
    std::atomic<uint16_t> bpm;
    std::atomic<float> noteDuration;
    bool seqTrig;
    unsigned long sequencerStartTime = 0;
    uint16_t counter;
    String configPath = "/config.json";
    const char *id;

  public:
    std::atomic<SequencerConfig *> seqConfig;
    Output(uint16_t defaultBpm, uint8_t defaultSubDiv, const char *idNum) : bpm(defaultBpm),
                                                                            sequencer(defaultBpm),
                                                                            counter(0),
                                                                            seqConfig(nullptr), id(idNum)
    {
    }

    const char *getId() const { return id; }

    void setSeqConfig(SequencerConfig *config)
    {
        if (config != nullptr)
        {
            for (int i = 0; i < 32; i++)
            {
                sequencer.setPitch(i, config->getPitch(i));
                sequencer.setVelocity(i, config->getVelocity(i));
            }

            sequencer.setMode(config->getMode());
            sequencer.setSteps(config->getNumSteps());
            seqConfig = config;
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
        SequencerConfig *config = seqConfig;
        if (config && config->getSubDiv() >= 1)
        {
            bpmAtSubDiv = bpm / config->getSubDiv();
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
        SequencerConfig *config = seqConfig;
        if (!config)
            return false;

        if (config->getSubDiv() == 1)
        {
            seqTrig = click;
        }
        else if (click)
        {
            counter = (counter + 1) % 128;
            seqTrig = (click && counter % config->getSubDiv() == 0);
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

    virtual ~Output() {};
};

}

#endif
