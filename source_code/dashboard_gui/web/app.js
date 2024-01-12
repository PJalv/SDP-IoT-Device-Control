
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

let powValue = 0;

async function publish(topic, message) {
    try {
        powValue = (powValue === 0) ? 1 : 0;
        console.log(`POWVALUE : ${powValue}`);
        if (topic === "led/status/power") {
            await eel.publish_to_mqtt(topic, `INT:${powValue}`)();
        } else {
            await eel.publish_to_mqtt(topic, message)();
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
