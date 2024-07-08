#ifndef SONAR_H
#define SONAR_H

#include "HardwareSerial.h"
#include "config.h"
#ifndef SONAR_T_H
#define SONAR_T_H

#include "HardwareSerial.h"
#include "Input_t.h"
#include <ArduinoJson.h>
#include <FS.h>
#include "LittleFS.h"
#include "SD.h"

class Sonar_t : public Input_t
{
private:
    HardwareSerial sonarSerial;
    gpio_num_t RX_PIN;
    gpio_num_t TX_PIN;

public:
    Sonar_t(gpio_num_t RX, gpio_num_t TX, unsigned long updatePeriod, const char *idNum)
        : Input_t(idNum, updatePeriod), RX_PIN(RX), TX_PIN(TX), sonarSerial(2) {}

    void begin()
    {
        sonarSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
        Serial.println("MB1604 SONAR Initialized");
    }

    void read() override
    {
        if (isEnabled)
        {
            unsigned long currentTime = millis();
            if (currentTime - lastUpdate >= updateInterval)
            {
                if (sonarSerial.available())
                {
                    String sreading = sonarSerial.readStringUntil('\r');
                    if (sreading.length() && sreading[0] == 'R')
                    {
                        sreading = sreading.substring(1);
                        reading = sreading.toInt();
                        reading = clamp(reading, min, max);

#ifdef SENSOR_VAL_PRINT
                        Serial.print("Sonar Reading: ");
                        Serial.println(reading);
#endif
                    }
                }
            }
        }
    }
};

#endif // SONAR_T_H

#endif