
async function getBrokerParams() {
    const params = await eel.getBrokerParams()();
    console.log(params);
    brokerIP = document.querySelector('#brokerIP')
    brokerIP.value = params[0];
    brokerPort = document.querySelector('#brokerPort')
    brokerPort.value = params[1];
}
async function getIP() {
    const ipaddr = await eel.getIP()();
    console.log(ipaddr);
    ipAddressElement = document.querySelector("#hub-ip").innerText = ipaddr;

}
async function saveBrokerParams() {
    let newipAddress = document.querySelector("#brokerIP").value;
    let newPort = document.querySelector("#brokerPort").value;
    console.log("Saving new values");
    await eel.saveBrokerParams([newipAddress, newPort])();
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
    await getBrokerParams();
    await getBrokerStatus();
});
