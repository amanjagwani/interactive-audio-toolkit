#ifndef LOADCELL_H
#define LOADCELL_H

#include "config.h"
#include "HX711_ADC.h"
#include "sensor.h"

struct LoadCell : public Sensor
{
    HX711_ADC hx711;
    int32_t prevReading;
    int32_t tareValue;
    unsigned long lastUpdate;
    int id; // use 1 or 2

    const String configPath = "/miscConfig.json";

    LoadCell(gpio_num_t doutPin, gpio_num_t sckPin, int idNum,
             unsigned long updatePeriod)
        : Sensor(updatePeriod, String("loadcell" + String(idNum)), true),
          lastUpdate(0),
          id(idNum),
          tareValue(0),
          hx711(doutPin, sckPin) {}

    void begin()
    {
        hx711.begin();
        float calibrationValue = 2190;
        unsigned long stabilizingTime = 2000;
        bool _tare = true;

        hx711.start(stabilizingTime, _tare);
        if (hx711.getTareTimeoutFlag())
        {
            Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
        }
        else
        {
            hx711.setCalFactor(calibrationValue);
            Serial.println("Load cell startup complete");
        }
        hx711.setReverseOutput();
    }

    void tare(int samples = 10)
    {
        int32_t sum = 0;
        for (int i = 0; i < samples; ++i)
        {
            sum += hx711.getData();
            delay(10);
        }
        tareValue = sum / samples;
        Serial.println("Tare value set to: " + String(tareValue));
    }

    void read()
    {
        if (isEnabled)
        {
            static bool newDataReady = false;

            if (hx711.update())
                newDataReady = true;

            unsigned long currentTime = millis();
            if (currentTime - lastUpdate >= updateInterval)
            {
                lastUpdate = currentTime;

                if (newDataReady)
                {
                    int32_t rawData = hx711.getData();
                    int32_t adjustedReading = rawData - tareValue;
                    reading = adjustedReading > 0 ? adjustedReading : 0;
#ifdef SENSOR_VAL_PRINT
                    Serial.print("Load_cell raw val: ");
                    Serial.print(rawData);
                    Serial.print(", Tare val: ");
                    Serial.print(tareValue);
                    Serial.print(", Adjusted val: ");
                    Serial.println(reading);
#endif
                    newDataReady = false;
                }
            }
        }
    }

    void clearTare()
    {
        tareValue = 0;
        Serial.println("Tare value cleared.");
    }
};

#endif
