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
    bool stepValsSet = false;

public:
    int8_t outputType;
    uint8_t randVal;

    const String seqId;

    OutputHandler(const String &id) : seqId(id), randVal(60), seqConfig(nullptr), output(nullptr), lastStep(0), outputType(-1) {};

    void init(SequencerConfig *config)
    {
        seqConfig = config;
        setSteps();
        setMode();
        setStepVals();
    }

    void setMode()
    {
        if (output != nullptr && seqConfig != nullptr)
        {
            output->setMode(seqConfig->getMode());
        }
    }

    void setSteps()
    {
        if (output != nullptr && seqConfig != nullptr)
        {
            output->setSteps(seqConfig->getNumSteps());
        }
    }

    void enableOutputTriggering()
    {
        if (seqConfig == nullptr || output == nullptr)
        {
            return;
        }

        if (!seqConfig->getTrigEnable() && seqConfig->getManualActivation())
        {
            output->setSeqConfig(seqConfig);
            return;
        }

        if (seqConfig->getTrigEnable())
        {
            if (seqConfig->currentActivation != nullptr &&
                (output->seqConfig == nullptr ||
                 output->seqConfig->currentActivation == nullptr ||
                 seqConfig->currentActivation->priorityLevel > output->seqConfig->currentActivation->priorityLevel))
            {

                output->setSeqConfig(seqConfig);
                return;
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
            if (seqConfig->getTrigEnable())
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
                        stepValsSet = false;
                    }
                }
                if (seqConfig->getProbability() == 100 && stepValsSet != true)
                {
                    setStepVals();
                    stepValsSet = true;
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

    void setPitches()
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
            }
        }
    }

    void setVelocities()
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
};

#endif
