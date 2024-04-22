import React from "react";
import {
  Text,
  View,
  StyleSheet,
  ImageBackground,
  TouchableOpacity,
  Dimensions,
} from "react-native";

const HomeScreen = ({ navigation }) => {
  return (
    <View style={styles.container}>
      <ImageBackground
        source={require("../assets/images/Welcome Background.webp")}
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

export default HomeScreen;
export { styles };
