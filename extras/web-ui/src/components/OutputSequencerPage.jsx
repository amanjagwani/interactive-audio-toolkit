import React from "react";
import Sequencer from "./Sequencer";
import { Container } from "react-bootstrap";
import { useParams } from "react-router-dom";
import { useEffect } from "react";
import { useWebSocket } from "./WebSocketContext";

function OutputSequencerPage() {
  const { activationId, macAddress, outputId } = useParams();
  const { sendGlobalActivationMessage } = useWebSocket();

  useEffect(() => {
    sendGlobalActivationMessage(`requestSequencer-${activationId}`, null);
  }, []);

  return (
    <Container className="my-5">
      <h2>{`${activationId}-${outputId.toUpperCase()} Output Sequencer`}</h2>
      <Sequencer
        seqId={`${activationId}-`}
        thresholdType={"none"}
        macAddress={macAddress}
        context="output"
        outputId={outputId}
      />
    </Container>
  );
}

export default OutputSequencerPage;
