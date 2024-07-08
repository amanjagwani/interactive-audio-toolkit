#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include "sequencerConfig.h"
#include "output.h"
#include <SD.h>

class OutputHandler_t
{
    SequencerConfig_t *seqConfig;
    Output_t *output;
    int lastStep;
    const String configPath = "/seqConfig.json";
    unsigned long lastRenderTime = 0;
    const unsigned long renderInterval = 2000;

public:
    int8_t outputType;
    uint8_t randVal;

    const String seqId;

    OutputHandler_t(const String &id, SequencerConfig_t *config) : seqId(id), randVal(60), seqConfig(config), output(nullptr), lastStep(0), outputType(-1){};

    void setMode()
    {
        if (output != nullptr && seqConfig != nullptr)
        {
            output->setMode(seqConfig->mode);
        }
    }

    void setSteps()
    {
        if (output != nullptr && seqConfig != nullptr)
        {
            output->setSteps(seqConfig->numSteps);
        }
    }
    void enableOutputTriggering()
    {
        bool isWithinRange;
        if (seqConfig != nullptr)
        {
            if (seqConfig->sensorInput == 4 || seqConfig->sensorInput == 5)
            {
                isWithinRange = seqConfig->sensorValue >= seqConfig->minValue && seqConfig->sensorValue <= seqConfig->threshold + seqConfig->thresholdRange && seqConfig->sensorValue > seqConfig->threshold;
            }
            else
                isWithinRange = seqConfig->sensorValue >= seqConfig->minValue && seqConfig->sensorValue >= seqConfig->threshold - seqConfig->thresholdRange && seqConfig->sensorValue < seqConfig->threshold;

            if (output != nullptr)
            {
                if (isWithinRange)
                {
                    output->setSeqConfig(seqConfig);
                }
            }
        }
    }

    void setOutput(Output_t *newOut)
    {
        if (output != nullptr)
        {
            output->resetConfig();
        }

        output = newOut;
        if (output != nullptr && seqConfig != nullptr)
        {
            output->setSeqConfig(seqConfig);
        }
    }

    void renderStepProbability()
    {
        if (seqConfig != nullptr && output != nullptr)
        {
            int currentStep = output->getStep();

            if (currentStep == seqConfig->numSteps - 1)
            {
                if (seqConfig->probability < 100)
                {
                    for (int i = 0; i < seqConfig->numSteps; i++)
                    {
                        int rand = random(0, randVal);
                        if (rand > seqConfig->probability)
                        {
                            output->setPitch(i, 0);
                            output->setVelocity(i, 0);
                        }
                    }
                }
                else
                {
                    setStepVals();
                }
            }
        }
    }

    void setStepVals()
    {
        if (!output || !seqConfig)
        {
            Serial.println("output or seqConfig is null");
            return;
        }
        else
        {
            for (int i = 0; i < 32; i++)
            {
                output->setPitch(i, seqConfig->pitches[i]);
                output->setVelocity(i, seqConfig->velocities[i]);
            }
        }
    }

    int getStep()
    {
        if (!output)
        {
            return 0;
        }
        return output->getStep();
    }

    void sendCurrentStep(std::function<void(const String &, int)> sendIntToClient,
                         std::function<void(const String &, bool)> sendBoolToClient)
    {
        if (output != nullptr && seqConfig != nullptr)
        {
            if (seqConfig->animationEnable)
            {
                if (seqConfig->sequencerActive)
                {
                    int thisStep = output->getStep();
                    if (thisStep != lastStep)
                    {
                        sendIntToClient(seqConfig->seqId + "CurrentStep", thisStep);
                        sendBoolToClient(seqConfig->seqId + "IsPlaying", seqConfig->sequencerActive);
                        lastStep = thisStep;
                    }
                }
            }
        }
    }

    String getHandlerConfigFileName(const String &seqId)
    {
        return "/handlerConfig" + seqId + ".json";
    }

    void loadConfigSD()
    {
        String filePath = getHandlerConfigFileName(seqId);
        File configFile = SD.open(filePath.c_str(), FILE_READ);
        if (!configFile)
        {
            Serial.println("Failed to open config file for reading: " + seqId);
            return;
        }

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration: " + seqId);
        }
        else
        {
            outputType = doc["outputType"] | outputType;
            randVal = doc["randVal"] | randVal;
        }
        configFile.close();
    }

    void saveConfigSD()
    {
        String filePath = getHandlerConfigFileName(seqId);
        File configFile = SD.open(filePath.c_str(), FILE_WRITE);
        if (!configFile)
        {
            Serial.println("Failed to open config file for writing: " + seqId);
            return;
        }

        StaticJsonDocument<256> doc;

        doc["outputType"] = outputType;
        doc["randVal"] = randVal;

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write to file for: " + seqId);
        }

        configFile.close();
    }

    void init()
    {
        setSteps();
        setMode();
        setStepVals();
    }
};
#endif