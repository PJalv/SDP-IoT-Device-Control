import * as React from 'react';
import { Button, Text, View, StyleSheet, ImageBackground, TouchableOpacity, Dimensions, Image } from 'react-native';
import { NavigationContainer } from '@react-navigation/native';
import { createNativeStackNavigator } from '@react-navigation/native-stack';

const Stack = createNativeStackNavigator();
const { width, height } = Dimensions.get('window');

const HomeScreen = ({ navigation }) => {
  return (
    <View style={styles.container}>
      <ImageBackground 
      source={require('./assets/images/Welcome Background.webp')} 
      style={styles.backgroundImage}
      >
        <TouchableOpacity
          style={styles.getstartedbutton}
          onPress={() => navigation.navigate('Device', { name: 'Device List' })}
        >
          <Text style={styles.getstartedbuttonText}>Get Started!</Text>
        </TouchableOpacity>
      </ImageBackground>
    </View>
  );
};

const DeviceScreen = ({ navigation, route }) => {
  return (
    <View style={styles.cardContainer}>
      <TouchableOpacity style={[styles.DeviceListing, { height: height / 2.5 }]} onPress={() => navigation.navigate('Home', { name: 'Welcome' })}>
        <Text style={styles.DeviceListingTitle}>Fan</Text>
        <View style={styles.imageRow}>
          <View style={styles.imageView}>
            <Image
              source={require('./assets/images/FanIcon.png')}
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
            <Text style={styles.DeviceListingText}>ON/OFF</Text>
            <Text style={styles.DeviceListingText}>....</Text>
          </View>
        </View>
      </TouchableOpacity>
      <TouchableOpacity style={[styles.DeviceListing, { height: height / 2.5 }]} onPress={() => navigation.navigate('Home', { name: 'Welcome' })}>
        <Text style={styles.DeviceListingTitle}>RGB Strip</Text>
        <View style={styles.imageRow}>
          <View style={styles.imageView}>
            <Image
              source={require('./assets/images/LEDIcon.png')}
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
            <Text style={styles.DeviceListingText}>ON/OFF</Text>
            <Text style={styles.DeviceListingText}>....</Text>
          </View>
        </View>
      </TouchableOpacity>
    </View>
  );
};

const MyStack = () => {
  return (
    <NavigationContainer>
      <Stack.Navigator>
        <Stack.Screen
          name="Home"
          component={HomeScreen}
          options={{ title: 'Welcome',
          headerStyle: {
            backgroundColor: '#24577a', // Example background color
          },
          headerTintColor: '#fff', // Example tint color for the title and buttons
          headerTitleStyle: {
            fontWeight: 'bold', // Example font weight for the title
            fontSize: 28,
          }, }}
        />
        <Stack.Screen
          name="Device"
          component={DeviceScreen}
          options={{ title: 'Device List',
          headerStyle: {
            backgroundColor: '#24577a', // Example background color
          },
          headerTintColor: '#fff', // Example tint color for the title and buttons
          headerTitleStyle: {
            fontWeight: 'bold', // Example font weight for the title
            fontSize: 28,
          }, }}
        />
      </Stack.Navigator>
    </NavigationContainer>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    flexDirection: 'column',
  },
  cardContainer: {
    flex: 1,
    flexDirection: 'column',
  },
  backgroundImage: {
    flex: 1,
    justifyContent: 'space-between', 
    alignItems: 'center', 
  },
  title: {
    fontSize: 70,
    color: 'white',
    marginTop: 100, 
  },
  getstartedbutton: {
    backgroundColor: 'blue',
    padding: 15,
    borderRadius: 5,
    position: 'absolute',
    bottom: 80,
    alignSelf: 'center',
  },
  getstartedbuttonText: {
    color: 'white',
    fontSize: 16,
  },
  DeviceListing: {
    width: '100%', // Take the full width
    justifyContent:'bottom',
    alignItems: 'center',
    backgroundColor: 'white',
    marginTop: 10,
  },
  DeviceListingTitle: {
    color: '#24577a',
    fontSize: 30,
    fontWeight: 'bold',
  },
  DeviceListingText: {
    color: '#24577a',
    fontSize: 20,
    fontWeight: 'normal',
  },
  icon: {
    width: 150,
    height: 150,
    marginTop: 25,
  },
  imageView: {
    alignItems: 'right',
  },
  statsRow: {
    flexDirection: 'row', 
    justifyContent: 'space-around', 
    position: 'absolute', 
    top:200 ,
    right: 10, 
    width: '100%', 
  },
  headerRow: {
    flexDirection: 'row', 
    justifyContent: 'space-around', 
    position: 'relative', 
    top: 150,
    left: -10, 
    width: '100%', 
  },
  statsColumn: {
    flexDirection: 'column', 
    justifyContent: 'space-between', 
    position: 'relative', 
    top: 50, 
    left: 0,
    width: '100%', 
  },
  imageRow: {
  flexDirection: 'row', 
  justifyContent: 'space-around', 
  position: 'absolute', 
  bottom: 150, 
  width: '100%', 
},
 
});

export default MyStack;


