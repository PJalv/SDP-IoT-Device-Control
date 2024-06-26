import eel
import random
import json
import socket
import paho.mqtt.client as mqtt
from dotenv import load_dotenv, dotenv_values, set_key
import os
import platform




# Get the directory path of the current Python file
current_dir = os.path.dirname(os.path.abspath(__file__))

# Construct the path to the .env file (one directory above the current directory)
env_path = os.path.normpath(os.path.join(current_dir, "../.env"))
if not os.path.exists(env_path):
    with open(env_path, 'w') as env_file:
        env_file.write("BROKER_NAME=\n")
        env_file.write("BROKER_ADDRESS=\n")
        env_file.write("BROKER_PORT=\n")
        
        
load_dotenv()
try:    
    BROKER_ADDRESS = os.getenv("BROKER_ADDRESS")
    BROKER_PORT = int(os.getenv("BROKER_PORT"))
    BROKER_NAME = os.getenv("BROKER_NAME")
except:
    print("Error")


# Initialize Eel
eel.init(os.path.join(os.path.dirname(__file__), 'web'))

# Initialize MQTT client
mqtt_client = mqtt.Client(client_id=BROKER_NAME)

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
    params = [BROKER_NAME, BROKER_ADDRESS, BROKER_PORT]
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
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))  # Connect to Google's public DNS server
        local_ip = s.getsockname()[0]
        s.close()
        return local_ip
    except Exception as e:
        print(f"Error: {e}")
        return None

@eel.expose
def saveBrokerParams(params):
    global BROKER_NAME, BROKER_ADDRESS, BROKER_PORT  # Declare them as global

    env_dict = dotenv_values(env_path)
    set_key(env_path, 'BROKER_NAME', params[0])
    set_key(env_path, 'BROKER_ADDRESS', params[1])
    set_key(env_path, 'BROKER_PORT', str(params[2]))
    
    BROKER_NAME = params[0]
    BROKER_ADDRESS = params[1]
    BROKER_PORT = int(params[2])

    
@eel.expose
def mqtt_connect():
    try:
        print(BROKER_ADDRESS)
        mqtt_client.connect(BROKER_ADDRESS, BROKER_PORT)
        mqtt_client.loop_start()
        # eel.sleep(2.0)
        print("Connected successfully")
        return "success"
    
    except Exception as e:
        print(f"Could not connect: {e}")
        # eel.sleep(2.0)
        return "error"



# Start Eel with the HTML file
if platform.system() == 'Windows':
    eel.start('html/index.html', mode='edge', block=False)
else:
    eel.start('html/index.html', host='0.0.0.0', mode='None', block=False)

while True:
    eel.sleep(1.0)
    

