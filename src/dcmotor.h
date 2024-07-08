#ifndef DC_MOTOR_H
#define DC_MOTOR_H

#include "output.h"
#include <driver/ledc.h>
#include "utils.h"

class DCMotor_t : public Output_t
{
    const int channel;
    const int initFreq;
    const int resolution;
    int dutyCycle;
    float freq;
    gpio_num_t MOTOR_PWM_PIN;

public:
    DCMotor_t(SequencerConfig_t *defaultConfig, gpio_num_t pin)
        : Output_t(120, 4, defaultConfig), initFreq(5000), channel(0), dutyCycle(0), resolution(8), freq(0), MOTOR_PWM_PIN(pin)
    {
        ledcSetup(channel, initFreq, resolution);
        ledcAttachPin(MOTOR_PWM_PIN, channel);
    }

    void process(bool click)
    {
        if (!seqConfig)
        {
            return;
        }
        seqTrig = getSubDivTrig(click);
        bool gate = sequencer.updateStepAndGate(seqTrig, seqConfig->sequencerActive);

        if (gate)
        {
            int pitch = sequencer.getCurrentPitch();
            if (pitch > 0)
            {
                digitalWrite(MOTOR_PWM_PIN, HIGH);
            }

            float velocity = sequencer.getCurrentVelocity();
            dutyCycle = static_cast<int>(scale(velocity, 0.0f, 1.0f, 0, 254));
        }
        else
        {
            dutyCycle = 0;
            freq = 0;
            digitalWrite(MOTOR_PWM_PIN, LOW);
        }
        ledcWrite(channel, dutyCycle);
    }

    void stop()
    {
        ledcWrite(channel, 0);
        ledcWriteTone(channel, 0);
        digitalWrite(MOTOR_PWM_PIN, LOW);
    }
};

#endif