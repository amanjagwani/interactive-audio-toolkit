import React from "react";
import { Line } from "react-chartjs-2";
import "chart.js/auto";

const SensorGraph = ({ id, sensorData, maxVal }) => {
  const numericMaxVal = parseInt(maxVal, 10);
  const options = {
    scales: {
      y: {
        min: 0,
        max: Math.ceil(numericMaxVal * 1.1),
        beginAtZero: true,
      },
    },
    elements: {
      line: {
        tension: 0.1,
      },
      point: {
        radius: 0,
      },
    },
    animation: {
      duration: 0,
    },
    maintainAspectRatio: false,
  };

  return <Line data={sensorData} options={options} />;
};

export default SensorGraph;
