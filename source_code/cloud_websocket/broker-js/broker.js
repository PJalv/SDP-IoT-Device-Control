const WebSocket = require("ws");
const jwt = require("jsonwebtoken");
const mqtt = require("mqtt");
const dotenv = require("dotenv");

const path = require("path");
const customPath = path.resolve(__dirname, "../../", ".env");

dotenv.config({ path: customPath });
const mqtt_client = mqtt.connect(`mqtt://${process.env.BROKER_ADDRESS}`);

mqtt_client.on("connect", () => {
  console.log("Connected to MQTT broker");
});

// Secret key for JWT verification
const secretKey = process.env.JWT_SECRET;

// Generate a JWT token
const token = jwt.sign({}, secretKey);

// WebSocket URL
const ws = new WebSocket(
  `ws://${process.env.SERVER_ADDRESS}:8080/ws?token=${token}&agent=broker`
);

ws.on("open", function open() {
  console.log("Connected to WebSocket server");
  setInterval(() => {
    // test payload for now
    let payload = {
      device: "fan",
      power: "ON",
      rpm: Math.floor(Math.random() * 1000),
    };
    ws.send(JSON.stringify(payload));
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
});

ws.on("close", function close() {
  console.log(`Connection closed`);
  return;
});
