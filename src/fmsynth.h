#ifndef FMSYNTH_H
#define FMSYNTH_H

#include <atomic>
#include "oscillator.h"
#include "adsr.h"
#include "metro.h"
#include "sequencer.h"
#include "output.h"
#include <SD.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "utils.h"

class FmSynth_t : public Output_t
{
  Oscillator carrier;
  Oscillator modulator;
  ADSR carAdsr;
  ADSR modAdsr;
  float freq;
  float modIndex;
  float modRatio;
  float level;
  float velocity;
  const char *id;

public:
  FmSynth_t(SequencerConfig_t *defaultConfig, const char *idNum)
      : Output_t(120, 4, defaultConfig), carrier(0.1, 440),
        modulator(0.1, 440),
        carAdsr(1),
        modAdsr(1),
        freq(220.0f),
        modIndex(0),
        modRatio(1),
        level(0.5f),
        velocity(0.5f),
        id(idNum)
  {
    carrier.initWaveTable();
    carAdsr.setParams(0.2 * noteDuration, 0.2 * noteDuration, 0.7, 0.2 * noteDuration);
    modAdsr.setParams(0.3, 0.5, 0.7, 0.1);
  }

  const double *process(bool click)
  {
    if (!seqConfig)
    {
      return nullptr;
    }
    seqTrig = getSubDivTrig(click);

    bool gate = sequencer.updateStepAndGate(seqTrig, seqConfig->sequencerActive);
    freq = midiNoteToFrequency(sequencer.getCurrentPitch()) + 0.5;
    if (freq < 4.0)
    {
      freq = 4.0;
    }
    velocity = sequencer.getCurrentVelocity();
    float modVal = modIndex * 2 * M_PI;

    return carrier.process(carAdsr.process(gate, velocity), level, freq,
                           modulator.process(modAdsr.process(gate, 1), modVal, freq * modRatio));
  }

  void saveConfigSD()
  {
    StaticJsonDocument<1024> doc;
    auto configSection = doc["output"][id];
    configSection["modIndex"] = modIndex;
    configSection["level"] = level;
    configSection["modRatio"] = modRatio;
    configSection["ampAttack"] = carAdsr.m_attack;
    configSection["ampDecay"] = carAdsr.m_decay;
    configSection["ampSustain"] = carAdsr.m_sustainLevel;
    configSection["ampRelease"] = carAdsr.m_release;
    configSection["modAttack"] = modAdsr.m_attack;
    configSection["modDecay"] = modAdsr.m_decay;
    configSection["modSustain"] = modAdsr.m_sustainLevel;
    configSection["modRelease"] = modAdsr.m_release;

    File configFile = SD.open("/outputConfig.json", FILE_WRITE);
    if (!configFile)
    {
      Serial.println("Failed to open output SD config file for writing");
      return;
    }

    if (serializeJson(doc, configFile) == 0)
    {
      Serial.println("Failed to write output config to SD file");
    }

    configFile.close();
  }

  void saveConfigFS()
  {
    StaticJsonDocument<1024> doc;
    auto configSection = doc["output"][id];
    configSection["modIndex"] = modIndex;
    configSection["level"] = level;
    configSection["modRatio"] = modRatio;
    configSection["ampAttack"] = carAdsr.m_attack;
    configSection["ampDecay"] = carAdsr.m_decay;
    configSection["ampSustain"] = carAdsr.m_sustainLevel;
    configSection["ampRelease"] = carAdsr.m_release;
    configSection["modAttack"] = modAdsr.m_attack;
    configSection["modDecay"] = modAdsr.m_decay;
    configSection["modSustain"] = modAdsr.m_sustainLevel;
    configSection["modRelease"] = modAdsr.m_release;

    File configFile = LittleFS.open("/outputConfig.json", FILE_WRITE);
    if (!configFile)
    {
      Serial.println("Failed to open output FS config file for writing");
      return;
    }

    if (serializeJson(doc, configFile) == 0)
    {
      Serial.println("Failed to write output config to FS file");
    }

    configFile.close();
  }

  void loadConfigSD()
  {
    File configFile = SD.open((char *)synthConfigPath.c_str(), FILE_READ);
    if (!configFile)
    {
      Serial.println("Failed to open SD config file for reading: FM Synth");
      return;
    }
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error)
    {
      Serial.println("Failed to read file, using default configuration: FM Synth");
    }
    else
    {
      auto configSection = doc["output"][id];
      modIndex = configSection["modIndex"] | modIndex;
      modRatio = configSection["modRatio"] | modRatio;
      level = configSection["level"] | level;
      carAdsr.m_attack = configSection["ampAttack"] | carAdsr.m_attack;
      carAdsr.m_decay = configSection["ampDecay"] | carAdsr.m_decay;
      carAdsr.m_sustainLevel = configSection["ampSustain"] | carAdsr.m_sustainLevel;
      carAdsr.m_release = configSection["ampRelease"] | carAdsr.m_release;
      modAdsr.m_attack = configSection["modAttack"] | modAdsr.m_attack;
      modAdsr.m_decay = configSection["modDecay"] | modAdsr.m_attack;
      modAdsr.m_sustainLevel = configSection["modSustain"] | modAdsr.m_attack;
      modAdsr.m_release = configSection["modRelease"] | modAdsr.m_attack;

      carAdsr.setParams(carAdsr.m_attack, carAdsr.m_decay, carAdsr.m_sustainLevel, carAdsr.m_release);
      modAdsr.setParams(modAdsr.m_attack, modAdsr.m_decay, modAdsr.m_sustainLevel, modAdsr.m_release);

      configFile.close();
    }
  }

  void loadConfigFS()
  {
    File configFile = LittleFS.open((char *)synthConfigPath.c_str(), FILE_READ);
    if (!configFile)
    {
      Serial.println("Failed to open FS config file for reading: FM Synth");
      return;
    }
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error)
    {
      Serial.println("Failed to read file, using default configuration: FM Synth");
    }
    else
    {
      auto configSection = doc["output"][id];
      modIndex = configSection["modIndex"] | modIndex;
      modRatio = configSection["modRatio"] | modRatio;
      level = configSection["level"] | level;
      carAdsr.m_attack = configSection["ampAttack"] | carAdsr.m_attack;
      carAdsr.m_decay = configSection["ampDecay"] | carAdsr.m_decay;
      carAdsr.m_sustainLevel = configSection["ampSustain"] | carAdsr.m_sustainLevel;
      carAdsr.m_release = configSection["ampRelease"] | carAdsr.m_release;
      modAdsr.m_attack = configSection["modAttack"] | modAdsr.m_attack;
      modAdsr.m_decay = configSection["modDecay"] | modAdsr.m_attack;
      modAdsr.m_sustainLevel = configSection["modSustain"] | modAdsr.m_attack;
      modAdsr.m_release = configSection["modRelease"] | modAdsr.m_attack;

      carAdsr.setParams(carAdsr.m_attack, carAdsr.m_decay, carAdsr.m_sustainLevel, carAdsr.m_release);
      modAdsr.setParams(modAdsr.m_attack, modAdsr.m_decay, modAdsr.m_sustainLevel, modAdsr.m_release);

      configFile.close();
    }
  }
};

#endif