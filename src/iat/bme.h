#ifndef BME_SENSOR_H
#define BME_SENSOR_H

#include "bme68xLibrary.h"
#include "Wire.h"

namespace iat
{

struct BME68xSensor
{
    Bme68x bme;
    const uint8_t I2C_ADDRESS = 0x76;
    gpio_num_t I2C_SDA, I2C_SCL;
    float temperature = 0.0;
    float pressure = 0.0;
    float humidity = 0.0;
    float gasResistance = 0.0;
    bool isConnected = false;
    unsigned long updateInterval;
    unsigned long lastUpdate;

    BME68xSensor(gpio_num_t sdaPin, gpio_num_t sclPin, unsigned long interval = 600)
        : I2C_SDA(sdaPin), I2C_SCL(sclPin), updateInterval(interval * 1000), lastUpdate(0) {}

    int begin()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        bme.begin(I2C_ADDRESS, Wire);
        if (bme.checkStatus() == BME68X_ERROR)
        {
            Serial.println("BME68x sensor error: " + bme.statusString());
            isConnected = false;
            return 1;
        }
        bme.setTPH();
        bme.setHeaterProf(300, 100);
        isConnected = true;
        read();
        return 0;
    }

    void read()
    {
        if (isConnected)
        {
            bme68xData data;
            bme.setOpMode(BME68X_FORCED_MODE);
            delayMicroseconds(bme.getMeasDur());
            if (bme.fetchData())
            {
                bme.getData(data);
                temperature = data.temperature;
                pressure = data.pressure;
                humidity = data.humidity;
                gasResistance = data.gas_resistance;
            }
        }
    }

    bool update()
    {
        unsigned long currentTime = millis();
        if (isConnected && currentTime - lastUpdate >= updateInterval)
        {
            read();
            lastUpdate = currentTime;
            return true;
        }
        return false;
    }
};

}

#endif
