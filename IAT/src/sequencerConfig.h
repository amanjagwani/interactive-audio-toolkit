#ifndef SEQUENCER_CONFIG_H
#define SEQUENCER_CONFIG_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFs.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>
#include <atomic>

struct ActivationConfig
{
    std::atomic<bool> isActive{false};
    int8_t priorityLevel{0};
    unsigned long startTime;
    float probability;
};

class SequencerConfig
{
private:
    uint16_t pitches[32];
    float velocities[32];
    float probability;
    int numSteps;
    uint8_t subDiv;
    uint32_t threshold;
    uint16_t *sensorValue;
    uint8_t mode;
    uint8_t output;
    uint16_t runTime;
    int *maxValue;
    int *minValue;
    int thresholdRange;
    bool isWithinRange = false;
    bool thresMode;
    unsigned long sequencerStartTime = 0;

    std::atomic<bool> sequencerActive;
    std::atomic<bool> manualActivation;
    std::atomic<bool> cooldown;
    ActivationConfig sensorActivation;
    ActivationConfig globalActivation;
    bool trigEnable;
    bool isLoadCell;
    const String seqId;
    const String configPath = "/seqConfig.json";

public:
    ActivationConfig *currentActivation;

    SequencerConfig(const String &id)
        : seqId(id), numSteps(16), subDiv(4), threshold(1000), mode(0),
          output(0), sensorValue(0), runTime(10), sequencerActive(false), trigEnable(false), probability(100), maxValue(0), currentActivation(nullptr), cooldown(false),
          isLoadCell(false)
    {
    }

    void init(bool loadCell, uint16_t *sensorReading, int *min, int *max)
    {
        sensorValue = sensorReading;
        minValue = min;
        maxValue = max;
        isLoadCell = loadCell;
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
    uint8_t getMode() const { return mode; }
    uint16_t getRunTime() const { return runTime; }
    int getThresholdRange() const { return thresholdRange; }
    bool getIsWithinRange() const { return isWithinRange; }
    bool getThresMode() const { return thresMode; }
    unsigned long getSequencerStartTime() const { return sequencerStartTime; }
    bool getSequencerActive() const { return sequencerActive; }
    bool getManualActivation() const { return manualActivation; }
    bool getTrigEnable() const { return trigEnable; }
    int8_t getPriorityLevel() const { return sensorActivation.priorityLevel; };
    String getSeqId() const { return seqId; }

    void setPriorityLevel(int8_t priority)
    {
        sensorActivation.priorityLevel = priority;
    }

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

    String getConfigFileName(const String &seqId)
    {
        return "/seqConfig" + seqId + ".json";
    }

    void processSensorActivation()
    {
        if (cooldown.load())
        {
            sensorActivation.isActive = false;
            return;
        }
        if (isLoadCell) // if loadcells then triggers when above thresholds.
        {
            if (thresholdRange > 10)
            {
                isWithinRange = *sensorValue >= *minValue && *sensorValue < *maxValue && *sensorValue <= threshold + thresholdRange && *sensorValue > threshold;
            }
            else if (thresholdRange < 10)
            {
                isWithinRange = *sensorValue >= *minValue && *sensorValue > threshold;
            }
        }
        else // if other sensors then triggers when under thresholds.
        {
            if (thresholdRange > 10)
            {
                isWithinRange = *sensorValue >= *minValue && *sensorValue >= threshold - thresholdRange && *sensorValue < threshold;
            }
            else if (thresholdRange < 10)
            {
                isWithinRange = *sensorValue >= *minValue && *sensorValue < threshold;
            }
        }
        if (trigEnable)
        {
            bool prevSensorActivation = sensorActivation.isActive.load();

            unsigned long currentTime = millis();

            if (isWithinRange && !sensorActivation.isActive)
            {
                sensorActivation.isActive = true;
                sensorActivation.startTime = currentTime;
                sensorActivation.probability = 100;
            }

            if (sensorActivation.isActive && !isWithinRange)
            {
                unsigned long elapsedTime = currentTime - sensorActivation.startTime;

                if (elapsedTime < runTime * 1000)
                {
                    float decrementStep = (100.0 / (runTime * 1000)) * elapsedTime;
                    sensorActivation.probability = std::max(100 - decrementStep, 0.0f);
                }
                else
                {
                    sensorActivation.isActive = false;
                    sensorActivation.startTime = 0;
                }
            }
        }
    }

    void setTrigger()

    {
        if (trigEnable)
        {

            processSensorActivation();

            if (sensorActivation.isActive && globalActivation.isActive)
            {
                if (sensorActivation.priorityLevel >= globalActivation.priorityLevel)
                {
                    sequencerActive.store(sensorActivation.isActive.load());
                    currentActivation = &sensorActivation;
                }
                else
                {
                    sequencerActive.store(globalActivation.isActive.load());
                    currentActivation = &globalActivation;
                }
                probability = currentActivation->probability;
            }
            else if (sensorActivation.isActive)
            {
                sequencerActive.store(sensorActivation.isActive.load());
                currentActivation = &sensorActivation;
                probability = sensorActivation.probability;
            }
            else if (globalActivation.isActive)
            {
                sequencerActive.store(globalActivation.isActive.load());
                currentActivation = &globalActivation;
                probability = globalActivation.probability;
            }
            else
            {
                sequencerActive.store(false);
                probability = 100;
                currentActivation = nullptr;
            }
        }
        else
        {
            sequencerActive.store(manualActivation.load());
            probability = 100;
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
        doc["thresholdRange"] = thresholdRange;
        doc["isLoadCell"] = isLoadCell;
        doc["sensorTrigPriority"] = sensorActivation.priorityLevel;

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
            thresholdRange = doc["thresholdRange"] | thresholdRange;
            isLoadCell = doc["isLoadCell"] | isLoadCell;
            sensorActivation.priorityLevel = doc["sensorTrigPriority"] | sensorActivation.priorityLevel;
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
