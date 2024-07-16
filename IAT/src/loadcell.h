#ifndef LOADCELL_T_H
#define LOADCELL_T_H

#include <Arduino.h>
#include "Q2HX711.h"
#include "input.h"

class LoadCell : public Input
{
private:
    Q2HX711 hx711;
    uint32_t prevReading;
    uint32_t tareValue;

public:
    LoadCell(gpio_num_t doutPin, gpio_num_t sckPin, const char *idNum, unsigned long updatePeriod)
        : Input(idNum, updatePeriod), tareValue(0), hx711(doutPin, sckPin) {}

    void tare()
    {
        tareValue = prevReading;
        Serial.println(tareValue);
    }

    void clearTare()
    {
        tareValue = 0;
    }

    void read() override
    {
        if (isEnabled)
        {
            unsigned long currentTime = millis();
            if (currentTime - lastUpdate >= updateInterval)
            {
                lastUpdate = currentTime;
                prevReading = hx711.read() - tareValue;

                reading = prevReading / 10000;
                reading = clamp(reading, min, max);

#ifdef SENSOR_VAL_PRINT
                Serial.println("LoadCell" + String(id) + ": " + String(reading));
#endif
            }
        }
    }
};

#endif // LOADCELL_T_H
