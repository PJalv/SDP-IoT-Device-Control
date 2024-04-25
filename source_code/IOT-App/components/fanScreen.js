import React, { useContext, useEffect, useState } from "react";

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
import RnVerticalSlider from "rn-vertical-slider";
import { ws, connectWebSocket } from "./webSocketComponent";
import { DeviceContext } from "./deviceContext";
import { GestureHandlerRootView } from "react-native-gesture-handler";
const FanScreen = () => {
  const { fanStats } = useContext(DeviceContext);
  const [selectedFunction, setSelectedFunction] = useState(
    fanStats.function === 1 ? "Breeze" : "Normal"
  ); // Initial function value

  const [dutyCycle, setdutyCycle] = useState(
    Math.round((fanStats.rpm / 1800) * (1024 - 96) + 96)
  ); // Initial dutyCycle value
  console.log(fanStats.rpm / 1800);
  const handleApplySettings = () => {
    try {
      let topic = "fan/control";
      let response = "ok";
      let payload_format;
      let payload;
      let packet;
      console.log(selectedFunction);
      if (selectedFunction === "Normal") {
        payload_format = "INT";
        const intdutyCycle = Math.round(dutyCycle);
        payload = intdutyCycle;
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
    <View
      style={[
        styles.container,
        fanStats.status.isOnline !== "Online" ? styles.OFFLINE : null,
      ]}
    >
      <Text style={styles.optionText}>Speed</Text>
      <GestureHandlerRootView
        style={{ flex: 1, justifyContent: "center", alignItems: "center" }}
        onTouchStart={() => setSelectedFunction("Normal")}
      >
        <RnVerticalSlider
          value={Math.round((fanStats.rpm / 1800) * (1024 - 96) + 96)}
          onChange={() => {
            setSelectedFunction("Normal");
          }}
          onComplete={(newValue) => {
            setSelectedFunction("Normal");
            setdutyCycle(newValue);
          }}
          height={350}
          width={80}
          step={1}
          min={96}
          max={1024}
          borderRadius={5}
          minimumTrackTintColor="#2979FF"
          maximumTrackTintColor="#D1D1D6"
          showBallIndicator={true}
          ballIndicatorColor="#2979FF"
          ballIndicatorTextColor="#fff"
          ballIndicatorWidth={80}
          ballIndicatorHeight={40}
          renderIndicator={(value) => (
            <Text style={styles.dutyCycleText}>{`${Math.round(
              (value.toFixed(0) / 1024) * 100
            )}%`}</Text>
          )}
        />
      </GestureHandlerRootView>
      <View style={styles.settingsContainer}>
        <View style={styles.option}>
          <TouchableOpacity
            onPress={() => {
              let topic = "fan/control/power";
              let response = "ok";
              let payload_format = "INT";
              let payload;
              let packet;
              if (fanStats.power === "ON") {
                payload = 0;
              } else {
                payload = 1;
              }
              packet = JSON.stringify({
                response,
                topic,
                payload_format,
                payload,
              });
              ws.send(packet);
            }}
            style={[
              styles.functionButton,
              {
                backgroundColor: fanStats.power === "ON" ? "blue" : "grey",
              },
            ]}
          >
            <Text style={styles.buttonText}>{fanStats.power}</Text>
          </TouchableOpacity>
          <Text style={styles.optionText}>Function</Text>
          <TouchableOpacity
            onPress={() => {
              if (selectedFunction === "Breeze") {
                setSelectedFunction("Normal");
              } else {
                setSelectedFunction("Breeze");
              }
            }}
            style={[
              styles.functionButton,
              {
                backgroundColor:
                  selectedFunction === "Breeze" ? "blue" : "grey",
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
    paddingTop: 10,
    marginBottom: -90,
  },
  dutyCycleText: {
    fontSize: 20,
    textAlign: "center",
  },
  settingsContainer: {
    flex: 1,
    paddingHorizontal: 20,
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
  optionText: {
    marginTop: 10,
    fontSize: 17,
    fontWeight: "bold",
    textAlign: "center",
  },
  OFFLINE: {
    backgroundColor: "rgba(0,0,0,0.4)",
    pointerEvents: "none",
  },
});

export default FanScreen;
