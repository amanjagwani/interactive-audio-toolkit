import React, { useState } from "react";
import Step from "./Step";
import { Button, Container, Form, Row, Col } from "react-bootstrap";
import { useWebSocket } from "./WebSocketContext";
import { useEffect } from "react";
import SynthControls from "./SynthControls";
import useStore from "../store";
import { useLocation, useNavigationType } from "react-router-dom";
function Sequencer({ seqId, thresholdType, macAddress, context, outputId }) {
  const { sendControlChange, sendControlChangeToServer, messages } =
    useWebSocket();

  const {
    sequencers,
    setSequencerState,
    sensors,
    sendStepUpdates,
    updateStep,
    setPlotting,
    setThreshold,
    sensorRanges,
    setSensorState,
    devices,
  } = useStore();
  const deviceOutputs = devices[macAddress]?.outputs || [];

  const getSensorRange = (sensorId) => {
    const defaultRange = { min: 0, max: 100, step: 1 };
    return sensorRanges[sensorId.toUpperCase()] || defaultRange;
  };
  const { min, max, step } = getSensorRange(seqId.split("-")[0].toUpperCase());

  const defaultStep = { pitch: 0, velocity: 0 };
  const initialSteps = Array.from({ length: 32 }, () => ({
    ...defaultStep,
  }));
  const location = useLocation();
  const navigationType = useNavigationType();

  const {
    subDiv = 4,
    output = "none",
    output2 = "none",
    mode = 0,
    numSteps = 32,
    runningTime = 20,
    priorityLevel = 2,
    steps = initialSteps,
    isPlaying = false,
    currentStep = 0,
    trigEnabled = false,
    randVal = 60,
    thresholdRange,
    isAnimating,
    triggerRate = 0,
    triggerCooldownInterval = 5,
    triggerInactivityThreshold = 10,
    triggerRateHighThreshold = 10,
    trigRatePriorityLevel = 2,
    cooldown = false,
    autoplay = false,
    enableTriggerRateProcessing = false,
    enableAutoplay = false,
    enableCooldown = false,
  } = sequencers[seqId] || {};

  const sensorState = sensors[seqId.split("-")[0]] || {};

  const {
    isPlotting,
    lowThres,
    midThres,
    highThres,
    minVal,
    maxVal,
    sensorValue,
  } = sensorState;
  const [localSensorValue, setLocalSensorValue] = useState(sensorValue);

  const adjustedThresholdType = thresholdType.endsWith("2")
    ? thresholdType.slice(0, -1) // Remove the last character if it's "2"
    : thresholdType;
  const threshold = sensorState[`${adjustedThresholdType}Thres`] || 0;
  const randomSteps = Array.from({ length: 32 }, () => ({
    pitch: Math.floor(Math.random() * 40 + 30),
    velocity: parseFloat(Math.random().toFixed(2)),
  }));

  useEffect(() => {
    const defaults = {};
    let updateRequired = false;

    if (subDiv === undefined) {
      defaults.subDiv = 4;
      updateRequired = true;
    }

    if (output === undefined) {
      defaults.output = "none";
      updateRequired = true;
    }

    if (output2 === undefined) {
      defaults.output2 = "none";
      updateRequired = true;
    }

    if (mode === undefined) {
      defaults.mode = 0;
      updateRequired = true;
    }

    if (numSteps === undefined) {
      defaults.numSteps = 16;
      updateRequired = true;
    }

    if (runningTime === undefined) {
      defaults.runningTime = 20;
      updateRequired = true;
    }

    if (priorityLevel === undefined) {
      defaults.priorityLevel = 2;
      updateRequired = true;
    }

    if (isPlaying === undefined) {
      defaults.isPlaying = false;
      updateRequired = true;
    }

    if (steps === undefined) {
      defaults.steps = randomSteps;
      updateRequired = true;
    }

    if (currentStep === undefined) {
      defaults.currentStep = 0;
      updateRequired = true;
    }

    if (trigEnabled === undefined) {
      defaults.trigEnabled = false;
      updateRequired = true;
    }

    if (updateRequired) {
      setSequencerState(seqId, defaults);
    }
  }, [seqId, setSequencerState]);

  const handleOutput = (value) => {
    const hasOutput = value !== "none";
    if (hasOutput && output && output !== "none") {
      sendControlChange(macAddress, `${seqId}-trigEnable`, false);
    }
    if (hasOutput) {
      sendControlChange(macAddress, `${seqId}-trigEnable`, true);
    }
    sendControlChange(macAddress, `${seqId}-output`, value);
    setSequencerState(seqId, {
      output: value,
      trigEnabled: hasOutput,
    });
    sendControlChange(macAddress, `${seqId}-threshold`, threshold);
    sendControlChange(macAddress, `${seqId}-thresholdRange`, thresholdRange);
  };

  const allOutputs = [
    ...deviceOutputs.map((o) => ({ label: o.id, value: o.id })),
    { label: "No Output", value: "none" },
  ];

  const renderOptions = (excludeValue) =>
    allOutputs
      .filter(({ value }) => value !== excludeValue)
      .map(({ label, value }) => (
        <option key={value} value={value}>
          {label}
        </option>
      ));

  const handlePlay = () => {
    const newIsPlayingState = !isPlaying;
    setSequencerState(seqId, { isPlaying: newIsPlayingState });
    sendControlChange(macAddress, `${seqId}-manualActivation`, newIsPlayingState);
  };

  const handleTrig = () => {
    const newTrig = !trigEnabled;
    setSequencerState(seqId, { trigEnabled: newTrig });
    sendControlChange(macAddress, `${seqId}-trigEnable`, newTrig);
    sendControlChange(macAddress, `${seqId}-threshold`, threshold);
    sendControlChange(macAddress, `${seqId}-thresholdRange`, thresholdRange);
  };
  const handleSave = () => {
    sendControlChange(macAddress, "saveSettings", 1);
  };

  const handleSubDiv = (subDiv) => {
    setSequencerState(seqId, { subDiv: subDiv });
    sendControlChange(macAddress, `${seqId}-subDiv`, subDiv);
  };

  const handleRandVal = (val) => {
    setSequencerState(seqId, { randVal: val });
    sendControlChange(macAddress, `${seqId}-randVal`, val);
  };

  const handleRunningTime = (runTime) => {
    setSequencerState(seqId, { runningTime: runTime });
    sendControlChange(macAddress, `${seqId}-runTime`, runTime);
  };

  const handlePriorityLevel = (priorityLevel) => {
    setSequencerState(seqId, { priorityLevel: priorityLevel });
    sendControlChange(macAddress, `${seqId}-sensorTrigPriority`, priorityLevel);
  };
  const handleStepUpdates = () => {
    if (macAddress) {
      sendStepUpdates(seqId, sendControlChange, macAddress);
    } else {
      console.error("macAddress is not defined in handleStepUpdates");
    }
  };

  const handleStepVals = (index, type, value) => {
    updateStep(seqId, index, type, value);
    handleStepUpdates();
  };

  const handleSequencerMode = (value) => {
    setSequencerState(seqId, { mode: value });
    sendControlChange(macAddress, `${seqId}-mode`, value);
  };

  const handleNumSteps = (value) => {
    setSequencerState(seqId, { numSteps: value });
    sendControlChange(macAddress, `${seqId}-numSteps`, value);
  };

  const handleTriggerRateHighThreshold = (value) => {
    setSequencerState(seqId, { triggerRateHighThreshold: value });
    sendControlChangeToServer(
      macAddress,
      seqId,
      "triggerRateHighThreshold",
      value
    );
  };

  const handleTriggerCooldownInterval = (value) => {
    setSequencerState(seqId, { triggerCooldownInterval: value });
    sendControlChangeToServer(
      macAddress,
      seqId,
      "triggerCooldownInterval",
      value * 1000
    );
  };

  const handleTriggerInactivityThreshold = (value) => {
    setSequencerState(seqId, { triggerInactivityThreshold: value });
    sendControlChangeToServer(
      macAddress,
      seqId,
      "triggerInactivityThreshold",
      value * 1000
    );
  };

  const handleEnableTriggerRateProcessing = (value) => {
    setSequencerState(seqId, { enableTriggerRateProcessing: value });
    sendControlChangeToServer(
      macAddress,
      seqId,
      "enableTriggerRateProcessing",
      value
    );
  };

  const handleEnableCooldown = (value) => {
    setSequencerState(seqId, { enableCooldown: value });
    sendControlChangeToServer(macAddress, seqId, "enableCooldown", value);
  };

  const handleEnableAutoplay = (value) => {
    setSequencerState(seqId, { enableAutoplay: value });
    sendControlChangeToServer(macAddress, seqId, "enableAutoplay", value);
  };

  const handleTrigRatePriorityLevel = (value) => {
    setSequencerState(seqId, { trigRatePriorityLevel: value });
    sendControlChangeToServer(macAddress, seqId, "trigRatePriorityLevel", value);
  };

  useEffect(() => {
    if (messages.length > 0) {
      const lastMessage = messages[messages.length - 1];
      if (lastMessage.macAddress === macAddress) {
        if (
          context === "sensorThreshold" &&
          lastMessage[`${seqId}-currentStep`] !== undefined
        ) {
          setSequencerState(seqId, {
            currentStep: lastMessage[`${seqId}-currentStep`],
          });
        }

        if (
          context === "output" &&
          lastMessage[`${outputId}CurrentStep`] !== undefined
        ) {
          setSequencerState(seqId, {
            currentStep: lastMessage[`${outputId}CurrentStep`],
          });
        }

        if (lastMessage["seqId"] == seqId) {
          if (lastMessage["triggerRate"] !== undefined) {
            setSequencerState(seqId, {
              triggerRate: lastMessage["triggerRate"],
            });
          }
          if (lastMessage["cooldown"] !== undefined) {
            setSequencerState(seqId, {
              cooldown: lastMessage["cooldown"],
            });
          }
          if (lastMessage["autoplay"] !== undefined) {
            setSequencerState(seqId, {
              autoplay: lastMessage["autoplay"],
            });
          }
        }

        if (lastMessage[`${seqId}IsPlaying`] !== undefined) {
          setSequencerState(seqId, {
            isPlaying: lastMessage[`${seqId}IsPlaying`],
          });
        }

        const baseSensorId = seqId.split("-")[0];
        if (lastMessage[baseSensorId] !== undefined) {
          setLocalSensorValue(lastMessage[baseSensorId]);
        }

        if (
          lastMessage.sequencerSettings &&
          lastMessage.sequencerSettings[seqId]
        ) {
          const settings = lastMessage.sequencerSettings[seqId];
          const steps = settings.pitches.map((pitch, index) => ({
            pitch,
            velocity: settings.velocities[index],
          }));
          setSequencerState(seqId, {
            runningTime: settings.runTime,
            threshold: settings.threshold,
            trigEnabled: settings.trigEnable,
            subDiv: settings.subDiv,
            sensorInput: settings.sensorInput,
            mode: settings.mode,
            numSteps: settings.numSteps,
            output: settings.output,
            steps: steps,
            randVal: settings.randVal,
            triggerCooldownInterval: settings.triggerCooldownInterval / 1000,
            triggerInactivityThreshold:
              settings.triggerInactivityThreshold / 1000,
            triggerRateHighThreshold: settings.triggerRateHighThreshold,
            priorityLevel: settings.sensorTrigPriority,
            isPlaying: settings.manualActivation,
            enableTriggerRateProcessing: settings.enableTriggerRateProcessing,
            enableAutoplay: settings.enableAutoplay,
            trigRatePriorityLevel: settings.trigRatePriorityLevel,
            enableCooldown: settings.enableCooldown,
          });

          setThreshold(
            seqId.split("-")[0],
            thresholdType,
            parseInt(settings.threshold)
          );
        }
      }
    }
  }, [messages]);

  useEffect(() => {
    sendControlChange(macAddress, `requestSettings`, 1);
  }, [seqId, macAddress]);

  const shouldRenderSynthControls = () => {
    const selected = context === "output" ? outputId : output;
    return deviceOutputs.some((o) => o.id === selected && o.type === "fmSynth");
  };

  const renderSteps = () => {
    const effectiveSteps =
      steps && steps.length > 0
        ? steps.slice(0, numSteps)
        : Array.from({ length: numSteps }, (_, index) => ({
            pitch: 0,
            velocity: 1,
          }));

    let rows = [];
    for (let row = 0; row < Math.ceil(numSteps / 8); row++) {
      let cols = [];
      for (let col = 0; col < 8; col++) {
        const index = row * 8 + col;
        if (index < effectiveSteps.length) {
          cols.push(
            <Col key={index}>
              <Step
                stepIndex={index}
                stepData={effectiveSteps[index]}
                currentStep={currentStep}
                onStepChange={handleStepVals}
                macAddress={macAddress}
              />
            </Col>
          );
        }
      }
      rows.push(
        <Row key={row} className="mb-2 justify-content-center">
          {cols}
        </Row>
      );
    }
    return rows;
  };

  const togglePlotting = () => {
    const baseSensorId = seqId.split("-")[0];
    const newPlottingState = !isPlotting;
    setPlotting(baseSensorId, newPlottingState);
    sendControlChange(macAddress, `${baseSensorId}-plot`, newPlottingState);
  };

  const toggleAnimation = () => {
    const newIsAnimating = !isAnimating;
    setSequencerState(seqId, { isAnimating: newIsAnimating });
    sendControlChange(macAddress, `${seqId}-toggleAnimation`, newIsAnimating);
  };

  useEffect(() => {
    const handleBeforeUnload = () => {
      const baseSensorId = seqId.split("-")[0];
      if (isPlotting) {
        setPlotting(baseSensorId, false);
        sendControlChange(macAddress, `${baseSensorId}-plot`, false);
      }

      if (isAnimating) {
        setSequencerState(seqId, { isAnimating: false });
        sendControlChange(macAddress, `${seqId}-toggleAnimation`, false);
      }

      if (isPlaying) {
        setSequencerState(seqId, { isPlaying: false });
        sendControlChange(macAddress, `${seqId}-manualActivation`, false);
      }
    };

    const handlePageHide = () => {
      handleBeforeUnload();
    };

    window.addEventListener("beforeunload", handleBeforeUnload);
    window.addEventListener("pagehide", handlePageHide);

    return () => {
      handleBeforeUnload();
      window.removeEventListener("beforeunload", handleBeforeUnload);
      window.removeEventListener("pagehide", handlePageHide);
    };
  }, [location.pathname, navigationType]);

  return (
    <Container className="my-4">
      {context === "sensorThreshold" && (
        <>
          <h2 className="mb-3">Sensor Threshold Sequencer</h2>
          <h3 className="mb-3">Current Threshold: {threshold}</h3>
          <h3 className="mb-3">Current Sensor Value: {localSensorValue}</h3>
        </>
      )}

      {context === "output" && (
        <>
          <h3>
            Output:{" "}
            {allOutputs.find((output) => output.value === outputId)?.label ||
              "Unknown Output"}
          </h3>
        </>
      )}

      <Row className="mb-3">
        <Col xs={12} md={4}>
          <Form.Group>
            {context === "sensorThreshold" && (
              <Row>
                <Button
                  className="mb-3"
                  variant={isPlotting ? "danger" : "success"}
                  onClick={togglePlotting}
                >
                  {isPlotting ? "Stop Plotting" : "Start Plotting"}
                </Button>
              </Row>
            )}
            <Row>
              <Button
                className="mb-3"
                variant={isAnimating ? "danger" : "success"}
                onClick={toggleAnimation}
              >
                {isAnimating ? "Stop Animating" : "Start Animating"}
              </Button>
            </Row>

            {context === "sensorThreshold" && (
              <>
                <Form.Group>
                  <Row className="mb-3">
                    <Col className="w-100" xs={12} md={6}>
                      <Form.Select
                        className="w-100"
                        aria-label="Output selection"
                        onChange={(e) => handleOutput(e.target.value)}
                        value={output}
                      >
                        <option>Select Output</option>
                        {renderOptions(output2)}
                      </Form.Select>
                    </Col>
                  </Row>
                </Form.Group>
              </>
            )}
          </Form.Group>
          <Form.Select
            aria-label="Sequencer Mode"
            onChange={(e) => handleSequencerMode(e.target.value)}
            className="mb-3"
            value={mode}
          >
            <option>Select Mode</option>
            <option value="0">Forward</option>
            <option value="1">Backward</option>
            <option value="2">Random</option>
          </Form.Select>
        </Col>
        <Col xs={12} md={{ span: 4, offset: 1 }}>
          <Form.Label>Subdivision: {subDiv}</Form.Label>
          <Form.Range
            aria-label={`Subdivision: ${subDiv}`}
            min={1}
            max={32}
            value={subDiv}
            onChange={(e) => handleSubDiv(e.target.value)}
          />

          <Form.Label>Number of Steps: {numSteps}</Form.Label>
          <Form.Range
            aria-label={`Number of Steps: ${numSteps}`}
            min={1}
            max={32}
            value={numSteps}
            onChange={(e) => handleNumSteps(e.target.value)}
          />

          <Form.Label>Sequence Running Time(seconds): {runningTime}</Form.Label>
          <Form.Range
            aria-label={`Running Time: ${runningTime}`}
            min={1}
            max={300}
            value={runningTime}
            onChange={(e) => handleRunningTime(e.target.value)}
          />
          <Form.Label>Random Skew: {randVal}</Form.Label>
          <Form.Range
            aria-label={`Random Skew: ${randVal}`}
            min={1}
            max={100}
            value={randVal}
            onChange={(e) => handleRandVal(e.target.value)}
          />
        </Col>
      </Row>
      <Row>{renderSteps()}</Row>
      <Row>
        <Col xs={12} className="mb-3">
          {context === "sensorThreshold" && (
            <Button
              variant={isPlaying ? "danger" : "success"}
              onClick={handlePlay}
              disabled={trigEnabled}
              style={{
                opacity: trigEnabled ? 0.5 : 1,
                cursor: trigEnabled ? "not-allowed" : "pointer",
              }}
            >
              {isPlaying
                ? `Turn Off ${output} Sequencer`
                : `Turn On ${output} Sequencer`}
            </Button>
          )}
          {context === "output" && (
            <Button
              variant={isPlaying ? "danger" : "success"}
              onClick={handlePlay}
              disabled={trigEnabled}
              style={{
                opacity: trigEnabled ? 0.5 : 1,
                cursor: trigEnabled ? "not-allowed" : "pointer",
              }}
            >
              {isPlaying ? `Turn Off Sequencer` : `Turn On Sequencer`}
            </Button>
          )}
        </Col>
      </Row>

      {context === "sensorThreshold" && (
        <Row>
          <Col xs={12} className="mb-3">
            <Button
              variant={trigEnabled ? "danger" : "success"}
              onClick={handleTrig}
            >
              {trigEnabled
                ? `Disable ${output} Trigger`
                : `Enable ${output} Trigger`}
            </Button>
          </Col>
          <Col xs={12} md={4} className="mb-3">
            <Form.Label>Trigger Priority Level: {priorityLevel}</Form.Label>
            <Form.Range
              aria-label={`Trigger Priority Level: ${priorityLevel}`}
              min={1}
              max={10}
              value={priorityLevel}
              onChange={(e) => handlePriorityLevel(e.target.value)}
            />
          </Col>
        </Row>
      )}

      {context === "output" && (
        <Row>
          <Col xs={12} className="mb-3">
            <Button
              variant={trigEnabled ? "danger" : "success"}
              onClick={handleTrig}
            >
              {trigEnabled
                ? `Disable Sequencer Activation`
                : `Enable Sequencer Activation`}
            </Button>
          </Col>
        </Row>
      )}

      {context === "sensorThreshold" && (
        <div className="p-4 border rounded shadow-sm bg-light">
          <h4 className="text-center mb-4">Sequencer Trigger Rate Settings</h4>

          <Row className="text-center mb-3">
            <Col>
              <Button
                variant={enableTriggerRateProcessing ? "success" : "danger"}
                onClick={() =>
                  handleEnableTriggerRateProcessing(
                    !enableTriggerRateProcessing
                  )
                }
              >
                {enableTriggerRateProcessing
                  ? "Disable Trigger Rate Processing"
                  : "Enable Trigger Rate Processing"}
              </Button>
            </Col>
          </Row>

          {enableTriggerRateProcessing && (
            <>
              <Row className="text-center mb-3">
                <Col xs={6}>
                  <Button
                    variant={enableCooldown ? "success" : "danger"}
                    onClick={() => handleEnableCooldown(!enableCooldown)}
                  >
                    {enableCooldown ? "Disable Cooldown" : "Enable Cooldown"}
                  </Button>
                </Col>
                <Col xs={6}>
                  <Button
                    variant={enableAutoplay ? "success" : "danger"}
                    onClick={() => handleEnableAutoplay(!enableAutoplay)}
                  >
                    {enableAutoplay ? "Disable Autoplay" : "Enable Autoplay"}
                  </Button>
                </Col>
              </Row>

              {enableCooldown && (
                <div className="mb-4 p-3 border rounded bg-white">
                  <h5 className="text-center mb-3">Cooldown Settings</h5>
                  <small>
                    Click Start Animating to get real-time trigger-rate and
                    cooldown/autoplay status
                  </small>
                  {triggerRate !== undefined && (
                    <Row>
                      <h3 className="mb-3">Trigger Rate: {triggerRate}</h3>
                    </Row>
                  )}
                  <Row className="mb-3">
                    <Col>
                      <Form.Label>
                        High Threshold: {triggerRateHighThreshold || "--"}
                      </Form.Label>
                      <Form.Range
                        aria-label={`Trigger Rate Threshold: ${
                          triggerRateHighThreshold || "--"
                        }`}
                        min={0.01}
                        max={100}
                        step={0.01}
                        value={triggerRateHighThreshold}
                        onChange={(e) =>
                          handleTriggerRateHighThreshold(e.target.value)
                        }
                      />
                    </Col>
                  </Row>
                  <Row>
                    <Col>
                      <Form.Label>
                        Cool Down Interval: {triggerCooldownInterval || "--"} s
                      </Form.Label>
                      <Form.Range
                        aria-label={`Trigger Cool Down Interval: ${
                          triggerCooldownInterval || "--"
                        } s`}
                        min={1.0}
                        max={1800.0}
                        step={0.01}
                        value={triggerCooldownInterval}
                        onChange={(e) =>
                          handleTriggerCooldownInterval(e.target.value)
                        }
                      />
                    </Col>
                  </Row>
                  <Row className="text-center mt-3">
                    <Col>
                      <Button
                        variant={cooldown ? "success" : "danger"}
                        size="lg"
                        disabled
                      >
                        {cooldown
                          ? "Sequencer Cooling Down"
                          : "Sequencer Trigger Active"}
                      </Button>
                    </Col>
                  </Row>
                </div>
              )}

              {enableAutoplay && (
                <div className="mb-4 p-3 border rounded bg-white">
                  <h5 className="text-center mb-3">Autoplay Settings</h5>
                  <Row className="mb-3">
                    <Col>
                      <Form.Label>Autoplay Priority Level: {trigRatePriorityLevel}</Form.Label>
                      <Form.Range
                        aria-label={`Autoplay Priority Level: ${trigRatePriorityLevel}`}
                        min={1}
                        max={10}
                        value={trigRatePriorityLevel}
                        onChange={(e) => handleTrigRatePriorityLevel(e.target.value)}
                      />
                    </Col>
                  </Row>
                  <Row>
                    <Col>
                      <Form.Label>Inactivity Threshold</Form.Label>
                      <Row className="mb-4">
                        <Col sm={6}>
                          <Form.Label>
                            Hours:{" "}
                            {Math.floor(triggerInactivityThreshold / 3600)}
                          </Form.Label>
                          <Form.Range
                            min={0}
                            max={24}
                            step={1}
                            value={Math.floor(
                              triggerInactivityThreshold / 3600
                            )}
                            onChange={(e) => {
                              const hours = parseInt(e.target.value, 10);
                              handleTriggerInactivityThreshold(
                                (isNaN(hours) ? 0 : hours) * 3600 +
                                  Math.floor(
                                    (triggerInactivityThreshold % 3600) / 60
                                  ) *
                                    60
                              );
                            }}
                          />
                        </Col>
                        <Col sm={6}>
                          <Form.Label>
                            Minutes:{" "}
                            {Math.floor(
                              (triggerInactivityThreshold % 3600) / 60
                            )}
                          </Form.Label>
                          <Form.Range
                            min={0}
                            max={59}
                            step={1}
                            value={Math.floor(
                              (triggerInactivityThreshold % 3600) / 60
                            )}
                            onChange={(e) => {
                              const minutes = parseInt(e.target.value, 10);
                              handleTriggerInactivityThreshold(
                                Math.floor(triggerInactivityThreshold / 3600) *
                                  3600 +
                                  (isNaN(minutes) ? 0 : minutes) * 60
                              );
                            }}
                          />
                        </Col>
                      </Row>
                      <small className="text-muted">
                        Current Inactivity Threshold:{" "}
                        {triggerInactivityThreshold || "--"} seconds
                      </small>
                    </Col>
                  </Row>
                  <Row className="text-center mt-3">
                    <Col>
                      <Button
                        variant={autoplay ? "danger" : "success"}
                        size="lg"
                        disabled
                      >
                        {autoplay
                          ? "Sequencer Auto-play On"
                          : "Sequencer Auto-play Off"}
                      </Button>
                    </Col>
                  </Row>
                </div>
              )}
            </>
          )}
        </div>
      )}

      <Row>
        <Col xs={12} className="mb-3">
          <Button onClick={handleSave}>Save Settings</Button>
        </Col>
      </Row>
      {shouldRenderSynthControls() && (
        <>
          <Row className="mb-3">
            <Col xs={12} className="">
              <SynthControls
                synthId={context === "output" ? outputId : output}
                synthLabel={context === "output" ? outputId : output}
                macAddress={macAddress}
              />
            </Col>
          </Row>
        </>
      )}
    </Container>
  );
}

export default Sequencer;
