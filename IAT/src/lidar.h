#ifndef LIDAR_T_H
#define LIDAR_T_H

#include "SparkFun_VL53L1X.h"
#include "Wire.h"
#include "input.h"

class Lidar : public Input
{
public:
    enum class Mode
    {
        SHORT,
        LONG
    };

private:
    SFEVL53L1X distanceSensor;
    Mode mode;
    const gpio_num_t I2C_SDA;
    const gpio_num_t I2C_SCL;

public:
    Lidar(gpio_num_t SDAPin, gpio_num_t SCLPin, unsigned long updatePeriod, const char *idNum)
        : Input(idNum, updatePeriod), mode(Mode::LONG), I2C_SDA(SDAPin), I2C_SCL(SCLPin) {}

    void begin()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        if (distanceSensor.begin() != 0)
        {
            Serial.println("LIDAR Sensor failed to begin.");
        }
        else
        {
            mode = static_cast<Mode>(distanceSensor.getDistanceMode());
            Serial.println("LIDAR Sensor Initialized!");
            Serial.println(String("LIDAR mode: ") + (mode == Mode::LONG ? "long" : "short"));
            distanceSensor.setIntermeasurementPeriod(updateInterval);
            distanceSensor.startRanging();
        }
    }

    void setMode(Mode newMode)
    {
        mode = newMode;
        if (mode == Mode::SHORT)
        {
            distanceSensor.setDistanceModeShort();
            Serial.println("LIDAR set to short mode");
        }
        else if (mode == Mode::LONG)
        {
            distanceSensor.setDistanceModeLong();
            Serial.println("LIDAR set to long mode");
        }
    }

    void read() override
    {
        if (isEnabled)
        {
            if (distanceSensor.checkForDataReady())
            {
                reading = distanceSensor.getDistance();
                distanceSensor.clearInterrupt();
                reading = clamp(reading, min, max);
#ifdef SENSOR_VAL_PRINT
                Serial.print("Distance(mm): ");
                Serial.println(reading);
#endif
            }
        }
    }
};

#endif 
