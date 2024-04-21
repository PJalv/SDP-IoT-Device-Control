const WebSocket = require("ws");
const fs = require("fs");
const jwt = require("jsonwebtoken");
const mqtt = require("mqtt");
const dotenv = require("dotenv");

const path = require("path");
const envPath = path.resolve(__dirname, "../../", ".env");
const storagePath = path.resolve(
  __dirname,
  "../../dashboard_gui",
  "storage.json"
);
dotenv.config({ path: envPath });
const mqtt_client = mqtt.connect(`mqtt://${process.env.BROKER_ADDRESS}`);

mqtt_client.on("connect", () => {
  console.log("Connected to MQTT broker");
});

// Secret key for JWT verification
const secretKey = process.env.JWT_SECRET;

// Generate a JWT token
const token = jwt.sign({}, secretKey);

let ws = null;
let isConnected = false;
let messageInterval = null;
let reconnectTimeout = null; // Variable to keep track of the reconnection attempt

// Function to establish WebSocket connection
function connectWebSocket() {
  if (isConnected) {
    console.log("WebSocket is already connected");
    return;
  }
  // Close existing WebSocket connection if it exists
  if (ws) {
    ws.close();
  }

  // Attempt to establish a new WebSocket connection
  ws = new WebSocket(`wss://pjalv.com/ws?token=${token}&agent=broker`);
  ws.onopen = function () {
    console.log("Connected to WebSocket server");
    isConnected = true;
    messageInterval = setInterval(() => {
      try {
        console.log("Sending storage.json to broker...");
        const data = fs.readFileSync(storagePath);
        const topics = JSON.parse(data);
        ws.send(JSON.stringify(topics));
      } catch (error) {
        console.log("Error reading storage.json:", error);
      }
    }, 1000);
  };

  ws.onmessage = function (event) {
    try {
      const message = JSON.parse(event.data);
      console.log("Broker Received:", message);
      if (message.response !== "ok") {
        console.log("Response not valid!");
      } else {
        const payload =
          typeof message.payload === "object"
            ? message.payload.toString("utf8")
            : message.payload;
        mqtt_client.publish(message.topic, payload);
      }
    } catch (error) {
      console.log("Error parsing message:", error);
    }
  };

  ws.onerror = function (error) {
    console.log("WebSocket error:", error);
  };

  ws.onclose = function () {
    console.log(`Connection closed`);
    isConnected = false;
    clearInterval(messageInterval);
    // Clear previous reconnection attempt before starting a new one
    clearTimeout(reconnectTimeout);
    // Attempt to reconnect after a delay
    reconnectTimeout = setTimeout(connectWebSocket, 5000); // Retry after 5 seconds
  };
}

// Call the connectWebSocket function to initiate the connection
connectWebSocket();
