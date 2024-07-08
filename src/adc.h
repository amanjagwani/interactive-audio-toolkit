#ifndef LDR_H
#define LDR_H

#include "Arduino.h"
#include "input.h"

class Adc_t : public Input_t
{
    const gpio_num_t pin;

public:
    Adc_t(const gpio_num_t pinNum, const char *idNum, unsigned long updatePeriod) : Input_t(idNum, updatePeriod), pin(pinNum)
    {
        pinMode(pinNum, INPUT);
    };

    void read() override
    {
        if (isEnabled)
        {
            unsigned long currentTime = millis();
            if (currentTime - lastUpdate >= updateInterval)
            {
                lastUpdate = currentTime;
                reading = analogRead(pin);
                reading = clamp(reading, min, max);

#ifdef SENSOR_VAL_PRINT
                Serial.println("Adc" + String(id) + ": " + String(reading));
#endif
            }
        }
    }

    void readPWM()
    {
        if (isEnabled)
        {
            unsigned long currentTime = millis();
            if (currentTime - lastUpdate >= updateInterval)
            {
                lastUpdate = currentTime;
                unsigned long pulseDuration = pulseIn(pin, HIGH);

                if (pulseDuration > 0)
                {
                    reading = pulseDuration;
                    reading = clamp(reading, min, max);

#ifdef SENSOR_VAL_PRINT
                    Serial.println("Adc" + String(id) + ": " + String(reading));
#endif
                }
            }
        }
    }
};
#endif
