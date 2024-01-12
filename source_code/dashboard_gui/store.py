import paho.mqtt.client as mqtt
from dotenv import load_dotenv
import os
import json
env_path = os.path.join(os.path.dirname(__file__), '.env')

if not os.path.exists(env_path):
    with open(env_path, 'w') as env_file:
        env_file.write("BROKER_ADDRESS=your_default_broker_address\n")
        env_file.write("BROKER_PORT=your_default_broker_port\n")
        
        
load_dotenv()
BROKER_ADDRESS = os.getenv("BROKER_ADDRESS")
BROKER_PORT = int(os.getenv("BROKER_PORT"))
topics = ["led/status/power", "led/status/color"]

update_values = {
    "fan-device": {
        "power": 0,
        "duty_cycle": 0,
        "rpm": 0
    },
    "led-device": {
        "power": 0,
        "color": {
            "red": 0,
            "green": 0,
            "blue": 0
        }
    }   
    }
def update_json_file():

    file_path = "storage.json"
    if os.path.exists(file_path):
        # Load existing data from the file
        with open(file_path, 'r') as file:
            existing_data = json.load(file)

        # Update the existing data with the new values
        for key, value in update_values.items():
            if key in existing_data:
                existing_data[key].update(value)

        # Save the updated data back to the file
        with open(file_path, 'w') as file:
            json.dump(existing_data, file, indent=2)
            
        print(f"Updated values in {file_path}")
    else:
        print(f"File not found: {file_path}")
    

# Callback function for when a message is received
def on_message(client, userdata, message):
    topic = message.topic
    message = message.payload.decode()
    if "fan" in topic:
        print("FAN")
        if topic == "fan/status/power":
            print("POWER")
        elif topic == "fan/status/dc":
            print("DC")
    elif "led" in topic:
        print("LED")
        if topic == "led/status/power":
            update_values["led-device"]["power"] = int(message[4:5])
        elif topic == "led/status/color":
            data = json.loads(message)
            update_values["led-device"]["color"]["red"] = int(data['red'])
            update_values["led-device"]["color"]["green"] = int(data['green'])
            update_values["led-device"]["color"]["blue"] = int(data['blue'])
    elif "speaker" in topic:
        print("SPEAKER")
    else:
        print("Error parsing device topic.")
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
        input("Press Enter to exit...\n")
    except KeyboardInterrupt:
        # Press Ctrl+C to exit the loop
        break

# Disconnect from the MQTT broker
client.loop_stop()
client.disconnect()

