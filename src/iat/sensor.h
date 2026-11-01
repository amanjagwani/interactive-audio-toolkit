#ifndef SENSOR_H
#define SENSOR_H
#include "config.h"
#include <Arduino.h>
#include "sequencerConfig.h"
#include "outputHandler.h"

namespace iat
{

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

    virtual const char *getType() const { return "sensor"; }
    virtual int getFullScale() const { return 100; }
};
}

#endif
