#ifndef SENSOR_H
#define SENSOR_H
#include "config.h"
#include <Arduino.h>
#include "sequencerConfig.h"
#include "outputHandler.h"

struct Sensor
{
    uint16_t reading;
    bool plot;
    bool isEnabled;
    bool isConnected;
    bool isLoadCell;
    int min, max;
    String sensorId;
    unsigned long updateInterval;
    SequencerConfig lowSeq, midSeq, highSeq, lowSeq2, midSeq2, highSeq2;
    OutputHandler lowHandler, midHandler, highHandler, lowHandler2, midHandler2, highHandler2;

    Sensor(unsigned long interval, const String &id, bool loadCell) : sensorId(id), updateInterval(interval), reading(0), min(0), max(5000), plot(false), isEnabled(false), isConnected(true),
                                                                      lowSeq(String(String(sensorId) + "-low")), lowHandler(String(String(sensorId) + "-low")), midSeq(String(String(sensorId) + "-mid")), midHandler(String(String(sensorId) + "-mid")),
                                                                      highSeq(String(String(sensorId) + "-high")), highHandler(String(String(sensorId) + "-high")), isLoadCell(loadCell), lowSeq2(String(String(sensorId) + "-low2")), lowHandler2(String(String(sensorId) + "-low2")), midSeq2(String(String(sensorId) + "-mid2")), midHandler2(String(String(sensorId) + "-mid2")),
                                                                      highSeq2(String(String(sensorId) + "-high2")), highHandler2(String(String(sensorId) + "-high2")) {};

    void init()
    {
        if (isConnected)
        {
            lowSeq.init(isLoadCell, &reading, &min, &max);
            lowHandler.init(&lowSeq);
            midSeq.init(isLoadCell, &reading, &min, &max);
            midHandler.init(&midSeq);
            highSeq.init(isLoadCell, &reading, &min, &max);
            highHandler.init(&highSeq);
            lowSeq2.init(isLoadCell, &reading, &min, &max);
            lowHandler2.init(&lowSeq2);
            midSeq2.init(isLoadCell, &reading, &min, &max);
            midHandler2.init(&midSeq2);
            highSeq2.init(isLoadCell, &reading, &min, &max);
            highHandler2.init(&highSeq2);
        }
    }
    String getSensorId()
    {
        return sensorId;
    }

    uint16_t getReading()
    {
        return reading;
    }

    virtual void saveConfigSD()
    {
        StaticJsonDocument<1024> doc;
        auto configSection = doc["input"][sensorId];
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
        auto configSection = doc["input"][sensorId];
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
            auto configSection = doc["input"][sensorId];
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
            auto configSection = doc["input"][sensorId];
            isEnabled = configSection["isEnabled"] | isEnabled;
            max = configSection["maxVal"] | max;
            min = configSection["minVal"] | min;
        }
        configFile.close();
    }
};
#endif