import React, { useEffect, useState } from "react";
import {
  Button,
  Text,
  View,
  StyleSheet,
  ImageBackground,
  TouchableOpacity,
  Dimensions,
  Image,
} from "react-native";
import { NavigationContainer } from "@react-navigation/native";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import jwt from "expo-jwt";

const jwtSecret = process.env.EXPO_PUBLIC_JWT_SECRET;
let token = jwt.encode({}, jwtSecret, { algorithm: "HS256" });
console.log(token);
let ws = null;
let messageInterval = null;
let isConnected = false;

const connectWebSocket = (setFanStats, setLedStats) => {
  if (isConnected) {
    console.log("WebSocket is already connected.");
    return;
  }

  console.log("Attempting to connect to WebSocket server...");
  const serverIP = process.env.EXPO_PUBLIC_SERVER_ADDRESS;

  ws = new WebSocket(`ws://${serverIP}:8080/ws?token=${token}&agent=client`);

  ws.onopen = () => {
    ws.send("Hello, server!");
    console.log("WebSocket Connection Established...");
    isConnected = true;
  };

  ws.onmessage = (event) => {
    // console.log("Received message:", event.data);
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
    isConnected = false;
    setTimeout(connectWebSocket, 3000);
  };


};

const { width, height } = Dimensions.get("window");

const HomeScreen = ({ navigation }) => {
  return (
    <View style={styles.container}>
      <ImageBackground
        source={require("./assets/images/Welcome Background.webp")}
        style={styles.backgroundImage}
      >
        <TouchableOpacity
          style={styles.getstartedbutton}
          onPress={() => navigation.navigate("Device", { name: "Device List" })}
        >
          <Text style={styles.getstartedbuttonText}>Get Started!</Text>
        </TouchableOpacity>
      </ImageBackground>
    </View>
  );
};

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
        onPress={() => navigation.navigate("Home", { name: "Welcome" })}
      >
        <Text style={styles.DeviceListingTitle}>Fan</Text>
        <View style={styles.imageRow}>
          <View style={styles.imageView}>
            <Image
              source={require("./assets/images/FanIcon.png")}
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
        onPress={() => navigation.navigate("Home", { name: "Welcome" })}
      >
        <Text style={styles.DeviceListingTitle}>RGB Strip</Text>
        <View style={styles.imageRow}>
          <View style={styles.imageView}>
            <Image
              source={require("./assets/images/LEDIcon.png")}
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

const Stack = createNativeStackNavigator();

const MyStack = () => {
  return (
    <NavigationContainer>
      <Stack.Navigator>
        <Stack.Screen
          name="Home"
          component={HomeScreen}
          options={{
            title: "Welcome",
            headerStyle: {
              backgroundColor: "#24577a", // Example background color
            },
            headerTintColor: "#fff", // Example tint color for the title and buttons
            headerTitleStyle: {
              fontWeight: "bold", // Example font weight for the title
              fontSize: 28,
            },
          }}
        />
        <Stack.Screen
          name="Device"
          component={DeviceScreen}
          options={{
            title: "Device List",
            headerStyle: {
              backgroundColor: "#24577a", // Example background color
            },
            headerTintColor: "#fff", // Example tint color for the title and buttons
            headerTitleStyle: {
              fontWeight: "bold", // Example font weight for the title
              fontSize: 28,
            },
          }}
        />
      </Stack.Navigator>
    </NavigationContainer>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    flexDirection: "column",
  },
  cardContainer: {
    flex: 1,
    flexDirection: "column",
  },
  backgroundImage: {
    flex: 1,
    justifyContent: "space-between",
    alignItems: "center",
  },
  title: {
    fontSize: 70,
    color: "white",
    marginTop: 100,
  },
  getstartedbutton: {
    backgroundColor: "blue",
    padding: 15,
    borderRadius: 5,
    position: "absolute",
    bottom: 80,
    alignSelf: "center",
  },
  getstartedbuttonText: {
    color: "white",
    fontSize: 16,
  },
  DeviceListing: {
    width: "100%", // Take the full width
    justifyContent: "bottom",
    alignItems: "center",
    backgroundColor: "white",
    marginTop: 10,
  },
  DeviceListingTitle: {
    color: "#24577a",
    fontSize: 30,
    fontWeight: "bold",
  },
  DeviceListingText: {
    color: "#24577a",
    fontSize: 20,
    fontWeight: "normal",
  },
  icon: {
    width: 150,
    height: 150,
    marginTop: 25,
  },
  imageView: {
    alignItems: "right",
  },
  statsRow: {
    flexDirection: "row",
    justifyContent: "space-around",
    position: "absolute",
    top: 200,
    right: 10,
    width: "100%",
  },
  headerRow: {
    flexDirection: "row",
    justifyContent: "space-around",
    position: "relative",
    top: 150,
    left: -10,
    width: "100%",
  },
  statsColumn: {
    flexDirection: "column",
    justifyContent: "space-between",
    position: "relative",
    top: 50,
    left: 0,
    width: "100%",
  },
  imageRow: {
    flexDirection: "row",
    justifyContent: "space-around",
    position: "absolute",
    bottom: 150,
    width: "100%",
  },
});

export default MyStack;
