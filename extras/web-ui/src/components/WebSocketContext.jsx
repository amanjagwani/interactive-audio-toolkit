import React, { createContext, useContext, useEffect, useState } from "react";
import useStore from "../store";

const WebSocketContext = createContext(null);

export const WebSocketProvider = ({ children }) => {
  const [ws, setWs] = useState(null);
  const [messages, setMessages] = useState([]);

  function sendControlChange(macAddress, controlName, value) {
    if (!macAddress) {
      console.error("macAddress is undefined in sendControlChange");
      return;
    }

    if (ws) {
      ws.send(JSON.stringify({ macAddress, controlName, value }));
    } else {
      console.error("WebSocket is not available");
    }
  }

  function sendGlobalControlChange(controlName, value) {
    if (ws) {
      ws.send(JSON.stringify({ controlName, value }));
    } else {
      console.error("WebSocket is not available");
    }
  }

  function sendControlChangeToServer(macAddress, id, controlName, value) {
    if (!macAddress) {
      console.error("macAddress is undefined in sendControlChangeToServer");
      return;
    }

    if (ws) {
      ws.send(
        JSON.stringify({ macAddress, controlName: `${id}-${controlName}`, value })
      );
    } else {
      console.error("WebSocket is not available");
    }
  }

  function sendGlobalActivationMessage(action, activation) {
    if (!ws) {
      console.error("WebSocket is not available");
      return;
    }

    ws.send(
      JSON.stringify({
        controlName: "globalActivation",
        value: { action, activation },
      })
    );
  }

  useEffect(() => {
    const webSocket = new WebSocket(`ws://${window.location.host}/ws`);

    webSocket.onopen = () => {
      console.log("WebSocket Connection Established");
      webSocket.send(JSON.stringify({ controlName: "requestDevices", value: 1 }));
      webSocket.send(JSON.stringify({ controlName: "requestSettings", value: 1 }));
    };

    webSocket.onmessage = (e) => {
      const newMessage = JSON.parse(e.data);

      if (newMessage.type === "connectedDevices") {
        useStore.getState().setDevices(newMessage.data);
      }

      if (!("triggerRate" in newMessage) && !("deviceTriggerRate" in newMessage)) {
        console.log("Message from server: ", e.data);
      }

      setMessages((prevMessages) => [...prevMessages, newMessage].slice(-100));
    };

    setWs(webSocket);

    return () => webSocket.close();
  }, []);

  return (
    <WebSocketContext.Provider
      value={{
        ws,
        messages,
        sendControlChange,
        sendControlChangeToServer,
        sendGlobalActivationMessage,
        sendGlobalControlChange,
      }}
    >
      {children}
    </WebSocketContext.Provider>
  );
};

export const useWebSocket = () => useContext(WebSocketContext);
