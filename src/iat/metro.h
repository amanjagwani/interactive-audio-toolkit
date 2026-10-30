#ifndef METRO_H
#define METRO_H

#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>

namespace iat
{

class Metro
{
    unsigned long lastClick;
    bool isRunning;
    float millisPerBeat;
    bool click;
    int bpm;

public:
    Metro(int bpm) : lastClick(0), isRunning(0), bpm(bpm)
    {
        millisPerBeat = 60000.0f / bpm;
    }

    void start()
    {
        isRunning = 1;
        lastClick = millis();
    }
    void setSavedBpm()
    {
        millisPerBeat = 60000.0f / (bpm + 0.000001);
        Serial.print("Metro loaded saved BPM: ");
        Serial.println(bpm);
    }
    void setBpm(int globBpm)
    {
        bpm = globBpm;
        millisPerBeat = 60000.0f / (globBpm + 0.000001);
    }

    int getBpm()
    {
        return bpm;
    }

    bool getClick()
    {
        if (isRunning && (millis() - lastClick) >= millisPerBeat)
        {
            click = 1;
            lastClick = millis();
        }
        else
        {
            click = 0;
        }

        return click;
    }

    void stop()
    {
        isRunning = 0;
        click = 0;
    }

    bool getStatus()
    {
        return isRunning;
    }
    void saveConfig(fs::FS &fs)
    {
        JsonDocument doc;
        auto configSection = doc["metro"];
        configSection["bpm"] = bpm;

        File configFile = fs.open("/metroConfig.json", FILE_WRITE);
        if (!configFile)
        {
            Serial.println("Failed to open metro config file for writing");
            return;
        }

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write metro config file");
        }

        configFile.close();
    }

    void loadConfig(fs::FS &fs)
    {
        File configFile = fs.open("/metroConfig.json", FILE_READ);
        if (!configFile)
        {
            Serial.println("Failed to open config file for reading: metro");
            return;
        }
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration: metro");
        }
        else
        {
            auto configSection = doc["metro"];
            bpm = configSection["bpm"] | bpm;
        }
        configFile.close();
    }
};

}

#endif
