#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "stdlib.h"
#include <Arduino.h>

class Sequencer
{

public:
    enum class Mode
    {
        FORWARD,
        BACKWARD,
        RANDOM
    };

private:
    uint16_t pitches[32];
    float velocities[32];
    int8_t currentStep;
    int8_t numActiveSteps;
    unsigned long lastGateTime;
    unsigned long gateDuration;
    bool gateState;
    Mode sequencerMode;
    bool lastMetronomeState = false;

public:
    Sequencer(int bpm) : currentStep(0), lastGateTime(0), gateState(0), numActiveSteps(16), sequencerMode(Mode::FORWARD)
    {
        std::fill_n(pitches, 32, 0);
        std::fill_n(velocities, 32, 0.0f);
        setBpm(bpm);
    }

    void setBpm(float bpmSubDiv)
    {
        gateDuration = (60000.0f / bpmSubDiv) - 15;
    }

    void setMode(int mode)
    {
        if (mode >= 0 && mode <= 2)
        {
            sequencerMode = Mode(mode);
        }
    }

    void setPitch(int step, int value)
    {
        if (step >= 0 && step < 32)
        {
            pitches[step] = value;
        }
    }

    void setVelocity(int step, float value)
    {
        if (step >= 0 && step < 32)
        {
            velocities[step] = value;
        }
    }

    void setSteps(int numSteps)
    {
        if (numSteps <= 32 && numSteps >= 1)
        {
            numActiveSteps = numSteps;
        }
    }

    bool updateStepAndGate(bool metronomeTrigger, bool sequencerOn)
    {
        unsigned long currentTime = millis();

        if (metronomeTrigger && !lastMetronomeState && sequencerOn)
        {
            switch (sequencerMode)
            {
            case Mode::FORWARD:
                currentStep = (currentStep + 1) % numActiveSteps;
                break;

            case Mode::BACKWARD:
                currentStep = (currentStep - 1 + numActiveSteps) % numActiveSteps;
                break;

            case Mode::RANDOM:
                int8_t nextStep;
                do
                {
                    nextStep = random(numActiveSteps);
                } while (nextStep == currentStep);
                currentStep = nextStep;
                break;
            }
            lastGateTime = currentTime;
            gateState = true;
        }

        if (currentTime - lastGateTime >= gateDuration)
        {
            gateState = false;
        }

        lastMetronomeState = metronomeTrigger;
        return gateState;
    }

    uint16_t getCurrentPitch()
    {
        return pitches[currentStep];
    }

    float getCurrentVelocity()
    {
        return velocities[currentStep];
    }

    uint16_t getPitchAtIndex(int index)
    {
        if (index >= 0 && index < 32)
        {
            return pitches[index];
        }
        return 0;
    }

    float getVelocityAtIndex(int index)
    {
        if (index >= 0 && index < 32)
        {
            return velocities[index];
        }
        return 0;
    }

    int getStep()
    {
        return currentStep;
    }
};

#endif