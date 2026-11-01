import React, { useEffect, useState } from "react";
import { Link, useNavigate } from "react-router-dom";
import { useWebSocket } from "./WebSocketContext";
import useStore from "../store";
import {
  Container,
  Button,
  Accordion,
  Card,
  Form,
  InputGroup,
} from "react-bootstrap";

function HomePage() {
  const { messages, sendControlChange, sendGlobalControlChange } =
    useWebSocket();
  const devices = useStore((state) => state.devices);
  const setDevices = useStore((state) => state.setDevices);
  const connectedDevices = Object.values(devices);
  const [editingDevice, setEditingDevice] = useState(null);
  const [customNameInput, setCustomNameInput] = useState("");
  const [wifiSsid, setWifiSsid] = useState("");
  const [wifiPassword, setWifiPassword] = useState("");
  const navigate = useNavigate();

  useEffect(() => {
    sendGlobalControlChange(`requestDevices`, 1);
  }, []);

  const handleSaveName = (macAddress) => {
    setDevices([{ macAddress, customName: customNameInput }]);

    sendControlChange(macAddress, "customName", customNameInput);

    setEditingDevice(null);
    setCustomNameInput("");
  };

  const handleEditClick = (macAddress, currentName) => {
    setEditingDevice(macAddress);
    setCustomNameInput(currentName || "");
  };

  return (
    <Container className="mt-5 text-center">
      <h1>Connected Devices</h1>
      <div className="mt-4">
        {connectedDevices?.length > 0 ? (
          connectedDevices.map((device, index) => (
            <Card key={device.macAddress} className="mb-4 shadow-sm">
              <Card.Body>
                <div className="d-flex align-items-center justify-content-between">
                  <h5 className="mb-0">
                    Device:{" "}
                    {editingDevice === device.macAddress ? (
                      <InputGroup className="custom-name-input d-flex align-items-center">
                        <Form.Control
                          type="text"
                          placeholder="Enter custom name"
                          value={customNameInput}
                          className="me-2"
                          onChange={(e) => setCustomNameInput(e.target.value)}
                          onKeyDown={(e) => {
                            if (e.key === "Enter") {
                              handleSaveName(device.macAddress);
                            }
                          }}
                        />
                        <Button
                          variant="success"
                          className="me-2"
                          onClick={() => handleSaveName(device.macAddress)}
                        >
                          Save
                        </Button>
                        <Button
                          variant="secondary"
                          onClick={() => {
                            setEditingDevice(null);
                            setCustomNameInput("");
                          }}
                        >
                          Cancel
                        </Button>
                      </InputGroup>
                    ) : device.customName ? (
                      <>
                        {device.customName}
                        <Button
                          variant="link"
                          className="ms-2 p-0"
                          onClick={() =>
                            handleEditClick(device.macAddress, device.customName)
                          }
                        >
                          Edit
                        </Button>
                      </>
                    ) : (
                      <>
                        {device.macAddress}
                        <Button
                          variant="link"
                          className="ms-2 p-0"
                          onClick={() => handleEditClick(device.macAddress, "")}
                        >
                          Add Name
                        </Button>
                      </>
                    )}
                  </h5>
                  <Link
                    to={`/device/${device.macAddress}`}
                    state={{ customName: device.customName }}
                    className="btn btn-primary"
                  >
                    View Device
                  </Link>
                </div>
              </Card.Body>
              {device.sensors?.length > 0 && (
                <Accordion>
                  <Accordion.Item eventKey={`${index}`}>
                    <Accordion.Header>Sensors</Accordion.Header>
                    <Accordion.Body>
                      {device.sensors.map((sensor) => (
                        <div key={sensor.sensorId} className="mb-3">
                          <strong>{sensor.sensorId}</strong>
                          {sensor.seqIds?.length > 0 ? (
                            <ul>
                              {sensor.seqIds.map((seqId) => (
                                <li key={seqId}>{seqId}</li>
                              ))}
                            </ul>
                          ) : (
                            <p className="text-muted small">
                              No thresholds enabled
                            </p>
                          )}
                        </div>
                      ))}
                    </Accordion.Body>
                  </Accordion.Item>
                </Accordion>
              )}
            </Card>
          ))
        ) : (
          <p>No devices connected</p>
        )}
      </div>
      <Card className="mt-4 shadow-sm mx-auto" style={{ maxWidth: 400 }}>
        <Card.Body>
          <h5>WiFi settings</h5>
          <Form.Control
            className="mb-2"
            placeholder="SSID"
            value={wifiSsid}
            onChange={(e) => setWifiSsid(e.target.value)}
          />
          <Form.Control
            className="mb-2"
            type="password"
            placeholder="Password"
            value={wifiPassword}
            onChange={(e) => setWifiPassword(e.target.value)}
          />
          <Button
            onClick={() =>
              sendGlobalControlChange("wifiCredentials", {
                ssid: wifiSsid,
                password: wifiPassword,
              })
            }
          >
            Save and reboot
          </Button>
        </Card.Body>
      </Card>
      <div className="d-flex flex-column align-items-center mt-4">
        <Button
          className="mt-4 btn btn-secondary"
          onClick={() => navigate("/global", { state: { connectedDevices } })}
        >
          Global Processing
        </Button>
      </div>
    </Container>
  );
}

export default HomePage;
