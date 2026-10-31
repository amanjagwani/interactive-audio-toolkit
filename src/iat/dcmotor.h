#ifndef DC_MOTOR_H
#define DC_MOTOR_H

#include "output.h"
#include <driver/ledc.h>
#include "utils.h"

namespace iat
{

class DCMotor : public Output
{
    const int initFreq;
    const int resolution;
    int dutyCycle;
    float freq;
    gpio_num_t MOTOR_PWM_PIN;

  public:
    DCMotor(gpio_num_t pin, const char *id)
        : Output(120, 4, id), initFreq(5000), dutyCycle(0), resolution(8), freq(0), MOTOR_PWM_PIN(pin)
    {
        ledcAttach(MOTOR_PWM_PIN, initFreq, resolution);
    }

    void process(bool click)
    {
        if (!seqConfig)
        {
            return;
        }
        seqTrig = getSubDivTrig(click);
        bool gate = sequencer.updateStepAndGate(seqTrig, seqConfig.load()->getSequencerActive());

        if (gate)
        {
            float velocity = sequencer.getCurrentVelocity();
            dutyCycle = static_cast<int>(scale(velocity, 0.0f, 1.0f, 0, 254));
        }
        else
        {
            dutyCycle = 0;
            freq = 0;
        }
        ledcWrite(MOTOR_PWM_PIN, dutyCycle);
    }

    void stop()
    {
        ledcWrite(MOTOR_PWM_PIN, 0);
    }
};

}

#endif
