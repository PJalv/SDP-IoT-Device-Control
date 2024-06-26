document.body.style.userSelect = "none";

async function mqtt_connect() {
  statusElement = document.querySelector("#hub-status").innerText =
    "Connecting...";
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
    statusElement = document.querySelector("#hub-status").innerText =
      "Connected";
    statusElement = document.querySelector("#hub-status").style.color = "green";
  } else {
    statusElement = document.querySelector("#hub-status").innerText = "Offline";
    statusElement = document.querySelector("#hub-status").style.color = "red";
  }
}

async function updateData() {
  // console.log("Enter update");
  try {
    const data = await eel.read_storage()();
    return JSON.parse(data);
  } catch (error) {
    console.error("Error updating data:", error);
    return null; // Return null to indicate an error
  }
}

async function publish(topic, message = null) {
  try {
    const data = await updateData();
    let powValue;
    switch (topic) {
      case "fan/control":
        powValue = data["fan-device"]["power"];
        powValue = powValue === 1 ? 0 : 1;
        console.log(powValue);
        await eel.publish_to_mqtt(topic, `INT:${powValue}`)();
        break;
      case "led/control/power":
        powValue = data["led-device"]["power"];
        powValue = powValue === 1 ? 0 : 1;
        console.log(powValue);
        await eel.publish_to_mqtt(topic, `INT:${powValue}`)();
        break;
      // case "led/control/color":
      //     const colorObject = {
      //         red: Math.floor(Math.random() * 256),
      //         green: Math.floor(Math.random() * 256),
      //         blue: Math.floor(Math.random() * 256),
      //     };
      //     await eel.publish_to_mqtt(topic, `JSON:${JSON.stringify(colorObject)}`)();
      //     break;
      default:
        await eel.publish_to_mqtt(topic, `INT:${message}`);
        break;
    }
  } catch (error) {
    console.error("Error publishing data:", error);
  }
}
document.addEventListener("DOMContentLoaded", async function () {
  const data = await updateData();
  document.querySelector("#fan-device").classList =
    data["fan-device"]["status"]["isOnline"] == 0
      ? "card device-card shadow gray-out"
      : "card device-card shadow";
  const [brokerName] = await eel.getBrokerParams()();
  console.log(brokerName);
  const status = await eel.getBrokerStatus()();
  await getIP();
  console.log(status);
  if (!status) {
    await mqtt_connect();
    await getBrokerStatus();
  } else {
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
    document.querySelector("#led-power").innerText =
      data["led-device"]["power"] == 0 ? "OFF" : "ON";

    document.querySelector("#fan-power").innerText =
      data["fan-device"]["power"] == 0 ? "OFF" : "ON";
    document.querySelector("#fan-device").classList =
      data["fan-device"]["status"]["isOnline"] == 0
        ? "card device-card shadow gray-out"
        : "card device-card shadow";
    document.querySelector("#led-device").classList =
      data["led-device"]["status"]["isOnline"] == 0
        ? "card device-card shadow gray-out"
        : "card device-card shadow";

    document.querySelector("#fan-desc").innerHTML =
      data["fan-device"]["function"] === 1
        ? "Mode\n<span>Breeze</span>"
        : `RPM\n<span>${data["fan-device"]["rpm"]}</span>`;
    document.querySelector("#led-desc").innerHTML =
      data["led-device"]["function"] === 0
        ? "Color\n<div id='currentColorSquare' class='color-square mx-auto'> </div>"
        : data["led-device"]["function"] === 1
        ? "Mode\n<span>SR</span>"
        : data["led-device"]["function"] === 2
        ? "Mode\n<span>TR</span>"
        : null;
    document.querySelector(
      "#currentColorSquare"
    ).style.backgroundColor = `rgb(${data["led-device"]["color"]["red"]}, ${data["led-device"]["color"]["green"]}, ${data["led-device"]["color"]["blue"]})`;
  }
}, 500);
