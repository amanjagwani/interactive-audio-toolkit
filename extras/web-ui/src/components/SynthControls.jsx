import React, { useState } from "react";
import { Button, Modal, Form } from "react-bootstrap";
import { useWebSocket } from "./WebSocketContext";
import useStore from "../store";
import { useEffect } from "react";

function SynthControls({ synthId, synthLabel, macAddress }) {
  const [show, setShow] = useState(false);
  const setSynthSettings = useStore((state) => state.setSynthSettings);
  const synthSettings = useStore(
    (state) => state.synthSettings[synthId] || { ampAdsr: [], modAdsr: [] }
  );
  const { sendControlChange, messages } = useWebSocket();
  const {
    level,
    modIndex,
    modRatio,
    ampAttack,
    ampDecay,
    ampSustain,
    ampRelease,
    modAttack,
    modDecay,
    modSustain,
    modRelease,
  } = synthSettings;

  useEffect(() => {
    if (messages.length > 0) {
      const lastMessage = messages[messages.length - 1];

      if (lastMessage.macAddress === macAddress) {
        const settings = lastMessage.outputSettings?.[synthId];
        if (settings) {
          setSynthSettings(synthId, {
            level: settings.level,
            modIndex: settings.modIndex,
            modRatio: settings.modRatio,

            ampAttack: settings.ampEnv[0],
            ampDecay: settings.ampEnv[1],
            ampSustain: settings.ampEnv[2],
            ampRelease: settings.ampEnv[3],
            modAttack: settings.modEnv[0],
            modDecay: settings.modEnv[1],
            modSustain: settings.modEnv[2],
            modRelease: settings.modEnv[3],
          });
        }
      }
    }
  }, [messages]);

  useEffect(() => {
    sendControlChange(macAddress, `requestSettings`, 1);
  }, [synthId]);

  const handleClose = () => setShow(false);
  const handleShow = () => {
    setShow(true);
    sendControlChange(macAddress, `requestSettings`, 1);
  };

  const handleControlChange = (controlName, value) => {
    const key = controlName.replace(`${synthId}`, "");
    const updatedSettings = {
      ...synthSettings,
      [key]: parseFloat(value),
    };

    setSynthSettings(synthId, updatedSettings);
    sendControlChange(macAddress, `${synthId}-${key}`, parseFloat(value));
  };

  const handleAdsrControlChange = (controlName, value) => {
    const updatedSettings = {
      ...synthSettings,
      [controlName]: parseFloat(value),
    };

    setSynthSettings(synthId, updatedSettings);

    sendControlChange(macAddress, `${synthId}-ampEnv`, [
      updatedSettings.ampAttack,
      updatedSettings.ampDecay,
      updatedSettings.ampSustain,
      updatedSettings.ampRelease,
    ]);
    sendControlChange(macAddress, `${synthId}-modEnv`, [
      updatedSettings.modAttack,
      updatedSettings.modDecay,
      updatedSettings.modSustain,
      updatedSettings.modRelease,
    ]);
  };

  return (
    <>
      <Button variant="primary" onClick={handleShow}>
        Open Synth Controls {synthLabel}
      </Button>

      <Modal show={show} onHide={handleClose} size="lg">
        <Modal.Header closeButton>
          <Modal.Title>Synth Controls</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <div className="d-flex justify-content-around">
            <div>
              <Form.Group className="mb-3">
                <Form.Label>Level: {level}</Form.Label>
                <Form.Range
                  min="0"
                  max="1"
                  step="0.01"
                  value={level}
                  onChange={(e) =>
                    handleControlChange(`level${synthId}`, e.target.value)
                  }
                />
              </Form.Group>
              <Form.Group className="mb-3">
                <Form.Label>Modulation Index: {modIndex}</Form.Label>
                <Form.Range
                  min="0.00"
                  max="20.00"
                  step="0.01"
                  value={modIndex}
                  onChange={(e) =>
                    handleControlChange(`modIndex${synthId}`, e.target.value)
                  }
                />
              </Form.Group>
              <Form.Group className="mb-3">
                <Form.Label>Modulation Ratio: {modRatio}</Form.Label>
                <Form.Range
                  min="0.00"
                  max="10"
                  step="0.01"
                  value={modRatio}
                  onChange={(e) =>
                    handleControlChange(`modRatio${synthId}`, e.target.value)
                  }
                />
              </Form.Group>
              <div>
                <h5>Amplitude ADSR</h5>
                <Form.Group>
                  <Form.Label>Attack {ampAttack}</Form.Label>
                  <Form.Range
                    min="0.00"
                    max="5.00"
                    step="0.001"
                    value={ampAttack || 0.1}
                    onChange={(e) =>
                      handleAdsrControlChange("ampAttack", e.target.value)
                    }
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Decay {ampDecay}</Form.Label>
                  <Form.Range
                    min="0.001"
                    max="5.00"
                    step="0.001"
                    value={ampDecay || 0.1}
                    onChange={(e) =>
                      handleAdsrControlChange("ampDecay", e.target.value)
                    }
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Sustain {ampSustain}</Form.Label>
                  <Form.Range
                    min="0.001"
                    max="1.00"
                    step="0.001"
                    value={ampSustain || 0.5}
                    onChange={(e) =>
                      handleAdsrControlChange("ampSustain", e.target.value)
                    }
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Release {ampRelease}</Form.Label>
                  <Form.Range
                    min="0.001"
                    max="5.00"
                    step="0.001"
                    value={ampRelease || 0.1}
                    onChange={(e) =>
                      handleAdsrControlChange("ampRelease", e.target.value)
                    }
                  />
                </Form.Group>
              </div>
              <div>
                <h5>Modulation ADSR</h5>
                <Form.Group>
                  <Form.Label>Attack {modAttack}</Form.Label>
                  <Form.Range
                    min="0.00"
                    max="5.00"
                    step="0.001"
                    value={modAttack || 0.1}
                    onChange={(e) =>
                      handleAdsrControlChange("modAttack", e.target.value)
                    }
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Decay {modDecay}</Form.Label>
                  <Form.Range
                    min="0.001"
                    max="5.00"
                    step="0.001"
                    value={modDecay || 0.1}
                    onChange={(e) =>
                      handleAdsrControlChange("modDecay", e.target.value)
                    }
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Sustain {modSustain}</Form.Label>
                  <Form.Range
                    min="0.001"
                    max="1.00"
                    step="0.001"
                    value={modSustain || 0.5}
                    onChange={(e) =>
                      handleAdsrControlChange("modSustain", e.target.value)
                    }
                  />
                </Form.Group>
                <Form.Group>
                  <Form.Label>Release {modRelease}</Form.Label>
                  <Form.Range
                    min="0.001"
                    max="5.00"
                    step="0.001"
                    value={modRelease || 0.1}
                    onChange={(e) =>
                      handleAdsrControlChange("modRelease", e.target.value)
                    }
                  />
                </Form.Group>
              </div>{" "}
            </div>
          </div>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleClose}>
            Close
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
}

export default SynthControls;
