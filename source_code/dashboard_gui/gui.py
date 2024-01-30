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
        env_file.write("BROKER_ADDRESS=\n")
        env_file.write("BROKER_PORT=\n")
        
        
load_dotenv()
try:    
    BROKER_ADDRESS = os.getenv("BROKER_ADDRESS")
    BROKER_PORT = int(os.getenv("BROKER_PORT"))
except:
    print("Error")


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
def getBrokerParams():
    params = [BROKER_ADDRESS, BROKER_PORT]
    return params

@eel.expose
def getBrokerStatus():
    global status
    status = mqtt_client.is_connected()
    eel.sleep(0.1)
    return status

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
def saveBrokerParams(params):
    global BROKER_ADDRESS, BROKER_PORT  # Declare them as global

    with open(env_path, 'w') as env_file:
        env_file.write(f"BROKER_ADDRESS={params[0]}\n")
        env_file.write(f"BROKER_PORT={params[1]}\n")
        
    # Update the global variables
    BROKER_ADDRESS = params[0]
    BROKER_PORT = int(params[1])

    
@eel.expose
def mqtt_connect():
    try:
        print(BROKER_ADDRESS)
        mqtt_client.connect(BROKER_ADDRESS, BROKER_PORT)
        mqtt_client.loop_start()
        eel.sleep(2.0)
        print("Connected successfully")
        return "success"
    
    except Exception as e:
        print(f"Could not connect: {e}")
        # eel.sleep(2.0)
        return "error"



# Start Eel with the HTML file
eel.start('html/index.html', mode='edge', block=False)

while True:
    eel.sleep(1.0)
    

