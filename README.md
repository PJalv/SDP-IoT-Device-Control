<p align="center">
</p>
<p align="center">
    <h1 align="center">IoT Device Control System</h1>
</p>
<div align="center">
    <em>Spring 2024 Senior Design Project </em>
</div>
<p align="center">
    <em> Dylan St Laurent & Jorge Suarez </em>
</p>

<p align="center">
  <img src="https://www.cpp.edu/engineering/img/branding/ece-two-row.jpg" width="300" alt="project-logo"ngs`uuuu`u>
</p>
<p align="center">
	<img src="https://img.shields.io/github/license/PJalv/SDP-IoT-Device-Control?style=default&logo=opensourceinitiative&logoColor=white&color=0080ff" alt="license">
	<img src="https://img.shields.io/github/last-commit/PJalv/SDP-IoT-Device-Control?style=default&logo=git&logoColor=white&color=0080ff" alt="last-commit">
	<img src="https://img.shields.io/github/languages/top/PJalv/SDP-IoT-Device-Control?style=default&color=0080ff" alt="repo-top-language">
	<img src="https://img.shields.io/github/languages/count/PJalv/SDP-IoT-Device-Control?style=default&color=0080ff" alt="repo-language-count">
<p>
<p align="center">
	<!-- default option, no dependency badges. -->
</p>

<br><!-- TABLE OF CONTENTS -->

<details>
  <summary>Table of Contents</summary><br>

- [Overview](#overview)
- [Features](#features)
- [Repository Structure](#repository-structure)
- [Modules](#modules)
- [Getting Started](#getting-started)
  - [Installation](#installation)
  - [Usage](#usage)
- [Project Roadmap](#project-roadmap)
- [License](#license)
- [Acknowledgments](#acknowledgments)
</details>
<hr>

## Overview
![Big Picture](https://i.imgur.com/pV2HRJF.png)
The SDP-IoT-Device-Control project is structured to leverage a combination of hardware components and software technologies to provide a comprehensive IoT device control system. Here's an overview of the project's structure and the technologies used:

### Components:
1. **ESP32 Microcontrollers**: These are responsible for device control, data collection, and communication.
2. **Raspberry Pi**: Serves as a local server or gateway, facilitating the interface between ESP32 devices and the cloud server.
3. **Cloud Server**: Manages backend services, databases, and ensures data synchronization and remote access.
4. **Mobile App**: Enables users to interact with the IoT system, control devices, and view data.

### Software Technologies:
- **ESP-IDF**: This is used for programming ESP32 devices, providing essential IoT tools and libraries.
- **FreeRTOS**: Provides a real-time operating system for ESP32 devices, ensuring efficient resource management and task scheduling. We leverage FreeRTOS to separate vital ESP functions like WiFi and MQTT, allowing each to operate independently and efficiently.
- **MQTT Protocol**: Provides efficient real-time messaging across devices and servers.
- **Bun.js**: Implemented on the Raspberry Pi for fast package management and development.
- **React Native**: Utilized for developing the cross-platform mobile application.
- **WebSocket**: Supports real-time data transmission to the mobile app.
- **Go (Golang)**: Specifically used on the cloud server for its robust support for concurrency. Go routines and channels are employed to manage multiple device data streams simultaneously, significantly enhancing the system's scalability and its ability to handle large volumes of IoT device communications efficiently.

---

## Features

|     | Feature           | Description                                                                                                                                                                                         |
| --- | ----------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ⚙️  | **Architecture**  | _Utilizes a modular architecture with technologies such as Go, React Native, and Expo, orchestrated by CMake for IoT device control._ |
| 🔌  | **Integrations**  | _Incorporates JWT for security, MQTT for device messaging, and WebSocket for real-time communication, supporting web and mobile functionalities._ |
| 🧩  | **Modularity**    | _Features distinct components like fan_device and main, which enhance reusability and maintainability._ |
| 🛡️  | **Security**      | _Employs JWT and secure communication protocols such as MQTT and WebSocket to ensure data protection and secure access._ |
| 🚀  | **Scalability**   | _Supports increased traffic and load efficiently through its modular architecture and scalable technologies._ |


---

## Repository Structure

```sh
└── SDP-IoT-Device-Control/
    ├── README.md
    ├── schematics
    │   ├── AudioDriver
    │   ├── Espressif
    │   ├── FanSystem
    │   ├── LED_Device
    │   └── Power
    └── source_code
        ├── IOT-App
        ├── cloud_websocket
        ├── dashboard_gui
        ├── fan_device
        ├── led_device
        └── utils
```

---

## Modules

<details closed><summary>Fan Device Main Module</summary>

| File                                                                                                                           | Summary                                                                                                                                                                                                                                                                        |
| ------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [audio.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/fan_device/main/audio.h)                     | Enables setting up I2S audio with a defined buffer size for the fan device in the SDP-IoT-Device-Control repository.                                                                                                                                                           |
| [init.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/fan_device/main/init.h)                       | Initializes and finalizes the setup for the IoT fan device. Manages the initialization of FreeRTOS and GPIO, essential for device control. Part of the IoT Device Control repository structure under fan_device.                                                               |
| [main.c](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/fan_device/main/main.c)                       | Initializes system, sets up WiFi and MQTT tasks, processes events, maintains heartbeat, and handles counts. Coordinates device functionality within the SDP-IoT-Device-Control repositorys architecture.                                                                       |
| [tasks.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/fan_device/main/tasks.h)                     | Publishing status, playing audio, controlling breeze mode, managing tasks, establishing Wi-Fi connection, handling MQTT communication, managing heartbeats, and processing events. Crucial for fan system operation within the IoT device control architecture.                |
| [storage_nvs.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/fan_device/main/storage_nvs.h)         | Manages fan configuration data using Non-Volatile Storage (NVS) for the fan device within the SDP-IoT-Device-Control repositorys architecture. Key functionalities include setting and retrieving fan power, function, and duty cycle information.                             |
| [fan.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/fan_device/main/fan.h)                         | Implements fan control functionalities and interfaces with LED driver for RPM and power button handling. Manages duty cycle adjustments and interrupts, integrating with MQTT data for IoT device communication within the parent repositorys IoT device control architecture. |

</details>

<details closed><summary>Cloud WebSocket Broker (JavaScript)</summary>

| File                                                                                                                                     | Summary                                                                                                                                                                                                                                                                                                                                                                     |
| ---------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [broker.js](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/cloud_websocket/broker-js/broker.js)                 | Establishes connectivity between MQTT broker and WebSocket server, facilitating real-time data exchange. Handles JWT token generation and verification, synchronizing data with the broker. Manages reconnections and error handling, enhancing reliability for IoT device control system.                                                                                  |

</details>

<details closed><summary>Cloud WebSocket Server (Go)</summary>

| File                                                                                                                     | Summary                                                                                                                                                                                                                                                                                               |
| ------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [server.go](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/cloud_websocket/server-go/server.go) | Orchestrates WebSocket communication for IoT devices. Manages client connections, token verification, and message distribution in a secure manner. Handles commands and responses, gracefully falling back on failure. Monitors domain status and switches services dynamically for robust operation. |

</details>

<details closed><summary>LED Device Managed Components (Espressif LED Strip)</summary>

| File                                                                                                                                                                      | Summary                                                                                                                                                                                                                                                    |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [led_strip_spi.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/led_device/managed_components/espressif__led_strip/include/led_strip_spi.h)     | Implements creation of LED strip on SPI MOSI channel. Manages LED strip configuration and SPI specific settings to create LED strip handle successfully, handling errors like invalid arguments or unsupported configurations.                             |
| [led_strip.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/led_device/managed_components/espressif__led_strip/include/led_strip.h)             | Defines functions to manage RGB, RGBW, and HSV colors for LED strips. Contains methods to set colors for individual pixels, refresh LED memory, clear LED strip, and free resources. Enables control and customization of LED strip colors in IoT devices. |

</details>

<details closed><summary>LED Device Main Module</summary>

| File                                                                                                                           | Summary                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| ------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [led_config.c](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/led_device/main/led_config.c)           | Creates LED configuration for LED device with specified GPIO, LED count, and pixel format. Initializes LED strip object using SPI backend for improved performance, leveraging DMA support.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| [main.c](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/led_device/main/main.c)                       | The `main.c` file in the `led_device` module of the `SDP-IoT-Device-Control` repository serves as the main entry point for controlling LED devices. It integrates with various system components like I2S drivers, FreeRTOS tasks, LED strip libraries, MQTT protocols for communication, and Wi-Fi configurations. The code handles LED configurations, GPIO interactions, and JSON parsing to facilitate LED device functionality. Through this file, the LED device module manages LED operations, synchronization, and communication with the broader IoT infrastructure.This code supports the overarching architecture of the repository by providing a crucial component for controlling LED devices within the IoT ecosystem, ensuring seamless integration and communication with other system modules and functionalities. |
| [storage_nvs.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/led_device/main/storage_nvs.h)         | Manages LED device configuration data using Non-Volatile Storage in the repositorys LED device module, enhancing persistence and reliability.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |

</details>

<details closed><summary>Dashboard GUI</summary>

| File                                                                                                                       | Summary                                                                                                                                                                                                                                                              |
| -------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [gui.py](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/gui.py)                     | Facilitates interaction with MQTT broker through Eel GUI for IoT device control. Dynamically updates broker settings, reads JSON storage, and publishes messages. Handles MQTT connection and displays connection status in the GUI, enhancing user experience.      |
| [store.py](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/store.py)                 | Manages MQTT connections, updates device statuses, and detects disconnected devices. Subscribes to specific topics, parses messages, and updates JSON storage. Handles message receiving and device disconnection checks for LED and fan devices in the IoT system.  |
| [storage.json](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/storage.json)         | Stores device status, power, and configurations for fan and LED devices-Facilitates real-time monitoring and control of devices via the dashboard GUI within the IoT system                                                                                          |

</details>

<details closed><summary>Dashboard GUI Web (JavaScript)</summary>

| File                                                                                                                    | Summary                                                                                                                                                                                                                                                                |
| ----------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [settings.js](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/JS/settings.js) | Retrieves, displays, and updates broker settings and status for IoT device control. Integrates with backend APIs and UI elements. Facilitates MQTT connection handling and status monitoring, ensuring seamless communication with devices.                            |
| [led.js](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/JS/led.js)           | Improves LED control interface. Dynamically updates LED settings and colors based on user inputs. Facilitates toggling power and selecting operation modes using MQTT messaging. Enhances user experience by enabling real-time interaction with LED device.           |
| [app.js](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/JS/app.js)           | Implements MQTT connection and data update logic for IoT devices integration. Manages device statuses and controls via a dashboard interface. Visualizes device status and allows dynamic device control. Dynamically updates interface elements based on device data. |
| [fan.js](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/JS/fan.js)           | Manages fan speed control, power toggling, and data updates using MQTT messages. Dynamically displays fan status, RPM, and mode in a responsive GUI. Enables fan mode selection and updates fan parameters through MQTT communication.                                 |

</details>

<details closed><summary>Dashboard GUI Web (HTML)</summary>

| File                                                                                                                                | Summary                                                                                                                                                                                                                                                                                                                |
| ----------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [index.html](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/html/index.html)             | Generates a responsive dashboard GUI with cards for fan and LED devices. Allows toggling power and settings, displaying device status dynamically. Incorporates Bootstrap CSS and icons for a user-friendly interface.                                                                                                 |
| [Settings.html](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/html/Settings.html)       | Creates a responsive settings interface for configuring the IoT devices broker connection. Features include input fields for broker name, IP address, and port number, with a Connect' button. Leverages Bootstrap for styling and jQuery for functionality.                                                           |
| [eel.js](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/html/eel.js)                     | Enables dynamic communication between Python and JavaScript, facilitating function exposure and asynchronous calls. Handles WebSocket interactions with an embedded Python app, supporting customization of window size and position for a web-based dashboard GUI.                                                    |
| [FanSettings.html](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/html/FanSettings.html) | Showcases an interactive web interface for managing fan settings. Offers power toggling, speed adjustment, breeze mode selection, and real-time statistical data display. Enhances user experience with Bootstrap styling and jQuery functionalities. Integrated for seamless device control in the SDP-IoT ecosystem. |
| [LEDSettings.html](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/dashboard_gui/web/html/LEDSettings.html) | Defines the interactive LED device settings interface in the dashboard. Displays color and function options, power toggle, and current stats visualization. Utilizes Bootstrap templates and jQuery for dynamic behavior, enhancing user control over the LED device.                                                  |

</details>

<details closed><summary>Utility Modules</summary>

| File                                                                                           | Summary                                                                                                                                                                                                                                                                       |
| ---------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [wifi.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/utils/wifi.h) | Enables WiFi connectivity for IoT devices. Declares WiFi connection variables and functions. Facilitates seamless integration with parent repositorys cloud and device modules.                                                                                               |
| [mqtt.h](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/utils/mqtt.h) | Defines MQTT functionality for IoT devices, supporting integer and JSON payloads. Manages data with semaphores and tasks, handling message events through subscriptions and publications. Facilitates efficient communication within the SDP-IoT-Device-Control architecture. |

</details>

<details closed><summary>IoT App</summary>

| File                                                                                                                   | Summary                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| ---------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [App.js](https://github.com/PJalv/SDP-IoT-Device-Control/blob/master/source_code/IOT-App/App.js)                       | Defines navigation structure for IoT device control app using React Navigation. Screens for Home, Device List, Fan Control, and LED Control are specified with custom styling. Context from DeviceProvider wraps the app for device management.                                                                                                                                                                                                                                                                                                                                                          |

</details>
---

## Getting Started
**System Requirements:**

- **ESP32 Microcontroller**: Required for device control and data collection.
- **Linux-based Machine for Broker**: We used a Raspberry Pi with a touchscreen for this purpose.
- **Linux Machine for Cloud Server**: This could be the same machine as the broker or a separate one.
## Cloning the Repository

To get started with the SDP-IoT-Device-Control project, you need to clone the repository to your local machine. Follow the steps below:

1. **Open your terminal**: Ensure you have Git installed on your system. If not, you can download and install it from [here](https://git-scm.com/downloads).

2. **Clone the repository**: Use the following command to clone the repository to your local machine:
   ```sh
   git clone https://github.com/PJalv/SDP-IoT-Device-Control.git
   ```

3. **Navigate to the project directory**: Change your current directory to the project directory.
   ```sh
   cd SDP-IoT-Device-Control
   ```

5. **Set up environment variables**: Create a `.env` file in the root of your project and add the necessary environment variables as specified in the documentation.



### Installation

### Environment Variables

Create a `.env` file in the root of your project with the following content:
>```bash
>  BROKER_NAME='<your_broker_name>'
>  BROKER_ADDRESS='<your_broker_address>'
>  BROKER_PORT='<your_broker_port>'
>  SERVER_ADDRESS='<your_server_address>'
>  JWT_SECRET='<your_jwt_secret>'
>  OPENAI_APIKEY='<your_openai_api_key>'

### ESP32 Credential Setup
Edit the `creds-template.h` file with your network and broker credentials and save it as `credentials.h`
### Usage


### Microcontroller Setup

> Ensure you have ESP-IDF installed and configured. Use the `menuconfig` terminal of ESP-IDF to configure the `partition.csv` file.
>
> Set the COM port for your ESP32 device and then run the following commands to build, flash, and monitor the application:
>
> ```console
> $ idf.py set-target esp32
> $ idf.py menuconfig
> $ idf.py build
> $ idf.py -p [PORT] flash monitor
> ```

### Raspberry Pi Setup

> The Raspberry Pi hosts the broker, the dashboard GUI, and connects to the WebSocket server. Follow these steps to set it up:
>
> 1. Install the necessary dependencies:
>    ```console
>    $ sudo apt-get update
>    $ sudo apt-get install mosquitto mosquitto-clients
>    $ sudo apt-get install npm
>    ```
> 2. Install Bun, a fast JavaScript runtime:
>    ```console
>    $ curl -fsSL https://bun.sh/install | bash
>    ```
> 3. Clone the repository and navigate to the dashboard GUI directory:
>    ```console
>    $ git clone https://github.com/PJalv/SDP-IoT-Device-Control.git
>    $ cd SDP-IoT-Device-Control/source_code/dashboard_gui
>    ```
> 4. Install the required packages using Bun:
>    ```console
>    $ bun install
>    ```
> 5. Start the dashboard GUI:
>    ```console
>    $ bun run start
>    ```

### Server Setup

> The server is a Go application. Follow these steps to set it up:
>
> 1. Install Go from the official website: https://golang.org/dl/
> 2. Clone the repository and navigate to the server directory:
>    ```console
>    $ git clone https://github.com/PJalv/SDP-IoT-Device-Control.git
>    $ cd SDP-IoT-Device-Control/source_code/server
>    ```
> 3. Build and run the server:
>    ```console
>    $ go build server.go
>    $ ./server
>    ```

### App Setup

> The app is built using Expo. Follow these steps to set it up:
>
> 1. Install Expo CLI:
>    ```console
>    $ npm install -g expo-cli
>    ```
> 2. Clone the repository and navigate to the app directory:
>    ```console
>    $ git clone https://github.com/PJalv/SDP-IoT-Device-Control.git
>    $ cd SDP-IoT-Device-Control/source_code/IOT-App
>    ```
> 3. Install the required packages:
>    ```console
>    $ npm install
>    ```
> 4. Start the Expo development server:
>    ```console
>    $ expo start
>    ```


---

## Project Roadmap

- [x] `Refactor Fan Device`
- [ ] `Refactor LED Device`
<!-- - [ ] `` -->

---

<!-- ## Contributing

Contributions are welcome! Here are several ways you can contribute:

- **[Report Issues](https://github.com/PJalv/SDP-IoT-Device-Control/issues)**: Submit bugs found or log feature requests for the `SDP-IoT-Device-Control` project.
- **[Submit Pull Requests](https://github.com/PJalv/SDP-IoT-Device-Control/blob/main/CONTRIBUTING.md)**: Review open PRs, and submit your own PRs.
- **[Join the Discussions](https://github.com/PJalv/SDP-IoT-Device-Control/discussions)**: Share your insights, provide feedback, or ask questions.

<details closed>
<summary>Contributing Guidelines</summary>

1. **Fork the Repository**: Start by forking the project repository to your github account.
2. **Clone Locally**: Clone the forked repository to your local machine using a git client.
   ```sh
   git clone https://github.com/PJalv/SDP-IoT-Device-Control
   ```
3. **Create a New Branch**: Always work on a new branch, giving it a descriptive name.
   ```sh
   git checkout -b new-feature-x
   ```
4. **Make Your Changes**: Develop and test your changes locally.
5. **Commit Your Changes**: Commit with a clear message describing your updates.
   ```sh
   git commit -m 'Implemented new feature x.'
   ```
6. **Push to github**: Push the changes to your forked repository.
   ```sh
   git push origin new-feature-x
   ```
7. **Submit a Pull Request**: Create a PR against the original project repository. Clearly describe the changes and their motivations.
8. **Review**: Once your PR is reviewed and approved, it will be merged into the main branch. Congratulations on your contribution!
</details>

<details closed> -->
<summary>Contributor Graph</summary>
<br>
<p align="center">
   <a href="https://github.com{/PJalv/SDP-IoT-Device-Control/}graphs/contributors">
      <img src="https://contrib.rocks/image?repo=PJalv/SDP-IoT-Device-Control">
   </a>
</p>
</details>

---

## License

This project is protected under the [MIT](https://choosealicense.com/licenses/mit/) License.

---

## Acknowledgments

- **Advisor: Professor Tim Lin**
  - Electrical and Computer Engineering Department, College of Engineering
  - hlin@cpp.edu

[**Return**](#-overview)

---

