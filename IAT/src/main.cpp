#include <Arduino.h>
#include "adc.h"
#include "metro.h"
#include "fmsynth.h"
#include "sequencerConfig.h"
#include "outputHandler.h"
#include "lidar.h"
#include "i2sSetup.h"
#include "controlHandler.h"
#include "stepper.h"

const gpio_num_t SENSOR_PIN = GPIO_NUM_36;
const gpio_num_t STEPPER_STEP_PIN = GPIO_NUM_14;
const gpio_num_t STEPPER_DIR_PIN = GPIO_NUM_15;
const int BPM = 120;

Adc sensor(SENSOR_PIN, "sensor1", 200);
Lidar lidar(GPIO_NUM_21, GPIO_NUM_27, 100, "lidar1");
Metro metro(BPM);
FmSynth synth("synth1");
Stepper stepper(STEPPER_STEP_PIN, STEPPER_DIR_PIN, "stepper1");
Sensor *sensors[] = {&sensor, &lidar};
const size_t sensorCount = sizeof(sensors) / sizeof(sensors[0]);
Output *outputs[] = {&synth, &stepper};
const size_t outputCount = sizeof(outputs) / sizeof(outputs[0]);

SequencerConfig &sensorConfigLow = sensor.lowSeq;
SequencerConfig &lidarConfigLow = lidar.lowSeq;
OutputHandler &sensorHandlerLow = sensor.lowHandler;
OutputHandler &lidarHandlerLow = lidar.lowHandler;

void setup()
{
  Serial.begin(115200);
  lidar.begin();
  lidar.isEnabled = true;
  sensor.isEnabled = true;
  i2sSetup();
  metro.start();
  initSensors(sensors, sensorCount);
  initOutputs(outputs, outputCount);
  setupAllControlHandlers();

  int stepCount = 8;

  // Sequence 1 triggered by sensor on ADC
  const uint16_t pitchVals1[8] = {60, 64, 67, 65, 60, 72, 48, 67};
  const float velVals1[8] = {60, 120, 40, 87, 127, 20, 30, 90};

  // Sequence 2 triggered by Lidar sensor
  const uint16_t pitchVals2[8] = {40, 44, 47, 45, 40, 62, 28, 47};
  const float velVals2[8] = {60, 120, 40, 87, 127, 20, 30, 90};

  // Set sequencer parameters for ADC sensor low threshold
  sensorConfigLow.setNumSteps(stepCount);
  sensorConfigLow.setRunTime(20);
  sensorConfigLow.setMode(0);
  sensorConfigLow.setTrigEnable(true);
  sensorConfigLow.setSubDiv(4);
  sensorConfigLow.setThresMode(1);
  sensorConfigLow.setThreshold(1000);
  sensorConfigLow.setThresholdRange(500);
  sensorConfigLow.setPriorityLevel(2);

  // Set sequencer parameters for Lidar sensor low threshold
  lidarConfigLow.setNumSteps(stepCount);
  lidarConfigLow.setRunTime(20);
  lidarConfigLow.setMode(2);
  lidarConfigLow.setTrigEnable(true);
  lidarConfigLow.setSubDiv(1);
  lidarConfigLow.setThresMode(1);
  lidarConfigLow.setThreshold(1000);
  lidarConfigLow.setThresholdRange(500);
  sensorConfigLow.setPriorityLevel(4);

  // Set sequencer step values
  for (int i = 0; i < stepCount; i++)
  {
    sensorConfigLow.setPitch(i, pitchVals1[i]);
    sensorConfigLow.setVelocity(i, velVals1[i]);
    lidarConfigLow.setPitch(i, pitchVals2[i]);
    lidarConfigLow.setVelocity(i, velVals2[i]);
  }

  // Initialise output handlers for dynamic output triggering
  sensorHandlerLow.setOutput(&synth);
  lidarHandlerLow.setOutput(&synth);
  sensorHandlerLow.setProbWeight(75); // sets sequencer trail off decay contour
  lidarHandlerLow.setProbWeight(60);  // sets sequencer trail off decay contour

  // set synth parameters
  synth.setModIndex(4);
  synth.setModRatio(3);
  synth.setCarEnv(0.3, 0.3, 0.5, 0.3);
  synth.setModEnv(0.4, 0.1, 0.3, 0.1);
}

void loop()
{
  // read sensor values
  sensor.read();
  uint16_t sensorReading = sensor.getReading();
  Serial.print("ADC Sensor reading:");
  Serial.println(sensorReading);

  lidar.read();
  uint16_t lidarReading = lidar.getReading();
  Serial.print("Lidar Reading");
  Serial.println(lidarReading);

  // run metronome and synth
  bool click = metro.getClick();
  const double *buffer = nullptr;
  if (synth.getSeqConfig()->getSequencerActive())
  {
    synth.setBpm(BPM);
    buffer = synth.process(click);
  }
  i2sAudioOut(buffer, buffer);

  Serial.print("Sequencer Active:");
  Serial.println(synth.getSeqConfig()->getSequencerActive());

  // Enable dynamic troggering of sequencer from sensor thresholds
  sensorConfigLow.setTrigger();
  sensorHandlerLow.enableOutputTriggering();
  sensorHandlerLow.renderStepProbability();
  lidarConfigLow.setTrigger();
  lidarHandlerLow.enableOutputTriggering();
  lidarHandlerLow.renderStepProbability();
}
