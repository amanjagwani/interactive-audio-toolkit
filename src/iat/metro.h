#ifndef METRO_H
#define METRO_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <atomic>

namespace iat
{

class Metro
{
    std::atomic<unsigned long> lastClick;
    std::atomic<bool> isRunning;
    std::atomic<float> millisPerBeat;
    std::atomic<bool> click;
    std::atomic<int> bpm;

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
        Serial.println(bpm.load());
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
};

}

#endif
