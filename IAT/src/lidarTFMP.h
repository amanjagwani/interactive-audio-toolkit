#ifndef LIDAR_TFMP_H
#define LIDAR_TFMP_H

#include "TFMPI2C.h"
#include "config.h"
#include "Wire.h"
#include "sensor.h"

struct LidarTFMP : public Sensor
{

    TFMPI2C tfmp;
    const String configPath = "/miscConfig.json";
    bool isConnected;
    gpio_num_t I2C_SDA, I2C_SCL;

    LidarTFMP(gpio_num_t sdaPin, gpio_num_t sclPin, unsigned long updatePeriod, const string id) : Sensor(updatePeriod, id, false) {};

    int begin()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        Wire.setClock(400000);

        printf("TFMP System reset: ");
        if (tfmp.sendCommand(SOFT_RESET, 0))
        {
            printf("passed.\r\n");
        }
        else
        {
            tfmp.printReply();
            return 1;
        }

        printf("Data-Frame rate: ");
        if (tfmp.sendCommand(SET_FRAME_RATE, FRAME_20))
        {
            printf("%2uHz.\n", FRAME_20);
        }
        else
        {
            tfmp.printReply();
            return 1;
        }
        return 0;
    }

    void read()
    {
        if (isEnabled)
        {
            if (tfmp.status == TFMP_READY)
            {
                int16_t data;
                tfmp.getData(data);
                reading = (uint16_t)data;
#ifdef SENSOR_VAL_PRINT
                Serial.print("TFMP Distance(cm): ");
                Serial.println(reading);
#endif
            }
            else
            {
#ifdef SENSOR_VAL_PRINT
                tfmp.printFrame();
                if (tfmp.status == TFMP_I2CWRITE)
                {
                    tfmp.recoverI2CBus(I2C_SDA, I2C_SCL);
                }
#endif
            }
        }
    }
};

#endif