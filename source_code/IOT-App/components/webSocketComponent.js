import jwt from "expo-jwt";

const jwtSecret = process.env.EXPO_PUBLIC_JWT_SECRET;
let token = jwt.encode({}, jwtSecret, { algorithm: "HS256" });
console.log(token);
export let ws = null;
let messageInterval = null;
let isConnected = false;
let reconnectTimeout = null;
export const connectWebSocket = (setFanStats, setLedStats) => {
  if (isConnected) {
    console.log("WebSocket is already connected.");
    return;
  }

  if (ws) {
    ws.close();
  }
  console.log("Attempting to connect to WebSocket server...");
  const serverIP = process.env.EXPO_PUBLIC_SERVER_ADDRESS;

  ws = new WebSocket(`wss://${serverIP}/ws?token=${token}&agent=client`);

  ws.onopen = () => {
    console.log("WebSocket Connection Established...");
    isConnected = true;
  };

  ws.onmessage = (event) => {
    const message = JSON.parse(event.data);
    console.log(message);
    try {
      setFanStats({
        status: {
          isOnline:
            message["fan-device"].status.isOnline === 1 ? "Online" : "Offline",
          lastHeartbeat: message["fan-device"].status.lastHeartbeat,
        },
        function: message["fan-device"].function,
        power: message["fan-device"].power === 1 ? "ON" : "OFF",
        rpm: message["fan-device"].rpm,
      });
      setLedStats({
        status: {
          isOnline:
            message["led-device"].status.isOnline === 1 ? "Online" : "Offline",
          lastHeartbeat: message["led-device"].status.lastHeartbeat,
        },
        function: message["led-device"].function,
        power: message["led-device"].power === 1 ? "ON" : "OFF",
        color: {
          red: message["led-device"].color.red,
          green: message["led-device"].color.green,
          blue: message["led-device"].color.blue,
        },
      });
    } catch (error) {
      console.log("Error parsing message:", error);
    }
  };

  ws.onerror = (error) => {
    console.log(`WebSocket error: ${error.message}`);
    ws = null;
    isConnected = false;
  };

  ws.onclose = () => {
    console.log("WebSocket Connection Closed...");
    clearInterval(messageInterval);
    clearInterval(reconnectTimeout);
    isConnected = false;
    reconnectTimeout = setTimeout(connectWebSocket, 3000);
  };
};
