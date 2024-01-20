import eel
import random
import json
import paho.mqtt.client as mqtt
from dotenv import load_dotenv
import os

env_path = os.path.join(os.path.dirname(__file__), '.env')

if not os.path.exists(env_path):
    with open(env_path, 'w') as env_file:
        env_file.write("BROKER_ADDRESS=your_default_broker_address\n")
        env_file.write("BROKER_PORT=your_default_broker_port\n")
        
        
load_dotenv()
BROKER_ADDRESS = os.getenv("BROKER_ADDRESS")
BROKER_PORT = int(os.getenv("BROKER_PORT"))


# Initialize Eel
eel.init(f'{os.path.dirname(__file__)}/web')

# Initialize MQTT client
mqtt_client = mqtt.Client()

@eel.expose
def get_random_data():
    return f"Random Number: {random.randint(1, 100)}"

@eel.expose
def get_external_data():
    external_data = {"value": random.randint(1, 100)}
    return json.dumps(external_data)

@eel.expose
def publish_to_mqtt(topic, message):
    mqtt_client.publish(topic, message)

@eel.expose
def getLEDPower():
    with open(f'{os.path.dirname(__file__)}/storage.json', 'r') as file:
    # Load the JSON data
        data = json.load(file)
    print(data["led-device"]["power"])
        # Retrieve the value associated with the key
    return int(data["led-device"]["power"])

# Start MQTT client
mqtt_client.connect(BROKER_ADDRESS, BROKER_PORT)
mqtt_client.loop_start()
print("Started Loop!")



# Start Eel with the HTML file
eel.start('index.html',size=(800,400),mode='edge', block=False)

while True:
    eel.sleep(1.0)

