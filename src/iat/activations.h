#ifndef ACTIVATIONS_H
#define ACTIVATIONS_H

#include <time.h>
#include <LittleFS.h>
#include "controlHandler.h"
#include "bme.h"

namespace iat
{

inline JsonDocument globalActivations;
inline BME68xSensor *weatherSensor = nullptr;
inline unsigned long weatherDebounceInterval = 1800000;
inline const char *globalActivationsPath = "/activations.json";

inline std::map<String, bool> lastTriggeredStates;
inline std::map<String, String> lastTriggeredTimes;
inline std::map<String, unsigned long> lastDebounceTimes;

inline bool weatherValid = false;
inline float weatherTemperature = 0;
inline float weatherHumidity = 0;
inline float weatherPressure = 0;
inline float weatherPressureRate = 0;
inline unsigned long lastPressureTime = 0;

inline NoiseSynth *activeNoise = nullptr;
inline unsigned long noiseOffTime = 0;
inline unsigned long lastActivationProcess = 0;

inline float toNumber(JsonVariantConst v)
{
    return v.is<const char *>() ? atof(v.as<const char *>()) : v.as<float>();
}

inline void saveGlobalActivations()
{
    File file = LittleFS.open(globalActivationsPath, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open activations file for writing");
        return;
    }
    serializeJson(globalActivations, file);
    file.close();
}

inline void loadGlobalActivations()
{
    File file = LittleFS.open(globalActivationsPath, FILE_READ);
    if (!file || deserializeJson(globalActivations, file))
    {
        globalActivations.to<JsonObject>();
    }
    if (file)
    {
        file.close();
    }
}

inline void sendGlobalActivations()
{
    JsonDocument doc;
    doc["type"] = "globalActivationSettings";
    doc["data"] = globalActivations.as<JsonObject>();
    sendMessage(doc);
}

inline void updateWeather()
{
    if (!weatherSensor || !weatherSensor->update())
    {
        return;
    }
    unsigned long currentTime = millis();
    float pressure = weatherSensor->pressure / 100.0f;
    if (weatherValid && currentTime > lastPressureTime)
    {
        weatherPressureRate = fabsf(pressure - weatherPressure) / ((currentTime - lastPressureTime) / 1000.0f);
    }
    weatherTemperature = weatherSensor->temperature;
    weatherHumidity = weatherSensor->humidity;
    weatherPressure = pressure;
    lastPressureTime = currentTime;
    weatherValid = true;

    JsonDocument doc;
    doc["type"] = "weatherUpdate";
    JsonObject data = doc["data"].to<JsonObject>();
    data["temperature"] = weatherTemperature;
    data["humidity"] = weatherHumidity;
    data["pressure"] = weatherPressure;
    data["rateOfChangePressure"] = weatherPressureRate;
    sendMessage(doc);
}

inline bool getWeatherParameter(const String &parameter, float &value)
{
    if (!weatherValid)
    {
        return false;
    }
    if (parameter == "temperature")
        value = weatherTemperature;
    else if (parameter == "humidity")
        value = weatherHumidity;
    else if (parameter == "pressure")
        value = weatherPressure;
    else if (parameter == "rateOfChangePressure")
        value = weatherPressureRate;
    else
        return false;
    return true;
}

inline void processGlobalActivations()
{
    unsigned long nowTimestamp = millis();

    if (activeNoise && nowTimestamp >= noiseOffTime)
    {
        activeNoise->triggerEnabled = false;
        activeNoise = nullptr;
    }

    updateWeather();

    if (nowTimestamp - lastActivationProcess < 1000)
    {
        return;
    }
    lastActivationProcess = nowTimestamp;

    struct tm timeInfo;
    bool haveTime = getLocalTime(&timeInfo, 0);
    char currentTime[6] = "";
    if (haveTime)
    {
        snprintf(currentTime, sizeof(currentTime), "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);
    }

    for (JsonPair kv : globalActivations.as<JsonObject>())
    {
        String id = kv.key().c_str();
        JsonObject activation = kv.value().as<JsonObject>();
        if (!(activation["enabled"] | true))
        {
            continue;
        }

        bool triggered = false;
        String inputType = activation["input"]["type"] | "";

        if (inputType == "SensorThreshold")
        {
            for (JsonObject threshold : activation["input"]["sensorThreshold"]["thresholds"].as<JsonArray>())
            {
                String device = threshold["device"] | "";
                if (device.length() && device != deviceMac)
                {
                    continue;
                }
                SequencerConfig *sc = findSequencerConfig(threshold["threshold"] | "");
                if (sc && sc->getTrigEnable() && sc->getIsWithinRange())
                {
                    triggered = true;
                    break;
                }
            }
        }
        else if (inputType == "TimeOfDay")
        {
            int hour = (int)toNumber(activation["input"]["timeOfDay"]["hour"]);
            int minute = (int)toNumber(activation["input"]["timeOfDay"]["minute"]);
            if (haveTime && timeInfo.tm_hour == hour && timeInfo.tm_min == minute &&
                lastTriggeredTimes[id] != currentTime)
            {
                triggered = true;
                lastTriggeredTimes[id] = currentTime;
            }
        }
        else if (inputType == "Weather")
        {
            String parameter = activation["input"]["weather"]["parameter"] | "";
            String condition = activation["input"]["weather"]["condition"] | "";
            float value = toNumber(activation["input"]["weather"]["value"]);
            float paramValue;
            if (getWeatherParameter(parameter, paramValue))
            {
                bool conditionMet = (condition == "above" && paramValue > value) ||
                                    (condition == "below" && paramValue < value) ||
                                    (condition == "equals" && fabsf(paramValue - value) < 0.05f);
                auto last = lastDebounceTimes.find(id);
                if (conditionMet && (last == lastDebounceTimes.end() || nowTimestamp - last->second > weatherDebounceInterval))
                {
                    triggered = true;
                    lastDebounceTimes[id] = nowTimestamp;
                }
            }
        }

        if (lastTriggeredStates[id] != triggered)
        {
            lastTriggeredStates[id] = triggered;
            JsonDocument doc;
            doc["type"] = "globalActivationTriggered";
            JsonObject data = doc["data"].to<JsonObject>();
            data["id"] = id;
            data["triggered"] = triggered;
            if (triggered && haveTime)
            {
                data["lastTriggeredTime"] = currentTime;
            }
            else
            {
                data["lastTriggeredTime"] = nullptr;
            }
            sendMessage(doc);
        }

        if (!triggered)
        {
            continue;
        }

        String outputType = activation["output"]["type"] | "";
        int8_t priorityLevel = (int8_t)toNumber(activation["priorityLevel"]);

        if (outputType == "EnabledSequencers")
        {
            for (JsonObject seq : activation["output"]["sequencers"].as<JsonArray>())
            {
                String device = seq["device"] | "";
                if (device.length() && device != deviceMac)
                {
                    continue;
                }
                SequencerConfig *sc = findSequencerConfig(seq["threshold"] | "");
                if (sc)
                {
                    sc->startGlobalActivation(priorityLevel);
                }
            }
        }
        else if (outputType == "Noise")
        {
            String noiseDevice = activation["output"]["noiseDevice"] | "";
            if (noiseDevice.length() && noiseDevice != deviceMac)
            {
                continue;
            }
            float noiseLength = activation["output"]["noiseLength"].isNull() ? 1.0f : toNumber(activation["output"]["noiseLength"]);
            for (auto &pair : outputMap)
            {
                if (String(pair.second->getType()) == "noiseSynth")
                {
                    activeNoise = static_cast<NoiseSynth *>(pair.second);
                    activeNoise->triggerEnabled = true;
                    noiseOffTime = nowTimestamp + (unsigned long)(noiseLength * 1000);
                    break;
                }
            }
        }
    }
}

inline void setupGlobalActivations(const char *timezone, BME68xSensor *bme = nullptr)
{
    weatherSensor = bme;
    configTzTime(timezone, "pool.ntp.org", "time.nist.gov");
    loadGlobalActivations();

    controlHandlers["globalActivation"] = [](JsonVariant value) {
        String action = value["action"] | "";
        JsonObject activation = value["activation"].as<JsonObject>();
        if (action == "request")
        {
            sendGlobalActivations();
            return;
        }
        String id = activation["id"] | "";
        if (id.length() == 0)
        {
            return;
        }
        if (action == "add" || action == "update")
        {
            globalActivations[id] = activation;
            saveGlobalActivations();
        }
        else if (action == "delete")
        {
            globalActivations.remove(id);
            lastTriggeredStates.erase(id);
            lastTriggeredTimes.erase(id);
            lastDebounceTimes.erase(id);
            saveGlobalActivations();
        }
    };
}

}

#endif
