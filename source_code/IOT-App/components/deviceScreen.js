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
const { width, height } = Dimensions.get("window");
import { ws, connectWebSocket } from "./webSocketComponent";
import { styles } from "./homeScreen";
const DeviceScreen = ({ navigation, route }) => {
  const [fanStats, setFanStats] = useState({
    status: { isOnline: 0, lastHeartbeat: 0 },
    function: 0,
    power: 0,
    rpm: 0,
  });
  const [ledStats, setLedStats] = useState({
    status: { isOnline: 0, lastHeartbeat: 0 },
    function: 0,
    power: 0,
    color: {
      red: 0,
      green: 0,
      blue: 0,
    },
  });

  useEffect(() => {
    connectWebSocket(setFanStats, setLedStats);
    return () => {
      if (ws) {
        ws.close();
      }
      clearInterval(messageInterval);
    };
  }, []);

  return (
    <View style={styles.cardContainer}>
      <TouchableOpacity
        style={[styles.DeviceListing, { height: height / 2.5 }]}
        onPress={() => navigation.navigate("Fan", { name: "Welcome" })}
      >
        <Text style={styles.DeviceListingTitle}>Fan</Text>
        <View style={styles.imageRow}>
          <View style={styles.imageView}>
            <Image
              source={require("../assets/images/FanIcon.png")}
              style={styles.icon}
            />
          </View>
        </View>

        <View style={styles.statsColumn}>
          <View style={styles.headerRow}>
            <Text style={styles.DeviceListingText}>Power</Text>
            <Text style={styles.DeviceListingText}>RPM</Text>
          </View>
          <View style={styles.statsRow}>
            <Text style={styles.DeviceListingText}>{fanStats.power}</Text>
            <Text style={styles.DeviceListingText}>{fanStats.rpm}</Text>
          </View>
        </View>
      </TouchableOpacity>
      <TouchableOpacity
        style={[styles.DeviceListing, { height: height / 2.5 }]}
        onPress={() => navigation.navigate("LED", { name: "Welcome" })}
      >
        <Text style={styles.DeviceListingTitle}>RGB Strip</Text>
        <View style={styles.imageRow}>
          <View style={styles.imageView}>
            <Image
              source={require("../assets/images/LEDIcon.png")}
              style={styles.icon}
            />
          </View>
        </View>

        <View style={styles.statsColumn}>
          <View style={styles.headerRow}>
            <Text style={styles.DeviceListingText}>Power</Text>
            <Text style={styles.DeviceListingText}>Color</Text>
          </View>
          <View style={styles.statsRow}>
            <Text style={styles.DeviceListingText}>{ledStats.power}</Text>
            <View
              style={[
                styles.DeviceListingText,
                {
                  width: 40,
                  height: 30,
                  backgroundColor: `rgb(${ledStats.color.red}, ${ledStats.color.green}, ${ledStats.color.blue})`,
                },
              ]}
            />
          </View>
        </View>
      </TouchableOpacity>
    </View>
  );
};

export default DeviceScreen;
