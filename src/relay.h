#ifndef RELAY_H
#define RELAY_H

#include "output.h"
#include "utils.h"

class Relay_t : public Output_t
{
    float freq;
    unsigned long duration;
    const gpio_num_t relayPin;
    float stepDivider;
    unsigned long lastSwitch;
    int relayState;

public:
    Relay_t(SequencerConfig_t *defaultConfig, const gpio_num_t pin) : Output_t(120, 4, defaultConfig), relayState(LOW), lastSwitch(0), relayPin(pin), stepDivider(1), freq(10), duration(1)
    {
        pinMode(relayPin, OUTPUT);
    }

    void process(bool click)
    {
        if (!seqConfig)
        {
            return;
        }
        seqTrig = getSubDivTrig(click);
        bool gate = sequencer.updateStepAndGate(seqTrig, seqConfig->sequencerActive);
        unsigned long currentTime = micros();

        if (gate && sequencer.getCurrentPitch() > 0)
        {
            stepDivider = scale(sequencer.getCurrentPitch(), 0, 127, 1, 128);
            if (stepDivider > 0)
            {
                duration = noteDuration * 1000000 / stepDivider;
            }
            if (currentTime - lastSwitch >= duration)
            {
                lastSwitch = currentTime;
                if (relayState == LOW)
                {
                    relayState = HIGH;
                }
                else
                {
                    relayState = LOW;
                }
            }
            digitalWrite(relayPin, relayState);
        }
    }
};

#endif