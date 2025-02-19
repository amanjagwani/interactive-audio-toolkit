#ifndef CONTROL_HANDLERS_H
#define CONTROL_HANDLERS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <functional>
#include "Sensor.h"
#include "sequencerConfig.h"
#include "outputHandler.h"
#include "output.h"

static std::map<String, Sensor *> sensorMap;

static inline void initSensors(Sensor *sensors[], size_t sensorCount)
{
    for (size_t i = 0; i < sensorCount; i++)
    {
        sensors[i]->init();
        sensorMap[sensors[i]->getSensorId()] = sensors[i];
    }
}

static std::map<String, Output *> outputMap;

static inline void initOutputs(Output *outputs[], size_t outputCount)
{
    for (size_t i = 0; i < outputCount; i++)
    {
        outputMap[outputs[i]->getId()] = outputs[i];
    }
}

static std::map<String, std::function<void(JsonVariant)>> controlHandlers;

static inline void setupControlHandlersForSensor(Sensor *sensor)
{
    String sensorId = sensor->getSensorId();
    controlHandlers[sensorId + "-plot"] = [sensor](JsonVariant value)
    {
        sensor->plot = value.as<bool>();
    };
    controlHandlers[sensorId + "-isEnabled"] = [sensor](JsonVariant value)
    {
        sensor->isEnabled = value.as<bool>();
    };
    controlHandlers[sensorId + "-min"] = [sensor](JsonVariant value)
    {
        sensor->min = value.as<int>();
    };
    controlHandlers[sensorId + "-max"] = [sensor](JsonVariant value)
    {
        sensor->max = value.as<int>();
    };
    controlHandlers[sensorId + "-updateInterval"] = [sensor](JsonVariant value)
    {
        sensor->updateInterval = value.as<unsigned long>();
    };

    SequencerConfig *seqConfigs[6] = {
        &sensor->lowSeq, &sensor->midSeq, &sensor->highSeq,
        &sensor->lowSeq2, &sensor->midSeq2, &sensor->highSeq2};

    OutputHandler *outputHandlers[6] = {
        &sensor->lowHandler, &sensor->midHandler, &sensor->highHandler,
        &sensor->lowHandler2, &sensor->midHandler2, &sensor->highHandler2};

    for (int i = 0; i < 6; i++)
    {
        SequencerConfig *sc = seqConfigs[i];
        OutputHandler *oh = outputHandlers[i];
        String seqId = sc->getSeqId();
        controlHandlers[seqId + "-threshold"] = [sc](JsonVariant value)
        {
            sc->setThreshold(value.as<uint32_t>());
        };
        controlHandlers[seqId + "-thresholdRange"] = [sc](JsonVariant value)
        {
            sc->setThresholdRange(value.as<uint32_t>());
        };
        controlHandlers[seqId + "-subDiv"] = [sc](JsonVariant value)
        {
            sc->setSubDiv(value.as<uint8_t>());
        };
        controlHandlers[seqId + "-runTime"] = [sc](JsonVariant value)
        {
            sc->setRunTime(value.as<uint16_t>());
        };
        controlHandlers[seqId + "-trigEnable"] = [sc](JsonVariant value)
        {
            sc->setTrigEnable(value.as<bool>());
        };
        controlHandlers[seqId + "-manualActivation"] = [sc](JsonVariant value)
        {
            sc->setManualActivation(value.as<bool>());
        };
        controlHandlers[seqId + "-sensorTrigPriority"] = [sc](JsonVariant value)
        {
            sc->setPriorityLevel(value.as<int8_t>());
        };
        controlHandlers[seqId + "-mode"] = [sc, oh](JsonVariant value)
        {
            uint8_t modeValue = value.as<uint8_t>();
            sc->setMode(modeValue);
            oh->setMode();
        };
        controlHandlers[seqId + "-numSteps"] = [sc, oh](JsonVariant value)
        {
            uint8_t numStepsValue = value.as<uint8_t>();
            sc->setNumSteps(numStepsValue);
            oh->setSteps();
        };
        controlHandlers[seqId + "-randVal"] = [oh](JsonVariant value)
        {
            oh->setProbWeight(value.as<int>());
        };
        controlHandlers[seqId + "-pitches"] = [oh, sc](JsonVariant value)
        {
            JsonArray pitches = value.as<JsonArray>();
            for (size_t j = 0; j < pitches.size(); j++)
            {
                sc->setPitch(j, pitches[j]);
            }
            oh->setPitches();
        };
        controlHandlers[seqId + "-velocities"] = [oh, sc](JsonVariant value)
        {
            JsonArray velocities = value.as<JsonArray>();
            for (size_t j = 0; j < velocities.size(); j++)
            {
                sc->setVelocity(j, velocities[j]);
            }
            oh->setVelocities();
        };
        controlHandlers[seqId + "-output"] = [oh, sc](JsonVariant value)
        {
            sc->setSequencerActive(false);
            int indexValue = value.as<int>();
            int count = 0;
            for (auto &outPair : outputMap)
            {
                if (count == indexValue)
                {
                    oh->setOutput(outPair.second);
                    break;
                }
                count++;
            }
        };
    }
}

static inline void setupAllControlHandlers()
{
    for (auto &pair : sensorMap)
    {
        setupControlHandlersForSensor(pair.second);
    }
}

static inline void processControlMessage(const String &jsonMessage)
{
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonMessage);
    if (error)
    {
        Serial.println("JSON parsing error");
        return;
    }
    String controlName = doc["controlName"];
    JsonVariant value = doc["value"];
    if (controlHandlers.find(controlName) != controlHandlers.end())
    {
        controlHandlers[controlName](value);
        Serial.println("Control updated: " + controlName);
    }
    else
    {
        Serial.println("No control handler for: " + controlName);
    }
}

#endif
