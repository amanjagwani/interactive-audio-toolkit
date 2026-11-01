import React from "react";
import { Link, useParams } from "react-router-dom";
import useStore from "../store";
import { useWebSocket } from "./WebSocketContext";
import { Form, Row, Col, Button } from "react-bootstrap";
import { useEffect } from "react";
import { useLocation } from "react-router-dom";
import NoiseControlModal from "./NoiseControl";
import { useState } from "react";

function MainPage() {
  const { bpm, setBpm, setPlotting, deviceStates, setDeviceState, devices } = useStore();
  const { sendControlChange, sendControlChangeToServer, messages } =
    useWebSocket();
  const { macAddress } = useParams();
  const sensors = devices[macAddress]?.sensors || [];
  const outputs = devices[macAddress]?.outputs || [];
  const noiseOutput = outputs.find((output) => output.type === "noiseSynth");
  const location = useLocation();
  const [showNoiseModal, setShowNoiseModal] = useState(false);

  const handleShowNoiseModal = () => setShowNoiseModal(true);
  const handleCloseNoiseModal = () => setShowNoiseModal(false);

  const name = location.state?.customName || macAddress;

  const deviceState = deviceStates[macAddress] || {};
  const {
    triggerRate = 0,
    triggerRateHighThreshold = 0,
    triggerCooldownInterval = 0,
    triggerInactivityThreshold = 0,
    trigRatePriorityLevel = 2,
    cooldown = false,
    autoplay = false,
    isAnimating = false,
    enableTriggerRateProcessing = false,
    enableAutoplay = false,
    enableCooldown = false,
    noiseAmp = 0.1,
    noiseBandwidth = 300,
    noiseModDepth = 200,
    noiseResonFreq = 2000,
    noiseUpdateRate = 2,
  } = deviceState;

  useEffect(() => {
    sendControlChange(macAddress, `requestSettings`, 1);
  }, []);

  useEffect(() => {
    if (messages.length > 0) {
      const lastMessage = messages[messages.length - 1];
      if (lastMessage.macAddress === macAddress) {
        if (lastMessage.globalSettings) {
          setBpm(lastMessage.globalSettings["bpm"]);
        }
        if (lastMessage["deviceTriggerRate"] !== undefined) {
          setDeviceState(macAddress, {
            triggerRate: lastMessage["deviceTriggerRate"],
          });
        }
        if (lastMessage["deviceCooldown"] !== undefined) {
          setDeviceState(macAddress, {
            cooldown: lastMessage["deviceCooldown"],
          });
        }
        if (lastMessage["deviceAutoplay"] !== undefined) {
          setDeviceState(macAddress, {
            autoplay: lastMessage["deviceAutoplay"],
          });
        }
        const noiseSettings =
          noiseOutput && lastMessage.outputSettings?.[noiseOutput.id];
        if (noiseSettings) {
          setDeviceState(macAddress, {
            ...deviceState,
            noiseAmp: noiseSettings.amp,
            noiseBandwidth: noiseSettings.bandwidth,
            noiseModDepth: noiseSettings.modDepth,
            noiseResonFreq: noiseSettings.resonFreq,
            noiseUpdateRate: noiseSettings.updateRate,
            manualEnabled: noiseSettings.manualEnabled,
          });
        }
        if (lastMessage.globalSettings) {
          if (
            lastMessage.globalSettings["triggerCooldownInterval"] !== undefined
          ) {
            setDeviceState(macAddress, {
              triggerCooldownInterval:
                lastMessage.globalSettings["triggerCooldownInterval"] / 1000,
            });
          }

          if (
            lastMessage.globalSettings["triggerInactivityThreshold"] !==
            undefined
          ) {
            setDeviceState(macAddress, {
              triggerInactivityThreshold:
                lastMessage.globalSettings["triggerInactivityThreshold"] / 1000,
            });
          }

          if (
            lastMessage.globalSettings["triggerRateHighThreshold"] !== undefined
          ) {
            setDeviceState(macAddress, {
              triggerRateHighThreshold:
                lastMessage.globalSettings["triggerRateHighThreshold"],
            });
          }

          if (
            lastMessage.globalSettings["enableTriggerRateProcessing"] !==
            undefined
          ) {
            setDeviceState(macAddress, {
              enableTriggerRateProcessing:
                lastMessage.globalSettings["enableTriggerRateProcessing"],
            });
          }

          if (lastMessage.globalSettings["enableAutoplay"] !== undefined) {
            setDeviceState(macAddress, {
              enableAutoplay: lastMessage.globalSettings["enableAutoplay"],
            });
          }

          if (lastMessage.globalSettings["trigRatePriorityLevel"] !== undefined) {
            setDeviceState(macAddress, {
              trigRatePriorityLevel: lastMessage.globalSettings["trigRatePriorityLevel"],
            });
          }

          if (lastMessage.globalSettings["enableCooldown"] !== undefined) {
            setDeviceState(macAddress, {
              enableCooldown: lastMessage.globalSettings["enableCooldown"],
            });
          }
        }
      }
    }
  }, [messages]);

  useEffect(() => {
    const handleBeforeUnload = () => {
      if (isAnimating) {
        setDeviceState(macAddress, { isAnimating: false });
        sendControlChangeToServer(macAddress, "device", "toggleAnimation",
          false
        );
      }
    };
    window.addEventListener("beforeunload", handleBeforeUnload);

    return () => {
      handleBeforeUnload();
      window.removeEventListener("beforeunload", handleBeforeUnload);
    };
  }, []);

  function handleBpm(bpm) {
    setBpm(bpm);
    sendControlChange(macAddress, "bpm", bpm);
  }

  const handleSave = () => {
    sendControlChange(macAddress, "saveSettings", 1);
  };

  const handleTriggerRateHighThreshold = (value) => {
    setDeviceState(macAddress, { triggerRateHighThreshold: value });
    sendControlChangeToServer(macAddress, "device", "triggerRateHighThreshold",
      value
    );
  };

  const handleTriggerCooldownInterval = (value) => {
    setDeviceState(macAddress, { triggerCooldownInterval: value });
    sendControlChangeToServer(macAddress, "device", "triggerCooldownInterval",
      value * 1000
    );
  };

  const handleTriggerInactivityThreshold = (value) => {
    setDeviceState(macAddress, { triggerInactivityThreshold: value });
    sendControlChangeToServer(macAddress, "device", "triggerInactivityThreshold",
      value * 1000
    );
  };

  const toggleAnimation = () => {
    const newIsAnimating = !isAnimating;
    setDeviceState(macAddress, { isAnimating: newIsAnimating });
    sendControlChangeToServer(macAddress, "device", "toggleAnimation",
      newIsAnimating
    );
  };

  const handleEnableTriggerRateProcessing = (value) => {
    setDeviceState(macAddress, { enableTriggerRateProcessing: value });
    sendControlChangeToServer(macAddress, "device", "enableTriggerRateProcessing",
      value
    );
  };

  const handleEnableCooldown = (value) => {
    setDeviceState(macAddress, { enableCooldown: value });
    sendControlChangeToServer(macAddress, "device", "enableCooldown", value);
  };

  const handleEnableAutoplay = (value) => {
    setDeviceState(macAddress, { enableAutoplay: value });
    sendControlChangeToServer(macAddress, "device", "enableAutoplay", value);
  };

  const handleTrigRatePriorityLevel = (value) => {
    setDeviceState(macAddress, { trigRatePriorityLevel: value });
    sendControlChangeToServer(macAddress, "device", "trigRatePriorityLevel", value);
  };

  return (
    <div className="container mt-5">
      <h1 className="mb-4 text-center">Device: {name}</h1>

      <div className="mb-5">
        <h3 className="text-center">Sensors</h3>
        <div className="d-flex flex-column align-items-center">
          {sensors.map((sensor) => (
            <Link
              key={sensor.sensorId}
              to={`/device/${macAddress}/sensor/${sensor.sensorId}`}
              className="btn btn-primary mb-2"
            >
              {sensor.sensorId}
            </Link>
          ))}
        </div>
      </div>

      <div className="mb-5">
        <h3 className="text-center">Global BPM</h3>
        <div className="d-flex flex-column align-items-center">
          <Form.Label className="mb-2">BPM: {bpm}</Form.Label>
          <Form.Control
            type="number"
            value={bpm}
            min={20}
            max={1200}
            onChange={(e) => handleBpm(e.target.value)}
            className="mb-3 w-auto"
          />
          <Form.Range
            aria-label={`Bpm: ${bpm}`}
            min={20}
            max={1200}
            value={bpm}
            onChange={(e) => handleBpm(e.target.value)}
            className="w-50"
          />
        </div>
      </div>

      <div className="mb-5">
        <h3 className="text-center">Device-level Trigger Rate Processing</h3>

        <Row className="text-center mb-4">
          <Col>
            <Button
              variant={enableTriggerRateProcessing ? "success" : "danger"}
              onClick={() =>
                handleEnableTriggerRateProcessing(!enableTriggerRateProcessing)
              }
              size="lg"
            >
              {enableTriggerRateProcessing
                ? "Disable Device-level Trigger Rate Processing"
                : "Enable Device-level Trigger Rate Processing"}
            </Button>
          </Col>
        </Row>

        {enableTriggerRateProcessing && (
          <>
            <Row className="mb-4">
              <Col xs={12} md={6} className="mb-3 text-center">
                <Button
                  variant={enableCooldown ? "success" : "danger"}
                  onClick={() => handleEnableCooldown(!enableCooldown)}
                >
                  {enableCooldown ? "Disable Cooldown" : "Enable Cooldown"}
                </Button>
              </Col>
              <Col xs={12} md={6} className="text-center">
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
                          Hours: {Math.floor(triggerInactivityThreshold / 3600)}
                        </Form.Label>
                        <Form.Range
                          min={0}
                          max={24}
                          step={1}
                          value={Math.floor(triggerInactivityThreshold / 3600)}
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
                          {Math.floor((triggerInactivityThreshold % 3600) / 60)}
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

            <Row className="text-center">
              <Col>
                <Button
                  variant={isAnimating ? "danger" : "success"}
                  onClick={toggleAnimation}
                  size="lg"
                >
                  {isAnimating ? "Stop Animating" : "Start Animating"}
                </Button>
              </Col>
            </Row>
          </>
        )}
      </div>
      {noiseOutput && (
      <div className="mb-5">
        <h3 className="text-center">Noise Generator</h3>
        <div className="d-flex justify-content-center">
          <Button variant="info" onClick={handleShowNoiseModal}>
            Open Noise Controls
          </Button>
        </div>
      </div>

      )}

      {noiseOutput && (
      <NoiseControlModal
        show={showNoiseModal}
        handleClose={handleCloseNoiseModal}
        sendControlChange={sendControlChange}
        macAddress={macAddress}
        noiseId={noiseOutput.id}
        noiseAmp={noiseAmp}
        noiseBandwidth={noiseBandwidth}
        noiseModDepth={noiseModDepth}
        noiseResonFreq={noiseResonFreq}
        noiseUpdateRate={noiseUpdateRate}
      />
      )}
    </div>
  );
}

export default MainPage;
