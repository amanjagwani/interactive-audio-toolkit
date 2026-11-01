#ifndef CONTROL_HANDLERS_H
#define CONTROL_HANDLERS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <map>
#include <functional>
#include <vector>
#include "sensor.h"
#include "sequencerConfig.h"
#include "outputHandler.h"
#include "output.h"
#include "ring.h"
#include "fmsynth.h"
#include "noiseSynth.h"
#include "loadcell.h"
#include "metro.h"

namespace iat
{

#ifndef IAT_CONTROL_MESSAGE_SIZE
#define IAT_CONTROL_MESSAGE_SIZE 2048
#endif

struct ControlMessage
{
    char json[IAT_CONTROL_MESSAGE_SIZE];
};

using ControlRing = Ring<ControlMessage, 8>;

inline bool sendControlMessage(ControlRing &ring, const char *data, size_t len)
{
    if (len >= IAT_CONTROL_MESSAGE_SIZE)
    {
        return false;
    }
    ControlMessage msg;
    memcpy(msg.json, data, len);
    msg.json[len] = '\0';
    return ring.write(msg);
}

inline std::map<String, Sensor *> sensorMap;

inline void initSensors(Sensor *sensors[], size_t sensorCount)
{
    for (size_t i = 0; i < sensorCount; i++)
    {
        sensors[i]->init();
        sensorMap[sensors[i]->getSensorId()] = sensors[i];
    }
}

inline std::map<String, Output *> outputMap;

inline void initOutputs(Output *outputs[], size_t outputCount)
{
    for (size_t i = 0; i < outputCount; i++)
    {
        outputMap[outputs[i]->getId()] = outputs[i];
    }
}

inline std::map<String, std::function<void(JsonVariant)>> controlHandlers;

inline std::vector<std::function<void(const char *, size_t)>> transports;
inline String deviceMac;
inline String deviceName = "iat";
inline Metro *controlMetro = nullptr;

struct DeviceTriggerRate
{
    static const int MAX_TRIGGER_TIMESTAMPS = 60;
    static const unsigned long TRIGGER_WINDOW_MS = 60000;
    static const unsigned long AUTOPLAY_MS = 10000;
    float triggerRate = 0;
    float triggerRateHighThreshold = 10;
    unsigned long triggerCooldownInterval = 5000;
    unsigned long triggerInactivityThreshold = 30000;
    int8_t trigRatePriorityLevel = 2;
    bool enableTriggerRateProcessing = false;
    bool enableCooldown = false;
    bool enableAutoplay = false;
    bool toggleAnimation = false;
    bool cooldown = false;
    bool autoPlay = false;
    unsigned long triggerTimestamps[MAX_TRIGGER_TIMESTAMPS];
    int triggerTimestampCount = 0;
    unsigned long lastActiveTimestamp = 0;
    unsigned long autoPlayStartTimestamp = 0;
    uint32_t lastTriggerCount = 0;
    unsigned long lastProcessTime = 0;
};

inline DeviceTriggerRate deviceTriggerRate;

inline void addTransport(std::function<void(const char *, size_t)> send)
{
    transports.push_back(send);
}

inline void sendMessage(JsonDocument &doc)
{
    String out;
    serializeJson(doc, out);
    for (auto &send : transports)
    {
        send(out.c_str(), out.length());
    }
}

inline void buildDeviceDescriptor(JsonDocument &doc, const char *macAddress, const char *deviceName)
{
    doc["type"] = "connectedDevices";
    JsonObject pcb = doc["data"].to<JsonArray>().add<JsonObject>();
    pcb["macAddress"] = macAddress;
    pcb["customName"] = deviceName;

    JsonArray sensors = pcb["sensors"].to<JsonArray>();
    for (auto &pair : sensorMap)
    {
        Sensor *sensor = pair.second;
        JsonObject s = sensors.add<JsonObject>();
        s["sensorId"] = sensor->getSensorId();
        s["sensorType"] = sensor->getType();
        s["fullScale"] = sensor->getFullScale();
        JsonArray seqIds = s["seqIds"].to<JsonArray>();
        for (SequencerConfig *sc : {&sensor->lowSeq, &sensor->midSeq, &sensor->highSeq,
                                    &sensor->lowSeq2, &sensor->midSeq2, &sensor->highSeq2})
        {
            seqIds.add(sc->getSeqId());
        }
    }

    JsonArray outputs = pcb["outputs"].to<JsonArray>();
    for (auto &pair : outputMap)
    {
        JsonObject o = outputs.add<JsonObject>();
        o["id"] = pair.second->getId();
        o["type"] = pair.second->getType();
    }
}

inline SequencerConfig *findSequencerConfig(const String &seqId)
{
    for (auto &pair : sensorMap)
    {
        Sensor *sensor = pair.second;
        for (SequencerConfig *sc : {&sensor->lowSeq, &sensor->midSeq, &sensor->highSeq,
                                    &sensor->lowSeq2, &sensor->midSeq2, &sensor->highSeq2})
        {
            if (sc->getSeqId() == seqId)
            {
                return sc;
            }
        }
    }
    return nullptr;
}

inline void processDeviceTriggerRate(unsigned long currentTime)
{
    DeviceTriggerRate &d = deviceTriggerRate;
    while (d.lastTriggerCount != deviceTriggerCount)
    {
        d.lastTriggerCount++;
        if (d.triggerTimestampCount == DeviceTriggerRate::MAX_TRIGGER_TIMESTAMPS)
        {
            memmove(d.triggerTimestamps, d.triggerTimestamps + 1, (DeviceTriggerRate::MAX_TRIGGER_TIMESTAMPS - 1) * sizeof(unsigned long));
            d.triggerTimestampCount--;
        }
        d.triggerTimestamps[d.triggerTimestampCount++] = currentTime;
        d.lastActiveTimestamp = currentTime;
    }

    if (!d.enableTriggerRateProcessing)
    {
        if (d.cooldown)
        {
            d.cooldown = false;
            globalCooldown.store(false);
        }
        return;
    }

    int newCount = 0;
    for (int i = 0; i < d.triggerTimestampCount; i++)
    {
        if (currentTime - d.triggerTimestamps[i] <= DeviceTriggerRate::TRIGGER_WINDOW_MS)
        {
            d.triggerTimestamps[newCount++] = d.triggerTimestamps[i];
        }
    }
    d.triggerTimestampCount = newCount;
    d.triggerRate = (float)d.triggerTimestampCount;

    if (d.enableCooldown)
    {
        if (d.triggerRate >= d.triggerRateHighThreshold && !d.cooldown)
        {
            d.cooldown = true;
            d.lastActiveTimestamp = currentTime;
            d.triggerTimestampCount = 0;
            d.triggerRate = 0;
            globalCooldown.store(true);
        }
        else if (d.cooldown && currentTime - d.lastActiveTimestamp >= d.triggerCooldownInterval)
        {
            d.cooldown = false;
            globalCooldown.store(false);
        }
    }

    if (d.enableAutoplay)
    {
        if (!d.autoPlay && currentTime - d.lastActiveTimestamp >= d.triggerInactivityThreshold)
        {
            d.autoPlay = true;
            d.autoPlayStartTimestamp = currentTime;
            for (auto &pair : sensorMap)
            {
                Sensor *sensor = pair.second;
                for (SequencerConfig *sc : {&sensor->lowSeq, &sensor->midSeq, &sensor->highSeq,
                                            &sensor->lowSeq2, &sensor->midSeq2, &sensor->highSeq2})
                {
                    if (sc->getTrigEnable())
                    {
                        sc->startGlobalActivation(d.trigRatePriorityLevel);
                    }
                }
            }
        }
        if (d.autoPlay && currentTime - d.autoPlayStartTimestamp >= DeviceTriggerRate::AUTOPLAY_MS)
        {
            d.autoPlay = false;
            d.lastActiveTimestamp = currentTime;
        }
    }

    if (d.toggleAnimation)
    {
        JsonDocument doc;
        doc["macAddress"] = deviceMac;
        doc["deviceTriggerRate"] = d.triggerRate;
        doc["deviceCooldown"] = d.cooldown;
        doc["deviceAutoplay"] = d.autoPlay;
        sendMessage(doc);
    }
}

inline void processTriggerRates()
{
    unsigned long currentTime = millis();
    if (currentTime - deviceTriggerRate.lastProcessTime < 250)
    {
        return;
    }
    deviceTriggerRate.lastProcessTime = currentTime;
    processDeviceTriggerRate(currentTime);

    for (auto &pair : sensorMap)
    {
        Sensor *sensor = pair.second;
        SequencerConfig *seqConfigs[6] = {
            &sensor->lowSeq, &sensor->midSeq, &sensor->highSeq,
            &sensor->lowSeq2, &sensor->midSeq2, &sensor->highSeq2};
        OutputHandler *outputHandlers[6] = {
            &sensor->lowHandler, &sensor->midHandler, &sensor->highHandler,
            &sensor->lowHandler2, &sensor->midHandler2, &sensor->highHandler2};

        for (int i = 0; i < 6; i++)
        {
            SequencerConfig *sc = seqConfigs[i];
            Output *output = outputHandlers[i]->getOutput();
            sc->processTriggerRate();

            if ((sc->cooldownStarted || sc->cooldownEnded) && output)
            {
                for (auto &otherPair : sensorMap)
                {
                    Sensor *other = otherPair.second;
                    SequencerConfig *otherSeqs[6] = {
                        &other->lowSeq, &other->midSeq, &other->highSeq,
                        &other->lowSeq2, &other->midSeq2, &other->highSeq2};
                    OutputHandler *otherHandlers[6] = {
                        &other->lowHandler, &other->midHandler, &other->highHandler,
                        &other->lowHandler2, &other->midHandler2, &other->highHandler2};
                    for (int j = 0; j < 6; j++)
                    {
                        if (otherSeqs[j] != sc && otherSeqs[j]->getTrigEnable() && otherHandlers[j]->getOutput() == output &&
                            otherSeqs[j]->getCooldown() != sc->cooldownStarted)
                        {
                            otherSeqs[j]->setCooldown(sc->cooldownStarted);
                        }
                    }
                }
            }

            if (sc->toggleAnimation && sc->getTrigEnable() && sc->enableTriggerRateProcessing)
            {
                JsonDocument doc;
                doc["macAddress"] = deviceMac;
                doc["sensorId"] = sensor->getSensorId();
                doc["seqId"] = sc->getSeqId();
                doc["triggerRate"] = sc->triggerRate;
                doc["cooldown"] = sc->getCooldown();
                doc["autoplay"] = sc->autoPlay;
                sendMessage(doc);
            }
        }
    }
}

inline void sendSensorData()
{
    for (auto &pair : sensorMap)
    {
        Sensor *sensor = pair.second;
        if (!sensor->plot)
        {
            continue;
        }
        JsonDocument doc;
        doc["macAddress"] = deviceMac;
        doc[sensor->getSensorId()] = sensor->getReading();
        sendMessage(doc);
    }
}

inline void sendCurrentSteps()
{
    for (auto &pair : sensorMap)
    {
        Sensor *sensor = pair.second;
        for (OutputHandler *oh : {&sensor->lowHandler, &sensor->midHandler, &sensor->highHandler,
                                  &sensor->lowHandler2, &sensor->midHandler2, &sensor->highHandler2})
        {
            int step = oh->getStep();
            if (step == oh->lastSentStep)
            {
                continue;
            }
            oh->lastSentStep = step;
            JsonDocument doc;
            doc["macAddress"] = deviceMac;
            doc[oh->seqId + "-currentStep"] = step;
            sendMessage(doc);
        }
    }
}

inline void sendSettings()
{
    for (auto &pair : sensorMap)
    {
        Sensor *sensor = pair.second;
        JsonDocument doc;
        doc["macAddress"] = deviceMac;
        JsonObject s = doc["sensorSettings"][sensor->getSensorId()].to<JsonObject>();
        s["isEnabled"] = sensor->isEnabled;
        s["plot"] = sensor->plot;
        s["min"] = sensor->min;
        s["max"] = sensor->max;
        s["updateInterval"] = sensor->updateInterval;
        sendMessage(doc);

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
            JsonDocument seqDoc;
            seqDoc["macAddress"] = deviceMac;
            JsonObject q = seqDoc["sequencerSettings"][sc->getSeqId()].to<JsonObject>();
            q["threshold"] = sc->getThreshold();
            q["thresholdRange"] = sc->getThresholdRange();
            q["subDiv"] = sc->getSubDiv();
            q["runTime"] = sc->getRunTime();
            q["trigEnable"] = sc->getTrigEnable();
            q["manualActivation"] = sc->getManualActivation();
            q["sensorTrigPriority"] = sc->getPriorityLevel();
            q["mode"] = sc->getMode();
            q["numSteps"] = sc->getNumSteps();
            q["randVal"] = oh->randVal;
            q["output"] = oh->getOutput() ? oh->getOutput()->getId() : "none";
            q["triggerRateHighThreshold"] = sc->triggerRateHighThreshold;
            q["triggerCooldownInterval"] = sc->triggerCooldownInterval;
            q["triggerInactivityThreshold"] = sc->triggerInactivityThreshold;
            q["enableTriggerRateProcessing"] = sc->enableTriggerRateProcessing;
            q["enableCooldown"] = sc->enableCooldown;
            q["enableAutoplay"] = sc->enableAutoplay;
            q["trigRatePriorityLevel"] = sc->trigRatePriorityLevel;
            JsonArray pitches = q["pitches"].to<JsonArray>();
            JsonArray velocities = q["velocities"].to<JsonArray>();
            for (int j = 0; j < 32; j++)
            {
                pitches.add(sc->getPitch(j));
                velocities.add(sc->getVelocity(j));
            }
            sendMessage(seqDoc);
        }
    }
}

inline void sendOutputSettings()
{
    for (auto &pair : outputMap)
    {
        Output *output = pair.second;
        String type = output->getType();
        JsonDocument doc;
        doc["macAddress"] = deviceMac;
        JsonObject o = doc["outputSettings"][output->getId()].to<JsonObject>();
        if (type == "fmSynth")
        {
            FmSynth *synth = static_cast<FmSynth *>(output);
            float env[4];
            o["modIndex"] = synth->getModIndex();
            o["modRatio"] = synth->getModRatio();
            o["level"] = synth->getLevel();
            synth->getCarEnv(env);
            JsonArray ampEnv = o["ampEnv"].to<JsonArray>();
            for (float v : env)
            {
                ampEnv.add(v);
            }
            synth->getModEnv(env);
            JsonArray modEnv = o["modEnv"].to<JsonArray>();
            for (float v : env)
            {
                modEnv.add(v);
            }
        }
        else if (type == "noiseSynth")
        {
            NoiseSynth *noise = static_cast<NoiseSynth *>(output);
            o["amp"] = noise->getNoiseAmp();
            o["bandwidth"] = noise->getBandwidth();
            o["modDepth"] = noise->getModDepth();
            o["resonFreq"] = noise->getBaseFreq();
            o["updateRate"] = noise->getUpdateRate();
            o["manualEnabled"] = noise->manualEnabled.load();
            o["triggerEnabled"] = noise->triggerEnabled.load();
        }
        else
        {
            continue;
        }
        sendMessage(doc);
    }

    JsonDocument doc;
    doc["macAddress"] = deviceMac;
    JsonObject g = doc["globalSettings"].to<JsonObject>();
    if (controlMetro)
    {
        g["bpm"] = controlMetro->getBpm();
    }
    g["triggerRateHighThreshold"] = deviceTriggerRate.triggerRateHighThreshold;
    g["triggerCooldownInterval"] = deviceTriggerRate.triggerCooldownInterval;
    g["triggerInactivityThreshold"] = deviceTriggerRate.triggerInactivityThreshold;
    g["enableTriggerRateProcessing"] = deviceTriggerRate.enableTriggerRateProcessing;
    g["enableCooldown"] = deviceTriggerRate.enableCooldown;
    g["enableAutoplay"] = deviceTriggerRate.enableAutoplay;
    g["trigRatePriorityLevel"] = deviceTriggerRate.trigRatePriorityLevel;
    sendMessage(doc);
}

inline fs::FS *settingsFs = nullptr;
inline const char *settingsPath = "/settings.json";

inline void collectSettings(JsonObject out)
{
    for (auto &pair : sensorMap)
    {
        Sensor *sensor = pair.second;
        String sensorId = sensor->getSensorId();
        out[sensorId + "-isEnabled"] = sensor->isEnabled;
        out[sensorId + "-min"] = sensor->min;
        out[sensorId + "-max"] = sensor->max;
        out[sensorId + "-updateInterval"] = sensor->updateInterval;

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
            out[seqId + "-output"] = oh->getOutput() ? oh->getOutput()->getId() : "none";
            out[seqId + "-threshold"] = sc->getThreshold();
            out[seqId + "-thresholdRange"] = sc->getThresholdRange();
            out[seqId + "-subDiv"] = sc->getSubDiv();
            out[seqId + "-runTime"] = sc->getRunTime();
            out[seqId + "-trigEnable"] = sc->getTrigEnable();
            out[seqId + "-sensorTrigPriority"] = sc->getPriorityLevel();
            out[seqId + "-mode"] = sc->getMode();
            out[seqId + "-numSteps"] = sc->getNumSteps();
            out[seqId + "-randVal"] = oh->randVal;
            JsonArray pitches = out[seqId + "-pitches"].to<JsonArray>();
            JsonArray velocities = out[seqId + "-velocities"].to<JsonArray>();
            for (int j = 0; j < 32; j++)
            {
                pitches.add(sc->getPitch(j));
                velocities.add(sc->getVelocity(j));
            }
            out[seqId + "-enableTriggerRateProcessing"] = sc->enableTriggerRateProcessing;
            out[seqId + "-enableCooldown"] = sc->enableCooldown;
            out[seqId + "-enableAutoplay"] = sc->enableAutoplay;
            out[seqId + "-triggerRateHighThreshold"] = sc->triggerRateHighThreshold;
            out[seqId + "-triggerCooldownInterval"] = sc->triggerCooldownInterval;
            out[seqId + "-triggerInactivityThreshold"] = sc->triggerInactivityThreshold;
            out[seqId + "-trigRatePriorityLevel"] = sc->trigRatePriorityLevel;
        }
    }

    for (auto &pair : outputMap)
    {
        Output *output = pair.second;
        String id = output->getId();
        String type = output->getType();
        if (type == "fmSynth")
        {
            FmSynth *synth = static_cast<FmSynth *>(output);
            float env[4];
            out[id + "-modIndex"] = synth->getModIndex();
            out[id + "-modRatio"] = synth->getModRatio();
            out[id + "-level"] = synth->getLevel();
            synth->getCarEnv(env);
            JsonArray ampEnv = out[id + "-ampEnv"].to<JsonArray>();
            for (float v : env)
            {
                ampEnv.add(v);
            }
            synth->getModEnv(env);
            JsonArray modEnv = out[id + "-modEnv"].to<JsonArray>();
            for (float v : env)
            {
                modEnv.add(v);
            }
        }
        else if (type == "noiseSynth")
        {
            NoiseSynth *noise = static_cast<NoiseSynth *>(output);
            out[id + "-amp"] = noise->getNoiseAmp();
            out[id + "-bandwidth"] = noise->getBandwidth();
            out[id + "-modDepth"] = noise->getModDepth();
            out[id + "-resonFreq"] = noise->getBaseFreq();
            out[id + "-updateRate"] = noise->getUpdateRate();
        }
    }

    if (controlMetro)
    {
        out["bpm"] = controlMetro->getBpm();
    }
    out["device-enableTriggerRateProcessing"] = deviceTriggerRate.enableTriggerRateProcessing;
    out["device-enableCooldown"] = deviceTriggerRate.enableCooldown;
    out["device-enableAutoplay"] = deviceTriggerRate.enableAutoplay;
    out["device-triggerRateHighThreshold"] = deviceTriggerRate.triggerRateHighThreshold;
    out["device-triggerCooldownInterval"] = deviceTriggerRate.triggerCooldownInterval;
    out["device-triggerInactivityThreshold"] = deviceTriggerRate.triggerInactivityThreshold;
    out["device-trigRatePriorityLevel"] = deviceTriggerRate.trigRatePriorityLevel;
}

inline void saveSettings()
{
    if (!settingsFs)
    {
        return;
    }
    JsonDocument doc;
    collectSettings(doc.to<JsonObject>());
    File file = settingsFs->open(settingsPath, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open settings file for writing");
        return;
    }
    serializeJson(doc, file);
    file.close();
    Serial.println("Settings saved");
}

inline void loadSettings(fs::FS &fs)
{
    settingsFs = &fs;
    controlHandlers["saveSettings"] = [](JsonVariant) {
        saveSettings();
    };

    File file = settingsFs->open(settingsPath, FILE_READ);
    if (!file)
    {
        return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error)
    {
        Serial.println("Failed to read settings file");
        return;
    }
    for (JsonPair kv : doc.as<JsonObject>())
    {
        auto handler = controlHandlers.find(kv.key().c_str());
        if (handler != controlHandlers.end())
        {
            handler->second(kv.value());
        }
    }
    Serial.println("Settings loaded");
    sendSettings();
    sendOutputSettings();
}

inline void setupControlHandlersForOutput(Output *output)
{
    String id = output->getId();
    String type = output->getType();
    if (type == "fmSynth")
    {
        FmSynth *synth = static_cast<FmSynth *>(output);
        controlHandlers[id + "-modIndex"] = [synth](JsonVariant value) {
            synth->setModIndex(value.as<float>());
        };
        controlHandlers[id + "-modRatio"] = [synth](JsonVariant value) {
            synth->setModRatio(value.as<float>());
        };
        controlHandlers[id + "-level"] = [synth](JsonVariant value) {
            synth->setLevel(value.as<float>());
        };
        controlHandlers[id + "-ampEnv"] = [synth](JsonVariant value) {
            JsonArray env = value.as<JsonArray>();
            synth->setCarEnv(env[0], env[1], env[2], env[3]);
        };
        controlHandlers[id + "-modEnv"] = [synth](JsonVariant value) {
            JsonArray env = value.as<JsonArray>();
            synth->setModEnv(env[0], env[1], env[2], env[3]);
        };
    }
    else if (type == "noiseSynth")
    {
        NoiseSynth *noise = static_cast<NoiseSynth *>(output);
        controlHandlers[id + "-amp"] = [noise](JsonVariant value) {
            noise->setNoiseAmp(value.as<float>());
        };
        controlHandlers[id + "-bandwidth"] = [noise](JsonVariant value) {
            noise->setBandwidth(value.as<float>());
        };
        controlHandlers[id + "-modDepth"] = [noise](JsonVariant value) {
            noise->setModDepth(value.as<float>());
        };
        controlHandlers[id + "-resonFreq"] = [noise](JsonVariant value) {
            noise->setResonFreq(value.as<float>());
        };
        controlHandlers[id + "-updateRate"] = [noise](JsonVariant value) {
            noise->setUpdateRate(value.as<float>());
        };
        controlHandlers[id + "-manualEnabled"] = [noise](JsonVariant value) {
            noise->manualEnabled = value.as<bool>();
        };
        controlHandlers[id + "-triggerEnabled"] = [noise](JsonVariant value) {
            noise->triggerEnabled = value.as<bool>();
        };
    }
}

inline void setupMetroControlHandlers(Metro &metro)
{
    controlMetro = &metro;
    controlHandlers["bpm"] = [](JsonVariant value) {
        controlMetro->setBpm(value.as<int>());
    };
}

inline void setupControlHandlersForSensor(Sensor *sensor)
{
    String sensorId = sensor->getSensorId();
    controlHandlers[sensorId + "-plot"] = [sensor](JsonVariant value) {
        sensor->plot = value.as<bool>();
    };
    controlHandlers[sensorId + "-isEnabled"] = [sensor](JsonVariant value) {
        sensor->isEnabled = value.as<bool>();
    };
    controlHandlers[sensorId + "-min"] = [sensor](JsonVariant value) {
        sensor->min = value.as<int>();
    };
    controlHandlers[sensorId + "-max"] = [sensor](JsonVariant value) {
        sensor->max = value.as<int>();
    };
    controlHandlers[sensorId + "-updateInterval"] = [sensor](JsonVariant value) {
        sensor->updateInterval = value.as<unsigned long>();
    };
    if (String(sensor->getType()) == "loadcell")
    {
        LoadCell *loadCell = static_cast<LoadCell *>(sensor);
        controlHandlers[sensorId + "-tare"] = [loadCell](JsonVariant) {
            loadCell->tare();
        };
        controlHandlers[sensorId + "-clearTare"] = [loadCell](JsonVariant) {
            loadCell->clearTare();
        };
    }

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
        controlHandlers[seqId + "-threshold"] = [sc](JsonVariant value) {
            sc->setThreshold(value.as<uint32_t>());
        };
        controlHandlers[seqId + "-thresholdRange"] = [sc](JsonVariant value) {
            sc->setThresholdRange(value.as<uint32_t>());
        };
        controlHandlers[seqId + "-subDiv"] = [sc](JsonVariant value) {
            sc->setSubDiv(value.as<uint8_t>());
        };
        controlHandlers[seqId + "-runTime"] = [sc](JsonVariant value) {
            sc->setRunTime(value.as<uint16_t>());
        };
        controlHandlers[seqId + "-trigEnable"] = [sc](JsonVariant value) {
            sc->setTrigEnable(value.as<bool>());
        };
        controlHandlers[seqId + "-manualActivation"] = [sc](JsonVariant value) {
            sc->setManualActivation(value.as<bool>());
        };
        controlHandlers[seqId + "-sensorTrigPriority"] = [sc](JsonVariant value) {
            sc->setPriorityLevel(value.as<int8_t>());
        };
        controlHandlers[seqId + "-mode"] = [sc, oh](JsonVariant value) {
            uint8_t modeValue = value.as<uint8_t>();
            sc->setMode(modeValue);
            oh->setMode();
        };
        controlHandlers[seqId + "-numSteps"] = [sc, oh](JsonVariant value) {
            uint8_t numStepsValue = value.as<uint8_t>();
            sc->setNumSteps(numStepsValue);
            oh->setSteps();
        };
        controlHandlers[seqId + "-randVal"] = [oh](JsonVariant value) {
            oh->setProbWeight(value.as<int>());
        };
        controlHandlers[seqId + "-pitches"] = [oh, sc](JsonVariant value) {
            JsonArray pitches = value.as<JsonArray>();
            for (size_t j = 0; j < pitches.size(); j++)
            {
                sc->setPitch(j, pitches[j]);
            }
            oh->setPitches();
        };
        controlHandlers[seqId + "-velocities"] = [oh, sc](JsonVariant value) {
            JsonArray velocities = value.as<JsonArray>();
            for (size_t j = 0; j < velocities.size(); j++)
            {
                sc->setVelocity(j, velocities[j]);
            }
            oh->setVelocities();
        };
        controlHandlers[seqId + "-globalActivation"] = [sc](JsonVariant value) {
            sc->startGlobalActivation(value.as<int8_t>());
        };
        controlHandlers[seqId + "-cooldown"] = [sc](JsonVariant value) {
            sc->setCooldown(value.as<bool>());
        };
        controlHandlers[seqId + "-enableTriggerRateProcessing"] = [sc](JsonVariant value) {
            sc->enableTriggerRateProcessing = value.as<bool>();
        };
        controlHandlers[seqId + "-enableCooldown"] = [sc](JsonVariant value) {
            sc->enableCooldown = value.as<bool>();
        };
        controlHandlers[seqId + "-enableAutoplay"] = [sc](JsonVariant value) {
            sc->enableAutoplay = value.as<bool>();
        };
        controlHandlers[seqId + "-toggleAnimation"] = [sc](JsonVariant value) {
            sc->toggleAnimation = value.as<bool>();
        };
        controlHandlers[seqId + "-triggerRateHighThreshold"] = [sc](JsonVariant value) {
            sc->triggerRateHighThreshold = value.as<float>();
        };
        controlHandlers[seqId + "-triggerCooldownInterval"] = [sc](JsonVariant value) {
            sc->triggerCooldownInterval = value.as<unsigned long>();
        };
        controlHandlers[seqId + "-triggerInactivityThreshold"] = [sc](JsonVariant value) {
            sc->triggerInactivityThreshold = value.as<unsigned long>();
        };
        controlHandlers[seqId + "-trigRatePriorityLevel"] = [sc](JsonVariant value) {
            sc->trigRatePriorityLevel = value.as<int8_t>();
        };
        controlHandlers[seqId + "-output"] = [oh, sc](JsonVariant value) {
            sc->setSequencerActive(false);
            auto it = outputMap.find(value.as<String>());
            oh->setOutput(it != outputMap.end() ? it->second : nullptr);
        };
    }
}

inline void setupAllControlHandlers()
{
    controlHandlers["requestDevices"] = [](JsonVariant) {
        JsonDocument doc;
        buildDeviceDescriptor(doc, deviceMac.c_str(), deviceName.c_str());
        sendMessage(doc);
    };
    controlHandlers["globalCooldown"] = [](JsonVariant value) {
        globalCooldown.store(value.as<bool>());
    };
    controlHandlers["device-enableTriggerRateProcessing"] = [](JsonVariant value) {
        deviceTriggerRate.enableTriggerRateProcessing = value.as<bool>();
    };
    controlHandlers["device-enableCooldown"] = [](JsonVariant value) {
        deviceTriggerRate.enableCooldown = value.as<bool>();
    };
    controlHandlers["device-enableAutoplay"] = [](JsonVariant value) {
        deviceTriggerRate.enableAutoplay = value.as<bool>();
    };
    controlHandlers["device-toggleAnimation"] = [](JsonVariant value) {
        deviceTriggerRate.toggleAnimation = value.as<bool>();
    };
    controlHandlers["device-triggerRateHighThreshold"] = [](JsonVariant value) {
        deviceTriggerRate.triggerRateHighThreshold = value.as<float>();
    };
    controlHandlers["device-triggerCooldownInterval"] = [](JsonVariant value) {
        deviceTriggerRate.triggerCooldownInterval = value.as<unsigned long>();
    };
    controlHandlers["device-triggerInactivityThreshold"] = [](JsonVariant value) {
        deviceTriggerRate.triggerInactivityThreshold = value.as<unsigned long>();
    };
    controlHandlers["device-trigRatePriorityLevel"] = [](JsonVariant value) {
        deviceTriggerRate.trigRatePriorityLevel = value.as<int8_t>();
    };
    controlHandlers["requestSettings"] = [](JsonVariant) {
        sendSettings();
        sendOutputSettings();
    };
    for (auto &pair : outputMap)
    {
        setupControlHandlersForOutput(pair.second);
    }
    for (auto &pair : sensorMap)
    {
        setupControlHandlersForSensor(pair.second);
    }
}

inline void processControlMessage(const char *jsonMessage)
{
    JsonDocument doc;
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
    }
    else
    {
        Serial.println("No control handler for: " + controlName);
    }
}

inline void processControlMessage(const String &jsonMessage)
{
    processControlMessage(jsonMessage.c_str());
}

inline void drainControlRing(ControlRing &ring)
{
    ControlMessage msg;
    while (ring.read(msg))
    {
        processControlMessage(msg.json);
    }
}

}

#endif
