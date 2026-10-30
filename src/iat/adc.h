#ifndef LDR_H
#define LDR_H
#include "sensor.h"

#include "Arduino.h"

struct Adc : public Sensor
{
    const gpio_num_t pin;
    unsigned long lastUpdate;
    const String configPath = "/miscConfig.json";

    Adc(const gpio_num_t pinNum, const char *idNum, unsigned long updatePeriod) : Sensor(updatePeriod, idNum, false), pin(pinNum), lastUpdate(0)
    {
        pinMode(pinNum, INPUT);
    };

    void read()
    {
        if (isEnabled)
        {
            unsigned long currentTime = millis();
            if (currentTime - lastUpdate >= updateInterval)
            {
                lastUpdate = currentTime;
                reading = analogRead(pin);
#ifdef SENSOR_VAL_PRINT
                Serial.println("Adc" + String(sensorId) + ": " + String(reading));
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
#ifdef SENSOR_VAL_PRINT
                    Serial.println("Adc" + String(sensorId) + ": " + String(reading));
#endif
                }
            }
        }
    }
};
#endif
