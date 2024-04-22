import React, { useEffect, useState } from "react";
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
  const [speed, setSpeed] = useState(50); // Initial speed value
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

  const customSwatches = new Array(6)
    .fill("#fff")
    .map(() => colorKit.randomRgbColor().hex());

  const selectedColor = useSharedValue(customSwatches[0]);
  const backgroundColorStyle = useAnimatedStyle(() => ({
    backgroundColor: selectedColor.value,
  }));

  const onColorSelect = (color) => {
    setSelectedFunction("Normal");
    // "worklet";
    selectedColor.value = color.hex;
    console.log(selectedColor.value);
    const rgbColor = hexToRgb(selectedColor.value);
    console.log({ r: rgbColor.r, g: rgbColor.g, b: rgbColor.b });
  };
  return (
    <View style={styles.container}>
      <View style={styles.colorPickerContainer}>
        <>
          <View style={styles.pickerContainer}>
            <ColorPicker
              value={selectedColor.value}
              sliderThickness={90}
              thumbSize={54}
              onChange={onColorSelect}
              //   boundedThumb
            >
              <HueCircular
                containerStyle={styles.hueContainer}
                thumbShape="circle"
              ></HueCircular>

              <View style={styles.previewTxtContainer}>
                <PreviewText style={{ color: "#707070" }} colorFormat="rgb" />
              </View>
            </ColorPicker>
          </View>
        </>

        {/* <Button title="Ok" onPress={() => setShowModal(false)} /> */}
      </View>
      <View style={styles.settingsContainer}>
        <View style={styles.option}>
          <Text>Power</Text>
          {/* Add a switch or button for power control */}
        </View>
        <View style={styles.option}>
          <Text>Function</Text>
          {/* Add a dropdown or radio buttons for function selection */}
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
    paddingTop: 50,
  },
  colorPickerContainer: {
    flex: 1,
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
});

export default LedScreen;
