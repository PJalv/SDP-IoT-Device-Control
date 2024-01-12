
document.body.style.userSelect = 'none';


async function updateData() {
    try {
        const data = await eel.get_random_data()();
        localStorage.setItem('pageData', data);
        document.getElementById('message').innerText = data;
        console.log(data);
    } catch (error) {
        console.error('Error updating data:', error);
    }
}

let powValue = 0, red = 0, green = 0, blue = 0;
async function publish(topic, message) {

    try {
        switch (topic) {
            case "led/control/power":
                powValue = await eel.getLEDPower()();
                powValue = (powValue === 0) ? 1 : 0;
                console.log(`POWVALUE : ${powValue}`);
                await eel.publish_to_mqtt(topic, `INT:${powValue}`)();
                break;
            case "led/control/color":
                let colorObject = {
                    red: 0,
                    green: 0,
                    blue: 0,
                }
                colorObject.red = Math.floor(Math.random() * 256);
                colorObject.green = Math.floor(Math.random() * 256);
                colorObject.blue = Math.floor(Math.random() * 256);
                await eel.publish_to_mqtt(topic, `JSON:${JSON.stringify(colorObject)}`)();
            default:
                break;
        }



    } catch (error) {
        console.error('Error publishing data:', error);
    }
}

// Try to get the saved data from localStorage
const savedData = localStorage.getItem('pageData');

if (savedData) {
    document.getElementById('message').innerText = savedData;
}
console.log("Hello");
// Periodically update the data and save it to localStorage
setInterval(updateData, 500);
