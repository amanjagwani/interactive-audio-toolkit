import React, { useState, useEffect } from "react";
import { Modal, Button, Form } from "react-bootstrap";

const NoiseControlModal = ({
  show,
  handleClose,
  sendControlChange,
  macAddress,
  noiseId,
  noiseAmp: initialNoiseAmp,
  noiseBandwidth: initialNoiseBandwidth,
  noiseModDepth: initialNoiseModDepth,
  noiseResonFreq: initialNoiseResonFreq,
  noiseUpdateRate: initialNoiseUpdateRate,
  manualEnabled: initialManualEnabled,
}) => {
  const [noiseAmp, setNoiseAmp] = useState(initialNoiseAmp);
  const [noiseBandwidth, setNoiseBandwidth] = useState(initialNoiseBandwidth);
  const [noiseModDepth, setNoiseModDepth] = useState(initialNoiseModDepth);
  const [noiseResonFreq, setNoiseResonFreq] = useState(initialNoiseResonFreq);
  const [noiseUpdateRate, setNoiseUpdateRate] = useState(
    initialNoiseUpdateRate
  );
  const [manualEnabled, setManualEnabled] = useState(initialManualEnabled);

  useEffect(() => {
    setNoiseAmp(initialNoiseAmp);
    setNoiseBandwidth(initialNoiseBandwidth);
    setNoiseModDepth(initialNoiseModDepth);
    setNoiseResonFreq(initialNoiseResonFreq);
    setNoiseUpdateRate(initialNoiseUpdateRate);
    setManualEnabled(initialManualEnabled);
  }, [
    initialNoiseAmp,
    initialNoiseBandwidth,
    initialNoiseModDepth,
    initialNoiseResonFreq,
    initialNoiseUpdateRate,
    initialManualEnabled,
  ]);

  const toControlName = (name) =>
    `${noiseId}-${name.replace(/^noise/, "").replace(/^./, (c) => c.toLowerCase())}`;

  const handleSliderChange = (controlName, value, setValue) => {
    setValue(value);
    sendControlChange(macAddress, toControlName(controlName), value);
  };

  return (
    <Modal show={show} onHide={handleClose} centered>
      <Modal.Header closeButton>
        <Modal.Title>Noise Generator Controls</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <Form.Group className="mb-3">
          <Form.Label>Noise Amplitude: {noiseAmp.toFixed(2)}</Form.Label>
          <Form.Range
            min={0}
            max={1}
            step={0.01}
            value={noiseAmp}
            onChange={(e) =>
              handleSliderChange(
                "noiseAmp",
                parseFloat(e.target.value),
                setNoiseAmp
              )
            }
          />
        </Form.Group>
        <Form.Group className="mb-3">
          <Form.Label>Resonant Frequency: {noiseResonFreq} Hz</Form.Label>
          <Form.Range
            min={20}
            max={20000}
            step={1}
            value={noiseResonFreq}
            onChange={(e) =>
              handleSliderChange(
                "noiseResonFreq",
                parseInt(e.target.value, 10),
                setNoiseResonFreq
              )
            }
          />
        </Form.Group>
        <Form.Group className="mb-3">
          <Form.Label>Noise Bandwidth: {noiseBandwidth} Hz</Form.Label>
          <Form.Range
            min={100}
            max={10000}
            step={1}
            value={noiseBandwidth}
            onChange={(e) =>
              handleSliderChange(
                "noiseBandwidth",
                parseInt(e.target.value, 10),
                setNoiseBandwidth
              )
            }
          />
        </Form.Group>
        <Form.Group className="mb-3">
          <Form.Label>Modulation Depth: {noiseModDepth.toFixed(2)}</Form.Label>
          <Form.Range
            min={0}
            max={10000}
            step={1}
            value={noiseModDepth}
            onChange={(e) =>
              handleSliderChange(
                "noiseModDepth",
                parseFloat(e.target.value),
                setNoiseModDepth
              )
            }
          />
        </Form.Group>
        <Form.Group className="mb-3">
          <Form.Label>Update Rate: {noiseUpdateRate} Hz</Form.Label>
          <Form.Range
            min={0.1}
            max={60}
            step={0.1}
            value={noiseUpdateRate}
            onChange={(e) =>
              handleSliderChange(
                "noiseUpdateRate",
                parseFloat(e.target.value),
                setNoiseUpdateRate
              )
            }
          />
        </Form.Group>
        <Form.Group className="text-center mb-3">
          <Button
            variant={manualEnabled ? "danger" : "success"}
            onClick={() => {
              const newManualState = !manualEnabled;
              setManualEnabled(newManualState);
              sendControlChange(
                macAddress,
                toControlName("noiseManualEnabled"),
                newManualState
              );
            }}
          >
            {manualEnabled ? "Turn Off Noise" : "Turn On Noise"}
          </Button>
        </Form.Group>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={handleClose}>
          Close
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default NoiseControlModal;
