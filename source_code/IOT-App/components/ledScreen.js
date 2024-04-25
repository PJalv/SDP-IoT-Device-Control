import React, { useContext, useEffect, useState } from "react";
import {
  Text,
  View,
  StyleSheet,
  TouchableOpacity,
  Modal,
  Button,
  Pressable,
} from "react-native";
import ColorPicker, {
  Panel1,
  Swatches,
  Preview,
  OpacitySlider,
  HueSlider,
  HueCircular,
  PreviewText,
  colorKit,
} from "reanimated-color-picker";
import Animated, {
  useAnimatedStyle,
  useSharedValue,
} from "react-native-reanimated";
import { ws, connectWebSocket } from "./webSocketComponent";
import { DeviceContext } from "./deviceContext";
function hexToRgb(hex) {
  // Remove '#' if present
  hex = hex.replace("#", "");

  // Convert hex to RGB
  let r = parseInt(hex.substring(0, 2), 16);
  let g = parseInt(hex.substring(2, 4), 16);
  let b = parseInt(hex.substring(4, 6), 16);

  // Return RGB as an object
  return { r, g, b };
}
const LedScreen = () => {
  const { ledStats } = useContext(DeviceContext);
  const [selectedFunction, setSelectedFunction] = useState("Normal"); // Initial function value
  const [showModal, setShowModal] = useState(false);
  const handleApplySettings = () => {
    try {
      let topic = "led/control/color";
      let response = "ok";
      let payload_format = "JSON";
      let payload;
      let packet;
      console.log(selectedFunction);
      if (selectedFunction === "Normal") {
        payload = hexToRgb(selectedColor.value);
        payload = { red: payload.r, green: payload.g, blue: payload.b };
        packet = JSON.stringify({ response, topic, payload_format, payload });
        console.log(packet);
        ws.send(packet);
      } else {
        if (selectedFunction === "Static Rainbow") {
          payload = { function: 1 };
        } else {
          payload = { function: 2 };
        }
        packet = JSON.stringify({ response, topic, payload_format, payload });
        ws.send(packet);
      }
    } catch (error) {
      console.log(error);
    }
  };

  const customSwatches = new Array(6).fill("#fff").map(() =>
    colorKit
      .randomRgbColor({
        r: [255, 255],
        g: [0, 0],
        b: [255, 255],
      })
      .hex()
  );
  let selectedColor = useSharedValue(customSwatches[0]);
  // selectedColor.value = `rgb(${ledStats.color.red}, ${ledStats.color.green},${ledStats.color.blue})`;
  const backgroundColorStyle = useAnimatedStyle(() => ({
    backgroundColor: selectedColor.value,
  }));

  const onColorSelect = (color) => {
    setSelectedFunction("Normal");
    selectedColor.value = color.hex;
    const rgbColor = hexToRgb(selectedColor.value);
  };
  let powerButton;
  return (
    <View
      style={[
        styles.container,
        ledStats.status.isOnline !== "Online" ? null : styles.OFFLINE,
      ]}
    >
      <View style={styles.colorPickerContainer}>
        <>
          <ColorPicker
            value={selectedColor.value} //
            thumbSize={40}
            onChange={onColorSelect}
            //   boundedThumb
          >
            <HueCircular
              containerStyle={styles.hueContainer}
              thumbShape="circle"
              sliderThickness={90}
            ></HueCircular>
          </ColorPicker>
        </>
        {/* <Button title="Ok" onPress={() => setShowModal(false)} /> */}
      </View>
      <View style={styles.settingsContainer}>
        <View style={styles.option}>
          <TouchableOpacity
            onPress={() => {
              let topic = "led/control/power";
              let response = "ok";
              let payload_format = "INT";
              let payload;
              let packet;
              if (ledStats.power === "ON") {
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
                backgroundColor: ledStats.power === "ON" ? "blue" : "grey",
              },
            ]}
          >
            <Text style={styles.buttonText}>{ledStats.power}</Text>
          </TouchableOpacity>
          <Text style={styles.optionText}>Function</Text>
          <TouchableOpacity
            onPress={() => {
              if (selectedFunction === "Static Rainbow") {
                setSelectedFunction("Normal");
              } else {
                setSelectedFunction("Static Rainbow");
              }
            }}
            style={[
              styles.functionButton,
              {
                backgroundColor:
                  selectedFunction === "Static Rainbow" ? "blue" : "grey",
              },
            ]}
          >
            <Text style={styles.buttonText}>Static Rainbow</Text>
          </TouchableOpacity>
          <TouchableOpacity
            onPress={() => {
              if (selectedFunction === "Trailing Rainbow") {
                setSelectedFunction("Normal");
              } else {
                setSelectedFunction("Trailing Rainbow");
              }
            }}
            style={[
              styles.functionButton,
              {
                backgroundColor:
                  selectedFunction === "Trailing Rainbow" ? "blue" : "grey",
              },
            ]}
          >
            <Text style={styles.buttonText}>Trailing Rainbow</Text>
          </TouchableOpacity>
        </View>
        <TouchableOpacity
          style={styles.applyButton}
          onPress={handleApplySettings}
        >
          <Text style={styles.buttonText}>Apply Settings</Text>
        </TouchableOpacity>
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    paddingTop: 10,
    marginBottom: -90,
  },
  colorPickerContainer: {
    flex: 1,
    alignSelf: "center",
    width: "95%",
    backgroundColor: "#fff",
    padding: 20,
    borderRadius: 20,
    shadowColor: "#000",
    shadowOffset: {
      width: 0,
      height: 5,
    },
    shadowOpacity: 0.34,
    shadowRadius: 6.27,

    elevation: 10,
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
});

export default LedScreen;
