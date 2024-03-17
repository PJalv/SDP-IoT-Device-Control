document.body.style.userSelect = 'none';
let selectedValue;

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

document.addEventListener('DOMContentLoaded', function () {

    let speedDisplay = document.querySelector('#speedDisplay');
    speedDisplay.innerText = 548;
    selectedValue = 548;
});

let slider = document.getElementById('fanSpeed');
let speedDisplay = document.querySelector('#speedDisplay');
slider.addEventListener('input', function () {
    // Get the selectedValuee from the dropdown
    selectedValue = slider.value;

    // Display or use the selectedValuee as needed
    // console.log("SelectedValuee: ", selectedValue);
    speedDisplay.innerText = selectedValue;
});

let powerButton = document.querySelector("#fanToggle");
powerButton.addEventListener('click', async function () {
    await publish('fan/control')

})

async function publish(topic, message = null) {
    try {
        const data = await updateData();
        let powValue;
        switch (topic) {
            case "fan/control":
                powValue = data['fan-device']['power'];
                powValue = (powValue === 1) ? 0 : 1;
                console.log(powValue);
                await eel.publish_to_mqtt(topic, `INT:${powValue}`)();
                break;
            case "led/control/power":
                powValue = data['led-device']['power'];
                powValue = (powValue === 1) ? 0 : 1;
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
                await eel.publish_to_mqtt(topic, `INT:${message}`)
                break;
        }
    } catch (error) {
        console.error('Error publishing data:', error);
    }
}



let breeze = document.getElementById('breezeMode');
breeze.addEventListener('change', function () {
    console.log(breeze.checked)
});


document.getElementById("updateFan").addEventListener("click", async function () {
    console.log("Enter submit");
    let topic, message, functionObject;

    topic = "fan/control";
    if (breeze.checked) {
        functionObject = {
            function: 1
        }
        message = `JSON:${JSON.stringify(functionObject)}`
    }
    else {
        message = `INT:${selectedValue}`
    }
    await eel.publish_to_mqtt(topic, message)();
})
setInterval(async () => {
    // console.log("enter loop");
    const data = await updateData();
    console.log(data);
    if (data !== null) {
        document.querySelector('#fanToggle').classList = data['fan-device']['power'] == 0 ? "btn btn-danger btn-primary" : "btn btn-success btn-primary"
        document.querySelector('#fan-power').innerText = data['fan-device']['power'] == 0 ? "OFF" : "ON";
        document.querySelector('#fan-duty-cycle').innerText = data['fan-device']['duty_cycle'];
        document.querySelector('#fan-rpm').innerText = data['fan-device']['rpm'];

    }
}, 500);
