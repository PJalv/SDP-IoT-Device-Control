
async function getBrokerParams() {
    const params = await eel.getBrokerParams()();
    console.log(params);
    brokerName = document.querySelector("#brokerNameInput");
    brokerName.value = params[0];
    document.querySelector("#brokerName").innerText = params[0];
    brokerIP = document.querySelector('#brokerIP')
    brokerIP.value = params[1];
    brokerPort = document.querySelector('#brokerPort')
    brokerPort.value = params[2];
}
async function getIP() {
    const ipaddr = await eel.getIP()();
    console.log(ipaddr);
    ipAddressElement = document.querySelector("#hub-ip").innerText = ipaddr;

}
async function saveBrokerParams() {
    let newBrokerName = document.querySelector('#brokerNameInput').value;
    let newipAddress = document.querySelector("#brokerIP").value;
    let newPort = document.querySelector("#brokerPort").value;
    console.log("Saving new values");
    await eel.saveBrokerParams([newBrokerName, newipAddress, newPort])();
    document.querySelector("#brokerName").innerText = newBrokerName;
}
async function mqtt_connect() {
    await eel.mqtt_connect()();
    await getIP();
    await getBrokerStatus();
}

async function mqtt_connect() {
    await eel.mqtt_connect()();
    await getIP();
    await getBrokerStatus();
}

async function getBrokerStatus() {
    const status = await eel.getBrokerStatus()();
    console.log(status)
    if (status) {
        statusElement = document.querySelector("#hub-status").innerText = "Connected";
    } else {
        statusElement = document.querySelector("#hub-status").innerText = "Offline";

        // Append the created 'div' element to the navbar

    }
}

document.addEventListener('DOMContentLoaded', async function () {
    await getIP();
    await getBrokerParams();
    await getBrokerStatus();
});
