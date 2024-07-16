#ifndef STEPPER_H
#define STEPPER_H

#include "output.h"
#include <AccelStepper.h>

class Stepper : public Output
{
    float freq;
    uint16_t defaultAccl;
    uint16_t maxSpeed;
    AccelStepper stepper;
    bool directionFlag;

    Stepper(gpio_num_t STEPPER_STEP_PIN, gpio_num_t STEPPER_DIR_PIN) : Output(120, 4), freq(440), defaultAccl(500), directionFlag(false),
                                                                       maxSpeed(5000), stepper(1, STEPPER_STEP_PIN, STEPPER_DIR_PIN)
    {
        stepper.setMaxSpeed(maxSpeed);
        stepper.setAcceleration(defaultAccl);
    }

    void process(bool click)
    {
        if (!seqConfig)
        {
            return;
        }
        seqTrig = getSubDivTrig(click);

        bool gate = sequencer.updateStepAndGate(seqTrig, seqConfig->getSequencerActive());

        if (gate)
        {
            directionFlag = !directionFlag;

            float newFreq = midiNoteToFrequency(sequencer.getCurrentPitch());
            // newFreq *= directionFlag ? 1 : -1;

            if (newFreq != freq)
            {
                freq = newFreq;
                stepper.setSpeed(freq);
            }
            stepper.runSpeed();
        }
    }
};

#endif