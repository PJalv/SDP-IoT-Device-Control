import paho.mqtt.client as mqtt
from dotenv import load_dotenv
import os
import json
import time
# Get the directory path of the current Python file
current_dir = os.path.dirname(os.path.abspath(__file__))

# Construct the path to the .env file (one directory above the current directory)
env_path = os.path.normpath(os.path.join(current_dir, "../.env"))

print(env_path)

if not os.path.exists(env_path):
    with open(env_path, 'w') as env_file:
        env_file.write("BROKER_ADDRESS=your_default_broker_address\n")
        env_file.write("BROKER_PORT=your_default_broker_port\n")
        
        
load_dotenv()
BROKER_ADDRESS = os.getenv("BROKER_ADDRESS")
BROKER_PORT = int(os.getenv("BROKER_PORT"))
topics = ["led/status", "led/status/color", "device_heartbeat", "fan/status", "fan/status/rpm"]

update_values = {
  "fan-device": {
    "status": {
      "isOnline": 0,
      "lastHeartbeat": 394820974
    },
    "function": 0,
    "power": 0,
    "duty_cycle": 0,
    "rpm": 0
  },
  "led-device": {
    "status": {
      "isOnline": 1,
      "lastHeartbeat": 1706670211
    },
    "power": 1,
    "function": 0,
    "color": {
      "red": 15,
      "green": 161,
      "blue": 17
    }
  }
}

def update_json_file():
    file_path = (os.path.join(os.path.dirname(__file__), 'storage.json'))
    if os.path.exists(file_path):
        try:
            with open(file_path, 'r+') as file:
                existing_data = json.load(file)
                existing_data.update(update_values)
                file.seek(0)  # Move to the beginning of the file
                json.dump(existing_data, file, indent=2)
                file.truncate()  # Truncate the file to remove any remaining content
                # print(f"Updated values in {file_path}")
        except json.JSONDecodeError as e:
            print(f"Error decoding JSON: {e}")
    else:
        print(f"File not found: {file_path}")
    

# Callback function for when a message is received
def on_message(client, userdata, message):
    try:
        topic = message.topic
        message = message.payload.decode()
        print(topic)
        print(message)
        if topic == "device_heartbeat":
            timestamp = int(time.time())
            if message == "fan":
                update_values["fan-device"]["status"]["isOnline"] = 1
                update_values["fan-device"]["status"]["lastHeartbeat"] = timestamp
                
            elif message == "led":
                update_values["led-device"]["status"]["isOnline"] = 1
                update_values["led-device"]["status"]["lastHeartbeat"] = timestamp
        elif "fan" in topic:
            data = json.loads(message)
            print(data)
            if topic == "fan/status":
                update_values["fan-device"]["function"] = int(data['function'])
                update_values["fan-device"]["power"] = int(data['power'])
                update_values["fan-device"]["duty_cycle"] = int(data['dutyCycle'])
                update_values["fan-device"]["rpm"] = int(data['rpm'])
        elif "led" in topic:
            data = json.loads(message)
            if topic == "led/status":
                update_values["led-device"]["power"] = int(data['power'])
                update_values["led-device"]["function"] = int(data['function'])
                update_values["led-device"]["color"]["red"] = int(data['color']['red'])
                update_values["led-device"]["color"]["green"] = int(data['color']['green'])
                update_values["led-device"]["color"]["blue"] = int(data['color']['blue']) 
        else:
            print("Error parsing device topic.")
        # global update_values
        update_json_file()
    except:
        print("Error")

def check_disconnected_devices():
    current_time = int(time.time())

    disconnect_threshold = 3  # Adjust as needed

    for device_id, device_info in update_values.items():
        last_heartbeat_time = device_info["status"]["lastHeartbeat"]

        elapsed_time = current_time - last_heartbeat_time

        if elapsed_time > disconnect_threshold:
            print(f"Device {device_id} disconnected")
            device_info["status"]["isOnline"] = 0
    update_json_file()

client = mqtt.Client()


client.on_message = on_message

client.connect(BROKER_ADDRESS, BROKER_PORT)



for topic in topics:
    try:
        client.subscribe(topic)
        print(f"Subscribed to topic {topic}") 
    except:
        print("Error subscribing")

# Loop to handle incoming messages
client.loop_start()


while True:
    try:
        # input()
        time.sleep(1.0)
        check_disconnected_devices()
    except KeyboardInterrupt:
        # Press Ctrl+C to exit the loop
        break

# Disconnect from the MQTT broker
client.loop_stop()
client.disconnect()

