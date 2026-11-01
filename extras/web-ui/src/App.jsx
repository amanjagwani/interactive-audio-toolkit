import React from "react";
import "bootstrap/dist/css/bootstrap.min.css";
import {
  BrowserRouter as Router,
  Routes,
  Route,
  Link,
} from "react-router-dom";
import { WebSocketProvider } from "./components/WebSocketContext";
import MainPage from "./components/MainPage";
import SensorPage from "./components/SensorPage";
import SequencerPage from "./components/SequencerPage";
import HomePage from "./components/HomePage";
import GlobalProcessing from "./components/GlobalProcessing";
import OutputSequencerPage from "./components/OutputSequencerPage";

const Navbar = () => (
  <nav className="navbar navbar-light bg-light">
    <span className="navbar-brand mb-0 h1 ms-3">Interactive Audio Toolkit</span>
  </nav>
);

function App() {
  return (
    <Router>
      <Navbar />
      <WebSocketProvider>
        <Routes>
          <Route path="/" element={<HomePage />} />
          <Route path="/global" element={<GlobalProcessing />} />
          <Route path="/device/:macAddress" element={<MainPage />} />
          <Route path="/device/:macAddress/sensor/:id" element={<SensorPage />} />
          <Route path="/device/:macAddress/sequencer/:id" element={<SequencerPage />} />
          <Route
            path="/:activationId/:macAddress/:outputId/sequencer"
            element={<OutputSequencerPage />}
          />
        </Routes>
      </WebSocketProvider>
    </Router>
  );
}

export default App;
