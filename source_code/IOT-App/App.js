import React, { useEffect, useState } from "react";
import { NavigationContainer } from "@react-navigation/native";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import HomeScreen from "./components/homeScreen";
import DeviceScreen from "./components/deviceScreen";
import FanScreen from "./components/fanScreen";
import LedScreen from "./components/ledScreen";
import { Dimensions } from "react-native";
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
        <Stack.Screen
          name="Fan"
          component={FanScreen}
          options={{
            title: "Fan Control",
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
          name="LED"
          component={LedScreen}
          options={{
            title: "LED Control",
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

export default MyStack;
