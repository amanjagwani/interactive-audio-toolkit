#ifndef LIDAR_LONG_T_H
#define LIDAR_LONG_T_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "TFMini.h"
#include "sensor.h"

class LidarLong : public Sensor
{
private:
    HardwareSerial lidarSerial;
    TFMini tfmini;
    uint16_t strength;
    gpio_num_t RX_PIN;
    gpio_num_t TX_PIN;

public:
    LidarLong(gpio_num_t RX, gpio_num_t TX, unsigned long updatePeriod, const char *idNum)
        : Sensor(updatePeriod, idNum, false), RX_PIN(RX), TX_PIN(TX), lidarSerial(2) {}

    void begin()
    {
        lidarSerial.begin(TFMINI_BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
        tfmini.begin(&lidarSerial);
        Serial.println("TFMINI LIDAR Initialized");
    }

    void read() override
    {
        if (isEnabled)
        {
            unsigned long currentTime = millis();
            if (currentTime - lastUpdate >= updateInterval)
            {
                if (lidarSerial.available())
                {
                    lastUpdate = currentTime;
                    reading = tfmini.getDistance();
                    reading = clamp(reading, min, max);

#ifdef SENSOR_VAL_PRINT
                    Serial.print("Lidar Long Range Reading: ");
                    Serial.println(reading);
#endif
                }
            }
        }
    }

    uint16_t getStrength()
    {
        strength = tfmini.getRecentSignalStrength();
#ifdef SENSOR_VAL_PRINT
        Serial.print("Lidar Long Range Strength: ");
        Serial.println(strength);
#endif
        return strength;
    }
};

#endif
