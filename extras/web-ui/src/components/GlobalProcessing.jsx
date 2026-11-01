import React, { useEffect, useState } from "react";
import useStore from "../store";
import {
  Modal,
  Badge,
  Form,
  Row,
  Col,
  Button,
  Container,
} from "react-bootstrap";
import { useWebSocket } from "./WebSocketContext";
import { useNavigate } from "react-router-dom";

const GlobalProcessing = () => {
  const navigate = useNavigate();
  const {
    globalActivations,
    addGlobalActivation,
    setGlobalActivation,
    deleteGlobalActivation,
  } = useStore();
  const {
    messages,
    sendControlChange,
    sendGlobalActivationMessage,
    sendGlobalControlChange,
  } = useWebSocket();
  const [showInputModal, setShowInputModal] = useState(false);
  const [showOutputModal, setShowOutputModal] = useState(false);
  const [activeModalIndex, setActiveModalIndex] = useState(null);
  const [selectedInputSequencers, setSelectedInputSequencers] = useState({});
  const [selectedOutputSequencers, setSelectedOutputSequencers] = useState({});
  const [macAddress, setMacAddress] = useState();
  const connectedDevices = Object.values(useStore((state) => state.devices));
  const [weatherData, setWeatherData] = useState(null);
  const [triggeredStates, setTriggeredStates] = useState({});
  const [lastTriggeredTimes, setLastTriggeredTimes] = useState({});
  const displayDeviceName = (device) => device.customName || device.macAddress;

  const allOutputs = [
    { label: "Audio Synthesis 1", value: "audio1" },
    { label: "Audio Synthesis 2", value: "audio2" },
    { label: "Stepper Motor", value: "stepper" },
    { label: "DC Motor", value: "dc" },
    { label: "Relay 1", value: "rel1" },
    { label: "Relay 2", value: "rel2" },
    { label: "No Output", value: "none" },
  ];

  useEffect(() => {
    if (messages.length > 0) {
      const lastMessage = messages[messages.length - 1];

      if (lastMessage.type === "globalActivationSettings" && lastMessage.data) {
        const activations = Object.values(lastMessage.data);
        activations.forEach((activation) => {
          addGlobalActivation(activation);
        });
      }

      if (lastMessage.type === "weatherUpdate" && lastMessage.data) {
        setWeatherData(lastMessage.data);
      }

      if (
        lastMessage.type === "globalActivationTriggered" &&
        lastMessage.data
      ) {
        const { id, triggered, lastTriggeredTime } = lastMessage.data;

        setTriggeredStates((prevStates) => ({
          ...prevStates,
          [id]: triggered,
        }));

        if (lastTriggeredTime) {
          setLastTriggeredTimes((prevTimes) => ({
            ...prevTimes,
            [id]: lastTriggeredTime,
          }));
        }
      }
    }
  }, [messages]);

  useEffect(() => {
    sendGlobalControlChange(`requestDevices`, 1);
    sendGlobalActivationMessage("request", null);
  }, []);

  const updateNestedState = (index, fieldPath, value) => {
    if (!fieldPath || !Array.isArray(fieldPath) || fieldPath.length === 0) {
      console.error("Invalid or undefined fieldPath:", fieldPath);
      return;
    }

    const updatedActivation = { ...globalActivations[index] };
    let target = updatedActivation;

    fieldPath.slice(0, -1).forEach((key) => {
      if (!target[key]) target[key] = {};
      target = target[key];
    });

    target[fieldPath[fieldPath.length - 1]] = value;
    setGlobalActivation(index, updatedActivation);
  };

  const handleOutputSelection = (device, output, index) => {
    const outputId = `${device}-${output.value}`;
    updateNestedState(index, ["output", "outputId"], outputId);
    handleUpdateActivation(index);
  };

  const handleOutputSequencerNavigation = (activationId, outputId) => {
    const [device, output] = outputId.split("-");
    navigate(`/${activationId}/${device}/${output}/sequencer`, {
      state: { device, output },
    });
  };

  const handleSequencerSelection = (index, device, sensor, threshold, type) => {
    const sequencerId = `${device}-${sensor}-${threshold}`;
    if (type === "input") {
      setSelectedInputSequencers((prevState) => ({
        ...prevState,
        [index]: Array.isArray(prevState[index])
          ? prevState[index].includes(sequencerId)
            ? prevState[index].filter((id) => id !== sequencerId)
            : [...prevState[index], sequencerId]
          : [sequencerId],
      }));
    } else {
      setSelectedOutputSequencers((prevState) => ({
        ...prevState,
        [index]: Array.isArray(prevState[index])
          ? prevState[index].includes(sequencerId)
            ? prevState[index].filter((id) => id !== sequencerId)
            : [...prevState[index], sequencerId]
          : [sequencerId],
      }));
    }
  };

  const saveSelectedSensorThresholds = () => {
    if (activeModalIndex === null) return;
    const selectedSequencers = selectedInputSequencers[activeModalIndex] || [];
    const updatedActivation = {
      ...globalActivations[activeModalIndex],
      input: {
        ...globalActivations[activeModalIndex].input,
        sensorThreshold: {
          thresholds: selectedSequencers.map((id) => {
            const [device, sensor, ...thresholdParts] = id.split("-");
            const threshold = thresholdParts.join("-");
            return { device, sensor, threshold };
          }),
        },
      },
    };

    setGlobalActivation(activeModalIndex, updatedActivation);
    setShowInputModal(false);
    setActiveModalIndex(null);
    sendGlobalActivationMessage("update", updatedActivation);
  };

  const saveSelectedSequencers = () => {
    if (activeModalIndex === null) return;
    const selectedSequencers = selectedOutputSequencers[activeModalIndex] || [];
    const updatedActivation = {
      ...globalActivations[activeModalIndex],
      output: {
        ...globalActivations[activeModalIndex].output,
        sequencers: selectedSequencers.map((id) => {
          const [device, sensor, ...thresholdParts] = id.split("-");
          const threshold = thresholdParts.join("-");
          return { device, sensor, threshold };
        }),
      },
    };

    setGlobalActivation(activeModalIndex, updatedActivation);
    setShowOutputModal(false);
    setActiveModalIndex(null);
    sendGlobalActivationMessage("update", updatedActivation);
  };

  const handleInputModalOpen = (index) => {
    const savedThresholds =
      globalActivations[index]?.input?.sensorThreshold?.thresholds || [];
    const initialInputSequencers = savedThresholds.map(
      ({ device, sensor, threshold }) => `${device}-${sensor}-${threshold}`
    );

    setSelectedInputSequencers((prevState) => ({
      ...prevState,
      [index]: initialInputSequencers,
    }));
    setActiveModalIndex(index);
    setShowInputModal(true);
  };

  const handleOutputModalOpen = (index) => {
    const savedSequencers = globalActivations[index]?.output?.sequencers || [];
    const initialOutputSequencers = savedSequencers.map(
      ({ device, sensor, threshold }) => `${device}-${sensor}-${threshold}`
    );

    setSelectedOutputSequencers((prevState) => ({
      ...prevState,
      [index]: initialOutputSequencers,
    }));
    setActiveModalIndex(index);
    setShowOutputModal(true);
  };

  const closeModal = () => {
    setShowInputModal(false);
    setShowOutputModal(false);
    setActiveModalIndex(null);
  };

  const handleEnableActivation = (index) => {
    const activation = globalActivations[index];

    if (!activation.id) {
      alert(
        "Please set an ID for this activation before enabling/disabling it."
      );
      return;
    }

    const isEnabled = !activation.enabled;
    updateNestedState(index, ["enabled"], isEnabled);

    const updatedActivation = { ...activation, enabled: isEnabled };

    const action = isEnabled ? "add" : "delete";
    sendGlobalActivationMessage(action, updatedActivation);
  };

  const handleUpdateActivation = (index) => {
    const activation = globalActivations[index];

    if (!activation.id) {
      alert("Please set an ID for this activation before updating it.");
      return;
    }

    sendGlobalActivationMessage("update", activation);
  };

  const handleDeleteActivation = (index) => {
    const activation = globalActivations[index];

    if (!activation.id) {
      alert("Please set an ID for this activation before enabling it.");
      return;
    }

    sendGlobalActivationMessage("delete", activation);
    deleteGlobalActivation(index);
  };

  const updateAndNotify = (index, fieldPath, value) => {
    if (fieldPath[0] === "id" && !value) {
      alert("ID cannot be empty. Please provide a unique ID.");
      return;
    }
    updateNestedState(index, fieldPath, value);
    handleUpdateActivation(index);
  };

  const updateAndNotifyId = (index, fieldPath, value) => {
    if (fieldPath[0] === "id" && !value) {
      return;
    }
    updateNestedState(index, fieldPath, value);
    const activation = globalActivations[index];
    sendGlobalActivationMessage("update", activation);
  };

  return (
    <Container className="mt-4">
      <h1 className="mb-4 text-center">Global Processing</h1>

      <div>
        <h3>Connected Devices</h3>
        {connectedDevices?.length > 0 ? (
          <ul>
            {connectedDevices.map((device) => (
              <li key={device.macAddress}>{displayDeviceName(device)}</li>
            ))}
          </ul>
        ) : (
          <p>No devices connected</p>
        )}
      </div>

      <div className="weather-section mt-4">
        <h3>Current Weather</h3>
        {weatherData ? (
          <ul>
            <li>Temperature: {weatherData.temperature.toFixed(2)} °C</li>
            <li>Pressure: {weatherData.pressure.toFixed(2)} hPa</li>
            <li>
              Pressure Rate:{" "}
              {weatherData.rateOfChangePressure !== null
                ? weatherData.rateOfChangePressure.toFixed(6)
                : "0.00"}{" "}
              hPa/s
            </li>
            <li>Humidity: {weatherData.humidity.toFixed(2)} %</li>
          </ul>
        ) : (
          <p>Loading weather data...</p>
        )}
      </div>

      <Button
        variant="primary"
        onClick={() => addGlobalActivation()}
        className="mb-4"
      >
        + Add Global Activation
      </Button>

      {globalActivations.map((activation, index) => (
        <div key={index} className="border p-3 mb-4 rounded shadow-sm bg-light">
          <h4 className="mb-3">Activation {index + 1}</h4>

          {lastTriggeredTimes[activation.id] && (
            <Row className="mb-3">
              <Col>
                <small className="text-muted">
                  Last Triggered: {lastTriggeredTimes[activation.id]}
                </small>
              </Col>
            </Row>
          )}

          <Row>
            <Col xs={12} md={4} className="mb-3">
              <Form.Label>
                Activation Priority Level: {activation.priorityLevel}
              </Form.Label>
              <Form.Range
                aria-label={`Activation Priority Level: ${activation.priorityLevel}`}
                min={1}
                max={10}
                value={activation.priorityLevel}
                onChange={(e) =>
                  updateAndNotify(index, ["priorityLevel"], e.target.value)
                }
              />
            </Col>
          </Row>

          <Form.Group as={Row} className="mb-3">
            <Form.Label column sm={2}>
              Activation ID:
            </Form.Label>
            <Col sm={10}>
              <Form.Control
                type="text"
                placeholder="Enter unique ID"
                value={activation.id || ""}
                onChange={(e) => {
                  let inputValue = e.target.value.trim();

                  if (inputValue.includes("-")) {
                    alert("Hyphens (-) are not allowed in Activation ID.");
                    inputValue = inputValue.replace(/-/g, "");
                  }

                  if (activation.enabled) {
                    alert(
                      "You must disable the activation before renaming it."
                    );
                    return;
                  }

                  updateNestedState(index, ["id"], inputValue);
                }}
                onBlur={() => {
                  const inputValue = activation.id || "";
                  const isDuplicate = globalActivations.some(
                    (act, idx) => act.id === inputValue && idx !== index
                  );

                  if (isDuplicate) {
                    alert(
                      "Activation ID must be unique. Please choose another ID."
                    );
                    updateAndNotifyId(index, ["id"], "");
                  } else if (inputValue) {
                    sendGlobalActivationMessage("update", activation);
                  }
                }}
              />
            </Col>
            <small className="text-muted">
              You must disable the activation before changing the ID. After ID
              change output sequencer re-config required.
            </small>
          </Form.Group>

          <Form.Group as={Row} className="mb-3">
            <Form.Label column sm={2}>
              Input Type:
            </Form.Label>
            <Col sm={10}>
              <Form.Select
                value={activation.input.type}
                onChange={(e) =>
                  updateAndNotify(index, ["input", "type"], e.target.value)
                }
              >
                <option value="">Select Input Type</option>
                <option value="TimeOfDay">Time of Day</option>
                <option value="SensorThreshold">Sensor Threshold</option>
                <option value="Weather">Weather</option>
              </Form.Select>
            </Col>
          </Form.Group>

          {activation.input.type === "TimeOfDay" && (
            <Row className="mb-3">
              <Col sm={6}>
                <Form.Group>
                  <Form.Label>Hour:</Form.Label>
                  <Form.Control
                    type="number"
                    min="0"
                    max="23"
                    value={activation.input.timeOfDay.hour || ""}
                    onChange={(e) =>
                      updateAndNotify(
                        index,
                        ["input", "timeOfDay", "hour"],
                        e.target.value
                      )
                    }
                  />
                </Form.Group>
              </Col>
              <Col sm={6}>
                <Form.Group>
                  <Form.Label>Minute:</Form.Label>
                  <Form.Control
                    type="number"
                    min="0"
                    max="59"
                    value={activation.input.timeOfDay.minute || ""}
                    onChange={(e) =>
                      updateAndNotify(
                        index,
                        ["input", "timeOfDay", "minute"],
                        e.target.value
                      )
                    }
                  />
                </Form.Group>
              </Col>
            </Row>
          )}

          {activation.input.type === "SensorThreshold" && (
            <>
              <div className="mb-3">
                <strong>Selected Thresholds:</strong>
                {activation.input.sensorThreshold?.thresholds?.length > 0 ? (
                  <ul>
                    {activation.input.sensorThreshold.thresholds.map(
                      ({ device, sensor, threshold }) => {
                        const connectedDevice = connectedDevices.find(
                          (connected) => connected.macAddress === device
                        );
                        const displayDeviceName = connectedDevice?.customName || device;
                        return (
                          <li key={`${device}-${sensor}-${threshold}`}>
                            Device: {displayDeviceName}, Sensor: {sensor}, Threshold:{" "}
                            {threshold}
                          </li>
                        );
                      }
                    )}
                  </ul>
                ) : (
                  <p>No thresholds selected</p>
                )}
              </div>
              <Button
                className="mb-3"
                onClick={() => handleInputModalOpen(index)}
              >
                Select Threshold
              </Button>
              <Modal
                show={showInputModal && activeModalIndex === index}
                onHide={closeModal}
              >
                <Modal.Header closeButton>
                  <Modal.Title>Select Sensor Threshold</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                  {connectedDevices.map((device) => (
                    <div key={device.macAddress}>
                      <h5>{displayDeviceName(device)}</h5>
                      {device.sensors.map((sensor) => (
                        <div key={sensor.sensorId}>
                          <strong>{sensor.sensorId}</strong>
                          <ul>
                            {sensor.seqIds.map((threshold) => (
                              <li key={threshold}>
                                <Form.Check
                                  type="checkbox"
                                  label={`${threshold}`}
                                  checked={
                                    Array.isArray(
                                      selectedInputSequencers[index]
                                    ) &&
                                    selectedInputSequencers[index].includes(
                                      `${device.macAddress}-${sensor.sensorId}-${threshold}`
                                    )
                                  }
                                  onChange={() =>
                                    handleSequencerSelection(
                                      index,
                                      device.macAddress,
                                      sensor.sensorId,
                                      threshold,
                                      "input"
                                    )
                                  }
                                />
                              </li>
                            ))}
                          </ul>
                        </div>
                      ))}
                    </div>
                  ))}
                </Modal.Body>
                <Modal.Footer>
                  <Button
                    variant="secondary"
                    onClick={() => setShowInputModal(false)}
                  >
                    Cancel
                  </Button>
                  <Button
                    variant="primary"
                    onClick={() => saveSelectedSensorThresholds(index)}
                  >
                    Save
                  </Button>
                </Modal.Footer>
              </Modal>
            </>
          )}

          {activation.input.type === "Weather" && (
            <>
              <Form.Group as={Row} className="mb-3">
                <Form.Label column sm={2}>
                  Weather Parameter:
                </Form.Label>
                <Col sm={10}>
                  <Form.Select
                    value={activation.input.weather?.parameter || ""}
                    onChange={(e) =>
                      updateAndNotify(
                        index,
                        ["input", "weather", "parameter"],
                        e.target.value
                      )
                    }
                  >
                    <option value="">Select Weather Parameter</option>
                    <option value="temperature">Temperature (°C)</option>
                    <option value="humidity">Humidity (%)</option>
                    <option value="pressure">Pressure (hPa)</option>
                    <option value="rateOfChangePressure">
                      Pressure Rate (hPa/s)
                    </option>
                  </Form.Select>
                </Col>
              </Form.Group>

              {activation.input.weather?.parameter && (
                <>
                  <Form.Group as={Row} className="mb-3">
                    <Form.Label column sm={2}>
                      Condition:
                    </Form.Label>
                    <Col sm={10}>
                      <Form.Select
                        value={activation.input.weather?.condition || ""}
                        onChange={(e) =>
                          updateAndNotify(
                            index,
                            ["input", "weather", "condition"],
                            e.target.value
                          )
                        }
                      >
                        <option value="">Select Condition</option>
                        <option value="above">Above</option>
                        <option value="below">Below</option>
                        <option value="equals">Equals</option>
                      </Form.Select>
                    </Col>
                  </Form.Group>

                  <Form.Group as={Row} className="mb-3">
                    <Form.Label column sm={2}>
                      Value:
                    </Form.Label>
                    <Col sm={10}>
                      <Form.Control
                        type="number"
                        value={activation.input.weather?.value || ""}
                        onChange={(e) =>
                          updateAndNotify(
                            index,
                            ["input", "weather", "value"],
                            e.target.value
                          )
                        }
                      />
                    </Col>
                  </Form.Group>
                </>
              )}
            </>
          )}

          <Form.Group as={Row} className="mb-3">
            <Form.Label column sm={2}>
              Output Type:
            </Form.Label>
            <Col sm={10}>
              <Form.Select
                value={activation.output.type || ""}
                onChange={(e) =>
                  updateAndNotify(index, ["output", "type"], e.target.value)
                }
              >
                <option value="">Select Output Type</option>
                <option value="EnabledSequencers">Enabled Sequencers</option>
                <option value="Noise">Noise</option>
              </Form.Select>
            </Col>
          </Form.Group>

          {activation.output.type === "EnabledSequencers" && (
            <>
              <div className="mb-3">
                <strong>Selected Sequencers:</strong>
                {activation.output.sequencers?.length > 0 ? (
                  <ul>
                    {activation.output.sequencers.map(
                      ({ device, sensor, threshold }) => {
                        const connectedDevice = connectedDevices.find(
                          (connected) => connected.macAddress === device
                        );
                        const displayDeviceName = connectedDevice?.customName || device;
                        return (
                          <li key={`${device}-${sensor}-${threshold}`}>
                            Device: {displayDeviceName}, Sensor: {sensor}, Threshold:{" "}
                            {threshold}
                          </li>
                        );
                      }
                    )}
                  </ul>
                ) : (
                  <p>No sequencers selected</p>
                )}
              </div>
              <Button
                className="mb-3"
                onClick={() => handleOutputModalOpen(index)}
              >
                Select Enabled Sequencers
              </Button>
              <Modal
                show={showOutputModal && activeModalIndex === index}
                onHide={closeModal}
              >
                <Modal.Header closeButton>
                  <Modal.Title>Select Sequencers</Modal.Title>
                </Modal.Header>
                <Modal.Body>
                  {connectedDevices.map((device) => (
                    <div key={device.macAddress}>
                      <h5>{displayDeviceName(device)}</h5>
                      {device.sensors.map((sensor) => (
                        <div key={sensor.sensorId}>
                          <strong>{sensor.sensorId}</strong>
                          <ul>
                            {sensor.seqIds.map((threshold) => (
                              <li key={threshold}>
                                <Form.Check
                                  type="checkbox"
                                  label={`${sensor.sensorId}-${threshold}`}
                                  checked={
                                    Array.isArray(
                                      selectedOutputSequencers[index]
                                    ) &&
                                    selectedOutputSequencers[index].includes(
                                      `${device.macAddress}-${sensor.sensorId}-${threshold}`
                                    )
                                  }
                                  onChange={() =>
                                    handleSequencerSelection(
                                      index,
                                      device.macAddress,
                                      sensor.sensorId,
                                      threshold,
                                      "output"
                                    )
                                  }
                                />
                              </li>
                            ))}
                          </ul>
                        </div>
                      ))}
                    </div>
                  ))}
                </Modal.Body>
                <Modal.Footer>
                  <Button
                    variant="secondary"
                    onClick={() => setShowOutputModal(false)}
                  >
                    Cancel
                  </Button>
                  <Button
                    variant="primary"
                    onClick={() => saveSelectedSequencers(index)}
                  >
                    Save
                  </Button>
                </Modal.Footer>
              </Modal>
            </>
          )}

          {activation.output.type === "Outputs" && (
            <>
              <Form.Group as={Row} className="mb-3">
                <Form.Label column sm={2}>
                  Output:
                </Form.Label>
                <Col sm={10}>
                  <Form.Select
                    value={activation.output.outputId || ""}
                    onChange={(e) => {
                      const [deviceMacAddress, outputValue] =
                        e.target.value.split("-");
                      const selectedOutput = allOutputs.find(
                        (o) => o.value === outputValue
                      );

                      if (selectedOutput) {
                        handleOutputSelection(
                          deviceMacAddress,
                          selectedOutput,
                          index
                        );
                      }
                    }}
                  >
                    <option value="">Select Output</option>
                    {connectedDevices.map((device) =>
                      allOutputs.map((output) => (
                        <option
                          key={`${device.macAddress}-${output.value}`}
                          value={`${device.macAddress}-${output.value}`}
                        >
                          {`${displayDeviceName(device)} - ${output.label}`}
                        </option>
                      ))
                    )}
                  </Form.Select>
                </Col>
              </Form.Group>

              {activation.output.outputId && (
                <Button
                  className="mb-3"
                  variant="primary"
                  onClick={() =>
                    handleOutputSequencerNavigation(
                      activation.id,
                      activation.output.outputId
                    )
                  }
                >
                  Configure Sequencer for{" "}
                  {(() => {
                    const [deviceMacAddress, outputValue] =
                      activation.output.outputId.split("-");

                    const device = connectedDevices.find(
                      (device) => device.macAddress === deviceMacAddress
                    );
                    const output = allOutputs.find(
                      (o) => o.value === outputValue
                    );

                    const deviceName = device ? displayDeviceName(device) : "Unknown device";
                    const outputLabel = output
                      ? output.label
                      : "Unknown Output";

                    return `${deviceName} - ${outputLabel}`;
                  })()}
                </Button>
              )}
            </>
          )}

          {activation.output.type === "Noise" && (
            <>
              <Form.Group as={Row} className="mb-3">
                <Form.Label column sm={2}>
                  Device for Noise:
                </Form.Label>
                <Col sm={10}>
                  <Form.Select
                    value={activation.output.noiseDevice || ""}
                    onChange={(e) =>
                      updateAndNotify(
                        index,
                        ["output", "noiseDevice"],
                        e.target.value
                      )
                    }
                  >
                    <option value="">Select Device</option>
                    {connectedDevices.map((device) => (
                      <option key={device.macAddress} value={device.macAddress}>
                        {displayDeviceName(device)}
                      </option>
                    ))}
                  </Form.Select>
                </Col>
              </Form.Group>

              <Form.Group as={Row} className="mb-3">
                <Form.Label column sm={2}>
                  Noise Duration (s):
                </Form.Label>
                <Col sm={10}>
                  <Form.Control
                    type="number"
                    placeholder="Enter duration in seconds"
                    value={activation.output.noiseLength || ""}
                    onChange={(e) =>
                      updateAndNotify(
                        index,
                        ["output", "noiseLength"],
                        e.target.value
                      )
                    }
                  />
                </Col>
              </Form.Group>
            </>
          )}

          <Form.Group as={Row} className="mb-3">
            <Col sm={2}>
              <Form.Check
                type="checkbox"
                label="Enable"
                checked={activation.enabled}
                onChange={() => handleEnableActivation(index)}
              />
            </Col>
            <Col sm={10}>
              <Button
                variant="danger"
                onClick={() => handleDeleteActivation(index)}
              >
                Delete Activation
              </Button>
            </Col>
          </Form.Group>
        </div>
      ))}
    </Container>
  );
};

export default GlobalProcessing;
