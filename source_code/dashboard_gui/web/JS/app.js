document.body.style.userSelect = 'none';

async function mqtt_connect() {
    statusElement = document.querySelector("#hub-status").innerText = "Connecting...";
    await eel.mqtt_connect()();
    await getIP();
    await getBrokerStatus();
}

async function getIP() {
    const ipaddr = await eel.getIP()();
    console.log(ipaddr);
    ipAddressElement = document.querySelector("#hub-ip").innerText = ipaddr;

}
async function getBrokerStatus() {
    const status = await eel.getBrokerStatus()();
    if (status) {
        statusElement = document.querySelector("#hub-status").innerText = "Connected";
    } else {
        statusElement = document.querySelector("#hub-status").innerText = "Offline";

    }
}

async function updateData() {
    // console.log("Enter update");
    try {
        const data = await eel.read_storage()();
        return JSON.parse(data);
    } catch (error) {
        console.error('Error updating data:', error);
        return null;  // Return null to indicate an error
    }
}

async function publish(topic, message) {
    try {
        switch (topic) {
            case "led/control/power":
                let powValue = await eel.getLEDPower()();
                powValue = (powValue === 1) ? 0 : 1;
                console.log(powValue);
                await eel.publish_to_mqtt(topic, `INT:${powValue}`)();
                break;
            case "led/control/color":
                const colorObject = {
                    red: Math.floor(Math.random() * 256),
                    green: Math.floor(Math.random() * 256),
                    blue: Math.floor(Math.random() * 256),
                };
                await eel.publish_to_mqtt(topic, `JSON:${JSON.stringify(colorObject)}`)();
                break;
            default:
                break;
        }
    } catch (error) {
        console.error('Error publishing data:', error);
    }
}
document.addEventListener('DOMContentLoaded', async function () {
    const [brokerName] = await eel.getBrokerParams()();
    console.log(brokerName);
    const status = await eel.getBrokerStatus()();
    await getIP();
    console.log(status);
    if (!status) {
        await mqtt_connect();
        await getBrokerStatus();
    }
    else {
        await getBrokerStatus();
    }
    document.querySelector("#brokerName").innerText = brokerName;
});
setInterval(async () => {
    // console.log("enter loop");
    const data = await updateData();
    console.log(data);
    if (data !== null) {
        // console.log(`Updated data:`, JSON.stringify(data));
        // localStorage.setItem('pageData', JSON.stringify(data));
        document.querySelector("#currentColorSquare").style.backgroundColor = `rgb(${data["led-device"]["color"]["red"]}, ${data["led-device"]["color"]["green"]}, ${data["led-device"]["color"]["blue"]})`;
        document.querySelector('#fan-power').innerText = data['fan-device']['power'] == 1 ? "OFF" : "ON";
        document.querySelector('#fan-rpm').innerText = data["fan-device"]["rpm"];


    }
}, 500);
