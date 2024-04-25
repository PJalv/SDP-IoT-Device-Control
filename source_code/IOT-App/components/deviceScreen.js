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
const { width, height } = Dimensions.get("window");
import { DeviceContext } from "./deviceContext";
import { ws, connectWebSocket } from "./webSocketComponent";
import { styles } from "./homeScreen";
const DeviceScreen = ({ navigation, route }) => {
  const { fanStats, setFanStats, ledStats, setLedStats } =
    useContext(DeviceContext);
  useEffect(() => {
    // if (ws) {
    //   ws.close();
    // }
    connectWebSocket(setFanStats, setLedStats);

    return () => {};
  }, [setFanStats, setLedStats]);

  return (
    <View style={styles.cardContainer}>
      <TouchableOpacity
        style={[
          styles.DeviceListing,
          { height: height / 2.5 },
          fanStats.status.isOnline !== "Online" ? styles.OFFLINE : null,
        ]}
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
            <Text style={styles.DeviceListingText}>
              {fanStats.function === 0 ? "RPM" : "Function"}
            </Text>
          </View>
          <View style={styles.statsRow}>
            <Text style={styles.DeviceListingText}>{fanStats.power}</Text>
            {fanStats.function === 0 ? (
              <Text style={styles.DeviceListingText}>{fanStats.rpm}</Text>
            ) : fanStats.function === 1 ? (
              <Text style={styles.DeviceListingText}>Breeze</Text>
            ) : null}
          </View>
        </View>
      </TouchableOpacity>

      <TouchableOpacity
        style={[
          styles.DeviceListing,
          { height: height / 2.5 },
          ledStats.status.isOnline !== "Online" ? styles.OFFLINE : null,
        ]}
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
            <Text style={styles.DeviceListingText}>
              {ledStats.function === 0 ? "Color" : "Function"}
            </Text>
          </View>
          <View style={styles.statsRow}>
            <Text style={styles.DeviceListingText}>{ledStats.power}</Text>
            {ledStats.function === 0 ? (
              <View
                style={[
                  styles.DeviceListingText,
                  {
                    width: 40,
                    height: 30,
                    backgroundColor: `rgb(${ledStats.color.red}, ${ledStats.color.green}, ${ledStats.color.blue})`,
                  },
                ]}
              ></View>
            ) : ledStats.function === 1 ? (
              <Text style={styles.DeviceListingText}>Static Rainbow</Text>
            ) : ledStats.function === 2 ? (
              <Text style={styles.DeviceListingText}>Trailing Rainbow</Text>
            ) : null}
          </View>
        </View>
      </TouchableOpacity>
    </View>
  );
};

export default DeviceScreen;
