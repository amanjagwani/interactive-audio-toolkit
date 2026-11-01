#ifndef SEQUENCER_CONFIG_H
#define SEQUENCER_CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <atomic>

namespace iat
{

inline std::atomic<bool> globalCooldown{false};
inline uint32_t deviceTriggerCount = 0;

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
    std::atomic<uint8_t> subDiv;
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

    static const int MAX_TRIGGER_TIMESTAMPS = 60;
    static const unsigned long TRIGGER_DEBOUNCE_MS = 5000;
    static const unsigned long TRIGGER_WINDOW_MS = 60000;
    unsigned long triggerTimestamps[MAX_TRIGGER_TIMESTAMPS];
    int triggerTimestampCount = 0;
    unsigned long autoPlayStartTimestamp = 0;
    unsigned long lastActiveTimestamp = 0;

  public:
    float triggerRate = 0.0;
    float triggerRateHighThreshold = 10;
    unsigned long triggerCooldownInterval = 5000;
    unsigned long triggerInactivityThreshold = 30000;
    int8_t trigRatePriorityLevel = 2;
    bool enableTriggerRateProcessing = false;
    bool enableCooldown = false;
    bool enableAutoplay = false;
    bool toggleAnimation = false;
    bool autoPlay = false;
    bool cooldownStarted = false;
    bool cooldownEnded = false;

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

    void processSensorActivation()
    {
        if (cooldown.load() || globalCooldown.load())
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
            if (isWithinRange)
            {
                recordTrigger(currentTime);
            }
        }
    }

    void recordTrigger(unsigned long timestamp)
    {
        if (triggerTimestampCount > 0 && timestamp - triggerTimestamps[triggerTimestampCount - 1] < TRIGGER_DEBOUNCE_MS)
        {
            return;
        }
        if (triggerTimestampCount == MAX_TRIGGER_TIMESTAMPS)
        {
            memmove(triggerTimestamps, triggerTimestamps + 1, (MAX_TRIGGER_TIMESTAMPS - 1) * sizeof(unsigned long));
            triggerTimestampCount--;
        }
        triggerTimestamps[triggerTimestampCount++] = timestamp;
        lastActiveTimestamp = timestamp;
        deviceTriggerCount++;
    }

    void startGlobalActivation(int8_t priority)
    {
        globalActivation.priorityLevel = priority;
        globalActivation.startTime = 0;
        globalActivation.isActive = true;
    }

    void processGlobalActivation()
    {
        if (trigEnable && globalActivation.isActive)
        {
            unsigned long currentTime = millis();

            if (globalActivation.startTime == 0)
            {
                globalActivation.startTime = currentTime;
                globalActivation.probability = 100;
            }

            unsigned long elapsedTime = currentTime - globalActivation.startTime;
            if (elapsedTime < runTime * 1000)
            {
                float decrementStep = (100.0 / (runTime * 1000)) * elapsedTime;
                globalActivation.probability = std::max(100 - decrementStep, 0.0f);
            }
            else
            {
                globalActivation.isActive = false;
                globalActivation.startTime = 0;
            }
        }
    }

    bool getCooldown() const { return cooldown.load(); }

    void setCooldown(bool value)
    {
        cooldown.store(value);
        if (value)
        {
            lastActiveTimestamp = millis();
            triggerTimestampCount = 0;
            triggerRate = 0;
        }
    }

    void processTriggerRate()
    {
        cooldownStarted = false;
        cooldownEnded = false;
        if (!trigEnable || !enableTriggerRateProcessing)
        {
            if (cooldown.load())
            {
                cooldown.store(false);
                cooldownEnded = true;
            }
            return;
        }
        unsigned long currentTime = millis();
        int newCount = 0;
        for (int i = 0; i < triggerTimestampCount; i++)
        {
            if (currentTime - triggerTimestamps[i] <= TRIGGER_WINDOW_MS)
            {
                triggerTimestamps[newCount++] = triggerTimestamps[i];
            }
        }
        triggerTimestampCount = newCount;
        triggerRate = (float)triggerTimestampCount;

        if (enableCooldown)
        {
            if (triggerRate >= triggerRateHighThreshold && !cooldown.load())
            {
                setCooldown(true);
                cooldownStarted = true;
            }
            else if (cooldown.load() && currentTime - lastActiveTimestamp >= triggerCooldownInterval)
            {
                cooldown.store(false);
                cooldownEnded = true;
            }
        }

        if (enableAutoplay)
        {
            if (!autoPlay && currentTime - lastActiveTimestamp >= triggerInactivityThreshold)
            {
                autoPlay = true;
                autoPlayStartTimestamp = currentTime;
                startGlobalActivation(trigRatePriorityLevel);
            }
            if (autoPlay && currentTime - autoPlayStartTimestamp >= runTime * 1000UL)
            {
                autoPlay = false;
                lastActiveTimestamp = currentTime;
            }
        }
    }

    void setTrigger()

    {
        if (trigEnable)
        {
            processGlobalActivation();
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
};

}

#endif
