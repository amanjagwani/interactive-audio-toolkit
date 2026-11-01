import React, { useState, useEffect } from "react";
import { useParams, Link, useNavigate, useLocation } from "react-router-dom";
import { useWebSocket } from "./WebSocketContext";
import SensorGraph from "./SensorGraph";
import { Button, Container, Row, Col, Form } from "react-bootstrap";
import useStore from "../store";

function SensorPage() {
  const { messages, sendControlChange } = useWebSocket();
  const { macAddress, id } = useParams();
  const location = useLocation();
  const navigate = useNavigate();

  const [sensorData, setSensorData] = useState({
    labels: [],
    datasets: [
      {
        label: `${id} Data`,
        data: [],
        borderColor: "rgb(75, 192, 192)",
        tension: 0.1,
      },
      {
        label: "High Threshold",
        data: [],
        borderColor: "rgba(255, 99, 132)",
        borderDash: [10, 5],
        pointRadius: 0,
        borderWidth: 3,
      },
      {
        label: "Mid Threshold",
        data: [],
        borderColor: "rgba(54, 162, 235)",
        borderDash: [10, 5],
        pointRadius: 0,
        borderWidth: 3,
      },
      {
        label: "Low Threshold",
        data: [],
        borderColor: "rgba(255, 206, 86)",
        borderDash: [10, 5],
        pointRadius: 0,
        borderWidth: 3,
      },
    ],
  });

  const {
    sensors,
    setSensorState,
    setThreshold,
    setPlotting,
    setEnabled,
    sensorRanges,
    devices,
    setSequencerState,
    sequencers,
  } = useStore();
  const sensorState = sensors[id] || {};

  const getSensorRange = (sensorId) => {
    const defaultRange = { min: 0, max: 100, step: 1 };
    return sensorRanges[sensorId.toUpperCase()] || defaultRange;
  };
  const isLoadCell =
    devices[macAddress]?.sensors?.find((sensor) => sensor.sensorId === id)
      ?.sensorType === "loadcell";
  const { min, max, step } = getSensorRange(id.toUpperCase());

  const {
    isPlotting = false,
    isEnabled = false,
    lowThres = 0,
    midThres = 0,
    highThres = 0,
    sensorValue = 0,
    minVal = min,
    maxVal = max,
    tareValue = 0,
    trigTimeFrame = 60,
    trigOffPeriod = 20,
    trigThreshold = 60,
    trigRate = 0,
    isTriggerActive = true,
  } = sensorState;

  useEffect(() => {
    if (isPlotting && messages.length > 0) {
      const newMessages = messages.filter(
        (message) =>
          message.macAddress === macAddress && message[id] !== undefined
      );
      const newLabels = newMessages.map((message, index) => index);
      const newData = newMessages.map((message) => message[id]);
      const latestValue = newData.length > 0 ? newData[newData.length - 1] : 0;
      setSensorState(id, { sensorValue: latestValue });

      const lastMessage = messages[messages.length - 1];
      if (lastMessage[`${id}TrigRate`] !== undefined) {
        setSensorState(id, {
          trigRate: lastMessage[`${id}TrigRate`],
        });
      }

      if (lastMessage[`${id}TrigOff`] !== undefined) {
        setSensorState(id, {
          isTriggerActive: !lastMessage[`${id}TrigOff`],
        });
      }

      setSensorData((prevData) => ({
        labels: newLabels,
        datasets: prevData.datasets.map((dataset, index) => {
          if (index === 0) {
            return { ...dataset, data: newData };
          } else {
            const thresholdValue =
              index === 1 ? highThres : index === 2 ? midThres : lowThres;
            return {
              ...dataset,
              data: Array(newLabels.length).fill(thresholdValue),
            };
          }
        }),
      }));
    }
  }, [messages, id, isPlotting, highThres, midThres, lowThres]);

  useEffect(() => {
    const maxThreshold = max;

    const defaults = {};
    let updateRequired = false;

    if (sensorState.highThres === undefined) {
      defaults.highThres = maxThreshold;
      updateRequired = true;
    }
    if (sensorState.midThres === undefined) {
      defaults.midThres = maxThreshold;
      updateRequired = true;
    }
    if (sensorState.lowThres === undefined) {
      defaults.lowThres = maxThreshold;
      updateRequired = true;
    }

    if (updateRequired) {
      setSensorState(id, defaults);
    }
  }, [id, sensorState, setSensorState]);

  function handleSequencerNavigation(type) {
    console.log("Navigating with macAddress:", macAddress);
    if (!macAddress) {
      console.error("macAddress is missing before navigation!");
      return;
    }

    navigate(`/device/${macAddress}/sequencer/${id}-${type}`, {
      state: { thresholdType: type },
    });

    if (isPlotting) {
      setPlotting(id, false);
      sendControlChange(macAddress, `${id}-plot`, false);
    }
  }

  const handleThresholdChange = (type, value) => {
    setThreshold(id, type, parseInt(value, 10));
    setSensorState(id, { [`${type}Thres`]: parseInt(value, 10) });

    const thresholdTypes = ["low", "mid", "high"];
    const thresholds = {
      low: lowThres,
      mid: midThres,
      high: highThres,
      [type]: parseInt(value, 10),
    };

    const thresholdRanges = {};

    thresholdRanges.low =
      isLoadCell
        ? midThres - thresholds.low
        : thresholds.low - minVal;
    thresholdRanges.mid =
      isLoadCell
        ? thresholds.high - midThres
        : midThres - thresholds.low;
    thresholdRanges.high =
      isLoadCell
        ? maxVal - thresholds.high
        : thresholds.high - midThres;

    thresholdTypes.forEach((thresholdType) => {
      const thresholdValue = thresholds[thresholdType];
      const rangeValue = thresholdRanges[thresholdType];
      const seqId = `${id}-${thresholdType}`;
      const seqId2 = `${id}-${thresholdType}2`;

      setSequencerState(seqId, {
        ...sequencers[seqId],
        thresholdRange: rangeValue,
      });

      setSequencerState(seqId2, {
        ...sequencers[seqId2],
        thresholdRange: rangeValue,
      });

      sendControlChange(
        macAddress,
        `${id}-${thresholdType}-threshold`,
        thresholdValue
      );
      sendControlChange(
        macAddress,
        `${id}-${thresholdType}-thresholdRange`,
        rangeValue
      );
      sendControlChange(
        macAddress,
        `${id}-${thresholdType}2-threshold`,
        thresholdValue
      );
      sendControlChange(
        macAddress,
        `${id}-${thresholdType}2-thresholdRange`,
        rangeValue
      );
    });
  };

  const handleThresholdUpdate = (type, value) => {
    setThreshold(id, type, parseInt(value, 10));
    setSensorState(id, { [`${type}Thres`]: parseInt(value, 10) });
  };

  const handleMinValUpdate = (value) => {
    setSensorState(id, { minVal: parseInt(value, 10) });
  };

  const handleMaxValUpdate = (value) => {
    setSensorState(id, { maxVal: parseInt(value, 10) });
  };

  const handleMinValChange = (value) => {
    setSensorState(id, { minVal: parseInt(value, 10) });
    sendControlChange(macAddress, `${id}-min`, value);
  };

  const handleMaxValChange = (value) => {
    setSensorState(id, { maxVal: parseInt(value, 10) });
    sendControlChange(macAddress, `${id}-max`, value);
  };

  const handlePlot = () => {
    const newIsPlottingState = !isPlotting;
    setPlotting(id, newIsPlottingState);
    sendControlChange(macAddress, `${id}-plot`, newIsPlottingState);
  };

  useEffect(() => {
    const handleBeforeUnload = () => {
      if (isPlotting) {
        setPlotting(id, false);
        sendControlChange(macAddress, `${id}-plot`, false);
      }
    };
    window.addEventListener("beforeunload", handleBeforeUnload);

    return () => {
      handleBeforeUnload();
      window.removeEventListener("beforeunload", handleBeforeUnload);
    };
  }, []);

  const handleEnable = () => {
    const newEnabledState = !isEnabled;
    setEnabled(id, newEnabledState);
    sendControlChange(macAddress, `${id}-isEnabled`, newEnabledState);
  };

  const handleTare = () => {
    sendControlChange(macAddress, `${id}-tare`, 1);
  };

  const handleClearTare = () => {
    sendControlChange(macAddress, `${id}-clearTare`, 1);
  };

  useEffect(() => {
    if (messages.length > 0) {
      const lastMessage = messages[messages.length - 1];

      if (lastMessage.macAddress === macAddress) {
        const settings = lastMessage.sensorSettings?.[id];
        if (settings) {
          setSensorState(id, {
            isEnabled: settings.isEnabled,
            maxVal: settings.max,
            minVal: settings.min,
          });
        }
        ["low", "mid", "high"].forEach((type) => {
          const seqSettings = lastMessage.sequencerSettings?.[`${id}-${type}`];
          if (seqSettings) {
            setSensorState(id, { [`${type}Thres`]: seqSettings.threshold });
          }
        });
      }
    }
  }, [messages, macAddress, id, setSensorState]);

  useEffect(() => {
    sendControlChange(macAddress, `requestSettings`, 1);
  }, []);

  const getThresholdLabel = (thresholdType) => {
    const typeLabels = isLoadCell
      ? {
          high: "High Threshold (Heavy)",
          mid: "Mid Threshold (Medium)",
          low: "Low Threshold (Light)",
        }
      : {
          high: "High Threshold (Far)",
          mid: "Mid Threshold (Medium)",
          low: "Low Threshold (Close)",
        };
    return typeLabels[thresholdType];
  };

  return (
    <Container className="mt-5">
      <h2 className="text-center mb-4">{id.toUpperCase()} Sensor</h2>
      <h4 className="text-center mb-4">Current Value: {sensorValue}</h4>
      <Row>
        <Col
          md={{ span: 10, offset: 1 }}
          className="mb-5"
          style={{ height: "350px" }}
        >
          <SensorGraph
            key={id}
            id={id}
            sensorData={sensorData}
            maxVal={maxVal}
          />
        </Col>
      </Row>
      <Row></Row>
      <Row className="justify-content-center mt-3">
        <Col
          md={{ span: 4, offset: 0 }}
          className="d-flex flex-column align-items-start"
        >
          <Button
            className="mb-3"
            variant={isEnabled ? "danger" : "success"}
            onClick={handleEnable}
          >
            {isEnabled ? "Disable Sensor" : "Enable Sensor"}
          </Button>
          <Button
            className="mb-3"
            variant={isPlotting ? "danger" : "success"}
            onClick={handlePlot}
          >
            {isPlotting ? "Stop Plotting" : "Start Plotting"}
          </Button>
          {(isLoadCell) && (
            <>
              <Button className="mb-3" variant="info" onClick={handleTare}>
                Tare
              </Button>
              <Button className="mb-3" variant="info" onClick={handleClearTare}>
                Clear Tare
              </Button>
            </>
          )}
          <Form.Group className="d-flex align-items-center mb-3">
            <Form.Label className="me-3">
              {" "}
              <small>Min Value: </small>{" "}
            </Form.Label>
            <Form.Range
              className="custom-range flex-grow-1"
              value={minVal}
              min={min}
              max={max}
              step={step}
              onChange={(e) => handleMinValUpdate(e.target.value)}
              onMouseUp={(e) => handleMinValChange(e.target.value)}
              onTouchEnd={(e) => handleMinValChange(e.target.value)}
            />
            <Form.Control
              type="number"
              value={minVal}
              min={min}
              max={max}
              step={step}
              onChange={(e) => handleMinValUpdate(e.target.value)}
              onBlur={(e) => handleMinValChange(e.target.value)}
              className="ms-3 w-auto"
            />
          </Form.Group>

          <Form.Group className="d-flex align-items-center mb-3">
            <Form.Label className="me-3">
              <small>Max Value: </small>
            </Form.Label>
            <Form.Range
              className="custom-range flex-grow-1"
              value={maxVal}
              min={min}
              max={max}
              step={step}
              onChange={(e) => handleMaxValUpdate(e.target.value)}
              onMouseUp={(e) => handleMaxValChange(e.target.value)}
              onTouchEnd={(e) => handleMaxValChange(e.target.value)}
            />
            <Form.Control
              type="number"
              value={maxVal}
              min={min}
              max={max}
              step={step}
              onChange={(e) => handleMaxValUpdate(e.target.value)}
              onBlur={(e) => handleMaxValChange(e.target.value)}
              className="ms-3 w-auto"
            />
          </Form.Group>
        </Col>
        <Col md={{ span: 7, offset: 0.5 }}>
          <Form>
            {["high", "mid", "low"].map((type) => (
              <Form.Group key={type} className="d-flex align-items-center mb-3">
                <Form.Label className="me-3">
                  <small> {getThresholdLabel(type)}: </small>
                </Form.Label>
                <Form.Range
                  className="custom-range flex-grow-1"
                  value={{ low: lowThres, mid: midThres, high: highThres }[type]}
                  min={min}
                  max={max}
                  step={step}
                  onChange={(e) => handleThresholdUpdate(type, e.target.value)}
                  onMouseUp={(e) => handleThresholdChange(type, e.target.value)}
                  onTouchEnd={(e) =>
                    handleThresholdChange(type, e.target.value)
                  }
                />
                <Form.Control
                  type="number"
                  value={{ low: lowThres, mid: midThres, high: highThres }[type]}
                  min={min}
                  max={max}
                  step={step}
                  onChange={(e) => handleThresholdChange(type, e.target.value)}
                  className="ms-3 w-auto"
                />
                <Button
                  className="ms-3"
                  variant="primary"
                  onClick={() => handleSequencerNavigation(type)}
                >
                  {`${type.charAt(0).toUpperCase()}${type.slice(
                    1
                  )} Sequencer 1`}
                </Button>
                <Button
                  className="ms-3"
                  variant="primary"
                  onClick={() => handleSequencerNavigation(`${type}2`)}
                >
                  {`${type.charAt(0).toUpperCase()}${type.slice(
                    1
                  )} Sequencer 2`}
                </Button>
              </Form.Group>
            ))}
          </Form>
        </Col>
      </Row>
    </Container>
  );
}

export default SensorPage;
