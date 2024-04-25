import React, { createContext, useState } from "react";

export const DeviceContext = createContext();

export const DeviceProvider = ({ children }) => {
  const [fanStats, setFanStats] = useState({
    status: { isOnline: 0, lastHeartbeat: 0 },
    function: 0,
    power: "OFF",
    rpm: 0,
  });
  const [ledStats, setLedStats] = useState({
    status: { isOnline: 0, lastHeartbeat: 0 },
    function: 0,
    power: "OFF",
    color: { red: 0, green: 0, blue: 0 },
  });

  return (
    <DeviceContext.Provider
      value={{ fanStats, setFanStats, ledStats, setLedStats }}
    >
      {children}
    </DeviceContext.Provider>
  );
};
