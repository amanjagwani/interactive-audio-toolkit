#ifndef SEQUENCER_CONFIG_H
#define SEQUENCER_CONFIG_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFs.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "input.h"

struct SequencerConfig_t
{
    uint16_t pitches[32];
    float velocities[32];
    float probability;
    int numSteps;
    uint8_t subDiv;
    uint32_t threshold;
    uint32_t sensorValue;
    uint8_t sensorInput;
    uint8_t mode;
    uint8_t output;
    uint16_t runTime;
    int maxValue;
    int minValue;
    int thresholdRange;
    bool isWithinRange = false;
    unsigned long sequencerStartTime = 0;
    bool animationEnable;

    bool sequencerActive;
    bool manualActivation;
    bool trigEnable;
    const String seqId;
    const String configPath = "/seqConfig.json";

    Input_t *input;

    SequencerConfig_t(const String &id)
        : seqId(id), numSteps(16), subDiv(4), threshold(1000), sensorInput(0), mode(0),
          output(0), sensorValue(0), runTime(10), sequencerActive(false), trigEnable(false), probability(100), maxValue(0), animationEnable(false),
          input(nullptr)
    {
        for (int i = 0; i < 32; ++i)
        {
            pitches[i] = 40;
            velocities[i] = 1.0;
        }
    }

    void setSensorVal(Input_t *sensor)
    {
        sensorValue = sensor->getReading();
        maxValue = sensor->getMax();
        minValue = sensor->getMin();
    }

    String getConfigFileName(const String &seqId)
    {
        return "/seqConfig" + seqId + ".json";
    }

    void setTrigger()
    {
        unsigned long currentTime = millis();
        if (sensorInput == 4 || sensorInput == 5) // if loadcells then triggers when above thresholds.
        {
            if (thresholdRange > 10)
            {
                isWithinRange = sensorValue >= minValue && sensorValue < 300 && sensorValue <= threshold + thresholdRange && sensorValue > threshold;
            }
            else if (thresholdRange < 10)
            {
                isWithinRange = sensorValue >= minValue && sensorValue > threshold;
            }
        }
        else // if other sensors then triggers when under thresholds.
        {
            if (thresholdRange > 10)
            {
                isWithinRange = sensorValue >= minValue && sensorValue >= threshold - thresholdRange && sensorValue < threshold;
            }
            else if (thresholdRange < 10)
            {
                isWithinRange = sensorValue >= minValue && sensorValue < threshold;
            }
        }
        if (manualActivation)
        {
            probability = 100;
        }
        if (trigEnable)
        {
            if (isWithinRange && !sequencerActive)
            {
                sequencerActive = true;
                sequencerStartTime = currentTime;
                probability = 100;
                manualActivation = false;
            }

            if (sequencerActive && !isWithinRange)
            {
                unsigned long elapsedTime = currentTime - sequencerStartTime;

                if (elapsedTime < runTime * 1000)
                {
                    float decrementStep = (100.0 / (runTime * 1000)) * elapsedTime;
                    probability = 100 - decrementStep;

                    if (probability < 0)
                    {
                        probability = 0;
                    }
                }
                else
                {
                    sequencerActive = false;
                }
            }
        }
    }

    void saveConfigSD()
    {
        String filePath = getConfigFileName(seqId);
        File configFile = SD.open(filePath.c_str(), FILE_WRITE);
        if (!configFile)
        {
            Serial.println("Failed to open config file for writing: " + seqId);
            return;
        }

        StaticJsonDocument<4096> doc;

        doc["runTime"] = runTime;
        doc["threshold"] = threshold;
        doc["trigEnable"] = trigEnable;
        doc["subDiv"] = subDiv;
        doc["sensorInput"] = sensorInput;
        doc["mode"] = mode;
        doc["numSteps"] = numSteps;
        doc["maxVal"] = maxValue;
        doc["minVal"] = minValue;
        doc["thresholdRange"] = thresholdRange;

        JsonArray jpitches = doc.createNestedArray("pitches");
        JsonArray jvelocities = doc.createNestedArray("velocities");

        for (int i = 0; i < 32; i++)
        {
            jpitches.add(this->pitches[i]);
            jvelocities.add(this->velocities[i]);
        }

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write to file for: " + seqId);
        }

        configFile.close();
    }

    void loadConfigSD()
    {
        String filePath = getConfigFileName(seqId);
        File configFile = SD.open(filePath.c_str(), FILE_READ);
        if (!configFile)
        {
            Serial.println("Failed to open config file for reading: " + seqId);
            return;
        }

        StaticJsonDocument<4096> doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration: " + seqId);
        }
        else
        {
            runTime = doc["runTime"] | runTime;
            threshold = doc["threshold"] | threshold;
            trigEnable = doc["trigEnable"] | trigEnable;
            subDiv = doc["subDiv"] | subDiv;
            sensorInput = doc["sensorInput"] | sensorInput;
            mode = doc["mode"] | mode;
            numSteps = doc["numSteps"] | numSteps;
            maxValue = doc["maxVal"] | maxValue;
            minValue = doc["minVal"] | minValue;
            thresholdRange = doc["thresholdRange"] | thresholdRange;
            JsonArray jPitches = doc["pitches"];
            JsonArray jVelocities = doc["velocities"];
            for (size_t i = 0; i < jPitches.size(); i++)
            {
                pitches[i] = jPitches[i];
                velocities[i] = jVelocities[i];
            }
        }
        configFile.close();
    }
};

#endif