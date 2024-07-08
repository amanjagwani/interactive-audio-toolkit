#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "LittleFS.h"

class Input_t
{
protected:
    const char *id;
    uint16_t reading;
    unsigned long lastUpdate;
    unsigned long updateInterval;
    bool isEnabled;
    int min, max;

    uint16_t clamp(uint16_t value, int minValue, int maxValue)
    {
        if (value < minValue)
            return minValue;
        if (value > maxValue)
            return maxValue;
        return value;
    }

public:
    Input_t(const char *idNum, unsigned long updatePeriod) : id(idNum), reading(0), lastUpdate(0), updateInterval(updatePeriod), isEnabled(false), min(0), max(10000) {}

    virtual void read() = 0;

    const char *getId() const { return id; }
    uint16_t getReading() const { return reading; }
    bool getIsEnabled() const { return isEnabled; }
    int getMin() const { return min; }
    int getMax() const { return max; }
    unsigned long getUpdateInterval() const { return updateInterval; }

    void setIsEnabled(bool value) { isEnabled = value; }
    void setMin(int value) { min = value; }
    void setMax(int value) { max = value; }
    void setUpdateInterval(unsigned long interval) { updateInterval = interval; }

    virtual void saveConfigSD()
    {
        StaticJsonDocument<1024> doc;
        auto configSection = doc["input"][id];
        configSection["isEnabled"] = isEnabled;
        configSection["maxVal"] = max;
        configSection["minVal"] = min;
        configSection["updateInterval"] = updateInterval;

        File configFile = SD.open("/inputConfig.json", FILE_WRITE);
        if (!configFile)
        {
            Serial.println("Failed to open input SD config file for writing");
            return;
        }

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write input config to SD file");
        }

        configFile.close();
    }

    virtual void saveConfigFS()
    {
        StaticJsonDocument<1024> doc;
        auto configSection = doc["input"][id];
        configSection["isEnabled"] = isEnabled;
        configSection["maxVal"] = max;
        configSection["minVal"] = min;
        configSection["updateInterval"] = updateInterval;

        File configFile = LittleFS.open("/inputConfig.json", FILE_WRITE);
        if (!configFile)
        {
            Serial.println("Failed to open input FS config file for writing");
            return;
        }

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write input config to FS file");
        }

        configFile.close();
    }

    virtual void loadConfigSD()
    {
        File configFile = SD.open("/inputConfig.json", FILE_READ);
        if (!configFile)
        {
            Serial.println("Failed to open SD config file for reading");
            return;
        }
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration");
        }
        else
        {
            auto configSection = doc["input"][id];
            isEnabled = configSection["isEnabled"] | isEnabled;
            max = configSection["maxVal"] | max;
            min = configSection["minVal"] | min;
        }
        configFile.close();
    }

    virtual void loadConfigFS()
    {
        File configFile = LittleFS.open("/inputConfig.json", FILE_READ);
        if (!configFile)
        {
            Serial.println("Failed to open SD config file for reading");
            return;
        }
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration");
        }
        else
        {
            auto configSection = doc["input"][id];
            isEnabled = configSection["isEnabled"] | isEnabled;
            max = configSection["maxVal"] | max;
            min = configSection["minVal"] | min;
        }
        configFile.close();
    }
};

#endif