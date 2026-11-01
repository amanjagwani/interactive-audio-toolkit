import React, { useState } from "react";
import { Card, Button, Modal, Form } from "react-bootstrap";

const Step = ({
  stepIndex,
  stepData,
  currentStep,
  onStepChange,
  macAddress,
}) => {
  const [showModal, setShowModal] = useState(false);

  const handleShow = () => setShowModal(true);
  const handleClose = () => setShowModal(false);

  const stepClass =
    stepIndex === currentStep ? "bg-primary text-white" : "bg-light";

  return (
    <>
      <Card
        className={`m-2 p-3 ${stepClass}`}
        style={{ width: "6rem", cursor: "pointer" }}
        onClick={handleShow}
      >
        <Card.Body className="p-0 text-center">
          <Card.Title>{stepIndex + 1}</Card.Title>
          <Card.Text>{`p: ${stepData.pitch}`}</Card.Text>
          <Card.Text>{`v: ${
            typeof stepData.velocity === "number"
              ? stepData.velocity.toFixed(2)
              : stepData.velocity
          }`}</Card.Text>
        </Card.Body>
      </Card>

      <Modal show={showModal} onHide={handleClose}>
        <Modal.Header closeButton>
          <Modal.Title>Step {stepIndex + 1}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group className="mb-3">
              <Form.Label>Pitch: {stepData.pitch}</Form.Label>
              <Form.Range
                min={0}
                max={127}
                value={stepData.pitch ?? 0}
                onChange={(e) =>
                  onStepChange(stepIndex, "pitch", e.target.value)
                }
              />
            </Form.Group>
            <Form.Group className="mb-3">
              <Form.Label>Velocity: {stepData.velocity}</Form.Label>
              <Form.Range
                min={0}
                max={1}
                step={0.01}
                value={stepData.velocity ?? 0}
                onChange={(e) =>
                  onStepChange(stepIndex, "velocity", e.target.value)
                }
              />
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleClose}>
            Close
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

export default Step;
