import React, { useEffect, useState } from "react";

import {
  Text,
  View,
  StyleSheet,
  ImageBackground,
  TouchableOpacity,
  Dimensions,
  Image,
} from "react-native";
import Slider from "@react-native-community/slider";
import { ws, connectWebSocket } from "./webSocketComponent";
const FanScreen = () => {
  const [speed, setSpeed] = useState(50); // Initial speed value
  const [selectedFunction, setSelectedFunction] = useState("Normal"); // Initial function value

  const handleApplySettings = () => {
    try {
      let topic = "fan/control";
      let response = "ok";
      let payload_format;
      let payload;
      let packet;
      if (selectedFunction === "Normal") {
        payload_format = "INT";
        const intspeed = Math.round(speed);
        payload = intspeed;
        packet = JSON.stringify({ response, topic, payload_format, payload });
        console.log(packet);
        ws.send(packet);
      } else if (selectedFunction === "Breeze") {
        payload_format = "JSON";
        payload = { function: 1 };
        packet = JSON.stringify({ response, topic, payload_format, payload });
        console.log(packet);
        ws.send(packet);
      }
    } catch (error) {
      console.log(error);
    }
  };

  return (
    <View style={styles.container}>
      <View style={styles.option}>
        <Text>Power</Text>
        {/* Add a switch or button for power control */}
      </View>
      <View style={styles.option}>
        <Text>Speed</Text>
        <Slider
          style={{ width: 200, height: 40 }}
          minimumValue={96}
          maximumValue={1024}
          minimumTrackTintColor="#FFFFFF"
          maximumTrackTintColor="#000000"
          value={speed}
          onValueChange={setSpeed}
        />
        <Text>Speed: {speed.toFixed(0)}</Text>
      </View>
      <View style={styles.option}>
        <Text>Function</Text>
        {/* Add a dropdown or radio buttons for function selection */}
        <TouchableOpacity
          onPress={() => setSelectedFunction("Normal")}
          style={[
            styles.functionButton,
            {
              backgroundColor: selectedFunction === "Normal" ? "blue" : "grey",
            },
          ]}
        >
          <Text style={styles.buttonText}>Normal</Text>
        </TouchableOpacity>
        <TouchableOpacity
          onPress={() => setSelectedFunction("Breeze")}
          style={[
            styles.functionButton,
            {
              backgroundColor: selectedFunction === "Breeze" ? "blue" : "grey",
            },
          ]}
        >
          <Text style={styles.buttonText}>Breeze</Text>
        </TouchableOpacity>
      </View>
      <TouchableOpacity
        style={styles.applyButton}
        onPress={handleApplySettings}
      >
        <Text style={styles.buttonText}>Apply Settings</Text>
      </TouchableOpacity>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
  },
  option: {
    marginBottom: 20,
  },
  functionButton: {
    backgroundColor: "grey",
    padding: 10,
    borderRadius: 5,
    marginTop: 5,
  },
  applyButton: {
    backgroundColor: "blue",
    padding: 10,
    borderRadius: 5,
    marginTop: 20,
  },
  buttonText: {
    color: "white",
    textAlign: "center",
  },
});

export default FanScreen;
