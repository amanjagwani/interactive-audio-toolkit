import React from "react";
import Sequencer from "./Sequencer";
import { Container } from "react-bootstrap";
import { useParams, useLocation } from "react-router-dom";

function SequencerPage() {
  const location = useLocation();
  const { macAddress, id } = useParams();

  return (
    <Container className="my-5">
      <h2>{id.toUpperCase()} </h2>
      <Sequencer
        seqId={id}
        thresholdType={location.state.thresholdType}
        macAddress={macAddress}
        context="sensorThreshold"
        outputId={null}
      />
    </Container>
  );
}

export default SequencerPage;
