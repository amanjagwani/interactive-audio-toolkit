#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include "sequencerConfig.h"
#include "output.h"
#include <SD.h>

class OutputHandler
{
    SequencerConfig *seqConfig;
    Output *output;
    int lastStep;
    const String configPath = "/seqConfig.json";
    unsigned long lastRenderTime = 0;
    const unsigned long renderInterval = 2000;
    uint8_t randVal;

public:
    const String seqId;

    OutputHandler(const String &id, Output *out, SequencerConfig *config) : seqId(id), randVal(60), seqConfig(config), output(out), lastStep(0)
    {
        if (output != nullptr && seqConfig != nullptr)
        {
            output->setSeqConfig(seqConfig);
        }
    }

    void enableOutputTriggering()
    {
        bool isWithinRange = false;
        if (seqConfig != nullptr)
        {
            if (seqConfig->getThresMode())
            {
                isWithinRange = seqConfig->getSensorValue() >= seqConfig->getMinValue() && seqConfig->getSensorValue() <= seqConfig->getThreshold() + seqConfig->getThresholdRange() && seqConfig->getSensorValue() > seqConfig->getThreshold();
            }
            else
            {
                isWithinRange = seqConfig->getSensorValue() >= seqConfig->getMinValue() && seqConfig->getSensorValue() >= seqConfig->getThreshold() - seqConfig->getThresholdRange() && seqConfig->getSensorValue() < seqConfig->getThreshold();
            }

            if (output != nullptr)
            {
                if (isWithinRange)
                {
                    output->setSeqConfig(seqConfig);
                }
            }
        }
    }

    void setProbWeight(uint8_t value)
    {
        randVal = value;
    }

    void setOutput(Output *newOut)
    {
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

            if (currentStep == seqConfig->getNumSteps() - 1)
            {
                if (seqConfig->getProbability() < 100)
                {
                    for (int i = 0; i < seqConfig->getNumSteps(); i++)
                    {
                        int rand = random(0, randVal);
                        if (rand > seqConfig->getProbability())
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
                output->setPitch(i, seqConfig->getPitch(i));
                output->setVelocity(i, seqConfig->getVelocity(i));
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

        doc["randVal"] = randVal;

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write to file for: " + seqId);
        }

        configFile.close();
    }

    void init()
    {
        setStepVals();
    }
};

#endif
