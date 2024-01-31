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
});
setInterval(async () => {
    // console.log("enter loop");
    const data = await updateData();
    console.log(data);
    if (data !== null) {
        // Handle the data
        // console.log(`${i}: Updated data:`, JSON.stringify(data));
        // i++;
        // console.log("End loop");
        // Example: Save the data to localStorage
        // localStorage.setItem('pageData', JSON.stringify(data));
        // document.querySelector('h1').innerText = `${data["led-device"]["color"]["red"]}, ${data["led-device"]["color"]["green"]},${data["led-device"]["color"]["blue"]}`;
        // document.querySelector('#colorPicker').value = `#${data["led-device"]["color"]["red"].toString(16).padStart(2, '0')}${data["led-device"]["color"]["green"].toString(16).padStart(2, '0')}${data["led-device"]["color"]["blue"].toString(16).padStart(2, '0')}`;

    }
}, 500);
