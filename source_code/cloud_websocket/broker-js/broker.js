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

let ws;

// Function to establish WebSocket connection
function connectWebSocket() {
  ws = new WebSocket(
    `ws://${process.env.SERVER_ADDRESS}:8080/ws?token=${token}&agent=broker`
  );

  ws.on("open", function open() {
    console.log("Connected to WebSocket server");
    setInterval(() => {
      try {
        const data = fs.readFileSync(storagePath);
        const topics = JSON.parse(data);
        ws.send(JSON.stringify(topics));
      } catch (error) {
        console.log("Error reading storage.json:", error);
      }
    }, 1000);
  });

  ws.on("message", function incoming(message) {
    try {
      message = JSON.parse(message.toString("utf8"));
      console.log("Broker Received:", message);
      if (message.response !== "ok") {
        console.log("Response not valid!");
      } else {
        if (typeof message.payload === "object") {
          mqtt_client.publish(message.topic, message.payload.toString("utf8"));
        } else {
          mqtt_client.publish(message.topic, message.payload);
        }
      }
    } catch (error) {
      console.log("Error parsing message:", error);
    }
  });

  ws.on("error", (error) => {
    console.log("WebSocket error:", error);
    // Attempt to reconnect after a delay
    setTimeout(connectWebSocket, 5000); // Retry after 5 seconds
  });

  ws.on("close", function close() {
    console.log(`Connection closed`);
    // Attempt to reconnect after a delay
    setTimeout(connectWebSocket, 5000); // Retry after 5 seconds
  });
}

// Initial WebSocket connection
connectWebSocket();
