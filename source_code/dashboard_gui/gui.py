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
BROKER_PORT = os.getenv("BROKER_PORT")


# Initialize Eel
eel.init('web')

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


# Start MQTT client
mqtt_client.connect(BROKER_ADDRESS, BROKER_PORT)
mqtt_client.loop_start()
print("Started Loop!")



# Start Eel with the HTML file
eel.start('index.html', block=False)

while True:
    eel.sleep(1.0)

