#ifndef LIDAR_H
#define LIDAR_H

#include "SparkFun_VL53L1X.h"
#include "config.h"
#include "Wire.h"
#include "sensor.h"

struct Lidar : public Sensor
{

    enum class Mode
    {
        SHORT,
        LONG
    };

    SFEVL53L1X distanceSensor;
    Mode mode;
    const String configPath = "/miscConfig.json";
    bool isConnected;
    gpio_num_t I2C_SDA, I2C_SCL;

    Lidar(gpio_num_t sdaPin, gpio_num_t sclPin, unsigned long updatePeriod, const String id) : Sensor(updatePeriod, id, false), mode(Mode::LONG) {};

    int begin()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        if (distanceSensor.begin() != 0)
        {
            Serial.println("LIDAR Sensor failed to begin.");
            isConnected = false;
            return 1;
        }
        mode == Mode(distanceSensor.getDistanceMode());
        Serial.println("LIDAR Sensor Initialized!");
        Serial.println(String("LIDAR mode: ") + (mode == Mode::LONG ? "long" : "short"));
        distanceSensor.setIntermeasurementPeriod(updateInterval);
        distanceSensor.startRanging();
        isConnected = true;
        return 0;
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

    void read()
    {
        if (isEnabled)
        {
            if (distanceSensor.checkForDataReady())
            {
                reading = distanceSensor.getDistance();
                distanceSensor.clearInterrupt();
#ifdef SENSOR_VAL_PRINT
                Serial.print("Distance(mm): ");
                Serial.println(reading);
#endif
            }
        }
    }

    template <typename SendFunction>
    void readAndSend(SendFunction sendFunction)
    {
        if (isEnabled)
        {
            if (distanceSensor.checkForDataReady())
            {
                reading = distanceSensor.getDistance();
                distanceSensor.clearInterrupt();
#ifdef SENSOR_VAL_PRINT
                Serial.print("LIDAR Distance(mm): ");
                Serial.println(reading);

#endif

                sendFunction("lidar", reading);
            }
        }
    }
};

#endif