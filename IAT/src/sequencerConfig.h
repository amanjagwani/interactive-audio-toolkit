#ifndef SEQUENCER_CONFIG_H
#define SEQUENCER_CONFIG_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFs.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "input.h"

class SequencerConfig
{
private:
    uint16_t pitches[32];
    float velocities[32];
    float probability;
    int numSteps;
    uint8_t subDiv;
    uint32_t threshold;
    uint32_t sensorValue;
    uint8_t mode;
    uint16_t runTime;
    int maxValue;
    int minValue;
    int thresholdRange;
    bool isWithinRange;
    bool thresMode;
    unsigned long sequencerStartTime;
    Input *input;

    bool sequencerActive;
    bool manualActivation;
    bool trigEnable;
    const String seqId;
    const String configPath = "/seqConfig.json";

public:
    SequencerConfig(const String &id, Input *sensor, bool thresholdMode)
        : seqId(id), numSteps(16), subDiv(4), threshold(1000), input(sensor), mode(0),
          sensorValue(0), runTime(10), sequencerActive(false), trigEnable(false), probability(100), maxValue(0),
          minValue(0), thresholdRange(0), isWithinRange(false), thresMode(thresholdMode), sequencerStartTime(0), manualActivation(false)
    {
        for (int i = 0; i < 32; ++i)
        {
            pitches[i] = 40;
            velocities[i] = 1.0;
        }
    }

    uint16_t getPitch(int index) const { return pitches[index]; }
    float getVelocity(int index) const { return velocities[index]; }
    float getProbability() const { return probability; }
    int getNumSteps() const { return numSteps; }
    uint8_t getSubDiv() const { return subDiv; }
    uint32_t getThreshold() const { return threshold; }
    uint32_t getSensorValue() const { return sensorValue; }
    uint8_t getMode() const { return mode; }
    uint16_t getRunTime() const { return runTime; }
    int getMaxValue() const { return maxValue; }
    int getMinValue() const { return minValue; }
    int getThresholdRange() const { return thresholdRange; }
    bool getIsWithinRange() const { return isWithinRange; }
    bool getThresMode() const { return thresMode; }
    unsigned long getSequencerStartTime() const { return sequencerStartTime; }
    Input *getInput() const { return input; }
    bool getSequencerActive() const { return sequencerActive; }
    bool getManualActivation() const { return manualActivation; }
    bool getTrigEnable() const { return trigEnable; }
    String getSeqId() const { return seqId; }

    void setPitch(int index, uint16_t value)
    {
        if (index >= 0 && index < 32)
        {
            pitches[index] = value;
        }
    }
    void setVelocity(int index, float value)
    {
        if (index >= 0 && index < 32 && value >= 0.0 && value <= 1.0)
        {
            velocities[index] = value;
        }
    }
    void setProbability(float value)
    {
        if (value >= 0.0 && value <= 100.0)
        {
            probability = value;
        }
    }
    void setNumSteps(int value)
    {
        if (value > 0 && value < 33)
        {
            numSteps = value;
        }
    }
    void setSubDiv(uint8_t value)
    {
        if (value > 0 && value <= 64)
        {
            subDiv = value;
        }
    }
    void setThreshold(uint32_t value)
    {
        threshold = value;
    }

    void setThresholdRange(uint32_t value)
    {
        thresholdRange = value;
    }

    void setMode(uint8_t value)
    {
        if (value > 0 && value < 3)
            mode = value;
    }

    void setRunTime(uint16_t value)
    {
        runTime = value;
    }

    void setThresMode(bool value)
    {
        thresMode = value;
    }

    void setSequencerActive(bool value)
    {
        sequencerActive = value;
    }
    void setManualActivation(bool value)
    {
        manualActivation = value;
    }
    void setTrigEnable(bool value)
    {
        trigEnable = value;
    }

    void setSensorVal()
    {
        sensorValue = input->getReading();
        maxValue = input->getMax();
        minValue = input->getMin();
    }

    String getConfigFileName(const String &seqId)
    {
        return "/seqConfig" + seqId + ".json";
    }

    void setTrigger()
    {
        unsigned long currentTime = millis();
        if (thresMode) // if threshold mode is 1 then triggers when above threshold.
        {
            if (thresholdRange > 10)
            {
                isWithinRange = sensorValue >= minValue && sensorValue <= threshold + thresholdRange && sensorValue > threshold;
            }
            else if (thresholdRange < 10)
            {
                isWithinRange = sensorValue >= minValue && sensorValue > threshold;
            }
        }
        else // if threshold mode is 0 then triggers when under thresholds.
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
