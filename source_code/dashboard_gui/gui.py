import eel
import random
import json
import socket
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
eel.init(os.path.join(os.path.dirname(__file__), 'web'))

# Initialize MQTT client
mqtt_client = mqtt.Client()

@eel.expose
def read_storage():
    try:
        with open((os.path.join(os.path.dirname(__file__), 'storage.json')), 'r') as file:
            # Load JSON data from the file
            data = json.load(file)
            print("Sent JSON to EEL")
            return json.dumps(data)  # Return JSON as a string
    except Exception as e:
        print(f"Error: {e}")
        return None


@eel.expose
def publish_to_mqtt(topic, message):
    mqtt_client.publish(topic, message)
    print(f"SENT {message} to {topic}")


@eel.expose
def getBrokerStatus():
    return mqtt_client.is_connected()

@eel.expose
def getIP():
    try:
        # Get the local IP address
        local_ip = socket.gethostbyname(socket.gethostname())
        return local_ip
    except Exception as e:
        print(f"Error: {e}")
        return None

@eel.expose
def mqtt_connect():
    try:
        mqtt_client.connect(BROKER_ADDRESS, BROKER_PORT)
        mqtt_client.loop_start()
        print("Connected successfully")
        eel.sleep(2.0)
        return "success"
    
    except Exception as e:
        print(f"Could not connect: {e}")
        eel.sleep(2.0)
        return "error"



# Start Eel with the HTML file
eel.start('html/index.html', mode='edge', block=False)

while True:
    eel.sleep(1.0)
    

