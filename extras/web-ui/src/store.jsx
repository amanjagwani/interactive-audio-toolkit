import { create } from "zustand";

const globalActivation = {
  id: "",
  input: {
    type: "",
    timeOfDay: {
      hour: null,
      minute: null,
    },
    sensorThreshold: {
      device: null,
      sensor: null,
      threshold: null,
    },
    weather: {
      parameter: null,
      condition: null,
      value: null,
    },
  },
  output: {
    type: "",
    sequencers: [],
    output: {
      type: null,
    },
  },
  enabled: false,
};

const useStore = create((set) => ({
  sequencers: {},
  sensors: {},
  bpm: 120,

  synthSettings: {},

  deviceStates: {},
  setDeviceState: (macAddress, newState) =>
    set((state) => ({
      deviceStates: {
        ...state.deviceStates,
        [macAddress]: {
          ...state.deviceStates[macAddress],
          ...newState,
        },
      },
    })),

  setSynthSettings: (synthId, settings) =>
    set((state) => ({
      synthSettings: {
        ...state.synthSettings,
        [synthId]: {
          ampAdsr: settings.ampAdsr || [],
          modAdsr: settings.modAdsr || [],
          ...settings,
        },
      },
    })),

  setBpm: (newState) => set(() => ({ bpm: newState })),

  setSequencerState: (id, newState) =>
    set((prevState) => ({
      sequencers: {
        ...prevState.sequencers,
        [id]: {
          ...(prevState.sequencers[id] || {}),
          ...newState,
        },
      },
    })),

  setSensorState: (id, newState) =>
    set((prevState) => ({
      sensors: {
        ...prevState.sensors,
        [id]: {
          ...(prevState.sensors[id] || {}),
          ...newState,
        },
      },
    })),

  updateStep: (id, index, type, value) =>
    set((state) => {
      const sequencer = state.sequencers[id];
      const updatedSteps = [...sequencer.steps];
      updatedSteps[index] = { ...updatedSteps[index], [type]: value };
      return {
        sequencers: {
          ...state.sequencers,
          [id]: { ...sequencer, steps: updatedSteps },
        },
      };
    }),

  sendStepUpdates: (id, sendControlChange, macAddress) => {
    const sequencer = useStore.getState().sequencers[id];
    sendControlChange(
      macAddress,
      `${id}-pitches`,
      sequencer.steps.map((step) => step.pitch)
    );
    sendControlChange(
      macAddress,
      `${id}-velocities`,
      sequencer.steps.map((step) => step.velocity)
    );
  },

  setThreshold: (id, thresholdType, value) =>
    set((state) => ({
      sensors: {
        ...state.sensors,
        [id]: {
          ...state.sensors[id],
          [`${thresholdType}Thres`]: value,
        },
      },
    })),

  setPlotting: (id, plotting) =>
    set((state) => ({
      sensors: {
        ...state.sensors,
        [id]: {
          ...state.sensors[id],
          isPlotting: plotting,
        },
      },
    })),

  setEnabled: (id, enabled) =>
    set((state) => ({
      sensors: {
        ...state.sensors,
        [id]: {
          ...state.sensors[id],
          isEnabled: enabled,
        },
      },
    })),

  sensorRanges: {},

  devices: {},
  setDevices: (list) =>
    set((state) => {
      const devices = { ...state.devices };
      const sensorRanges = { ...state.sensorRanges };
      list.forEach((device) => {
        devices[device.macAddress] = { ...devices[device.macAddress], ...device };
        (device.sensors || []).forEach((sensor) => {
          sensorRanges[sensor.sensorId.toUpperCase()] = {
            min: 0,
            max: sensor.fullScale,
            step: 1,
          };
        });
      });
      return { devices, sensorRanges };
    }),

  globalActivations: [],
  selectedActivation: null,

  setGlobalActivation: (index, updatedActivation) =>
    set((state) => {
      const newActivations = [...state.globalActivations];
      newActivations[index] = {
        ...state.globalActivations[index],
        ...updatedActivation,
      };
      return { globalActivations: newActivations };
    }),

  addGlobalActivation: (activation = null) =>
    set((state) => {
      if (state.globalActivations.some((a) => a.id === activation?.id)) {
        return state;
      }

      const defaultActivation = {
        id: activation?.id || "",
        priorityLevel: activation?.priorityLevel || null,
        input: {
          type: activation?.inputType || "",
          timeOfDay: {
            hour: activation?.input?.timeOfDay?.hour || null,
            minute: activation?.input?.timeOfDay?.minute || null,
          },
          sensorThreshold: {
            thresholds: activation?.input?.sensorThreshold?.thresholds || [],
          },
          weather: {
            parameter: activation?.input?.weather?.parameter || null,
            condition: activation?.input?.weather?.condition || null,
            value: activation?.input?.weather?.value || null,
          },
          prayerTime: activation?.input?.prayerTime || null,
        },
        output: {
          type: activation?.outputType || "",
          sequencers: activation?.output?.sequencers || [],
          outputId: activation?.output?.outputId || null,
          noiseDevice: activation?.output?.noiseDevice || null,
          noiseLength: activation?.output?.noiseLength || null,
        },
        enabled: activation?.enabled || false,
      };

      return {
        globalActivations: [...state.globalActivations, defaultActivation],
      };
    }),

  deleteGlobalActivation: (index) =>
    set((state) => ({
      globalActivations: state.globalActivations.filter(
        (_, idx) => idx !== index
      ),
    })),

  setSelectedActivation: (activation) =>
    set(() => ({
      selectedActivation: activation,
    })),
}));

export default useStore;
