document.body.style.userSelect = 'none';


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
let rgbArray;
document.addEventListener('DOMContentLoaded', function () {
    let colorPicker = document.getElementById('ledColorPicker');
    let rgbValuesElement = document.getElementById('rgbValues');

    colorPicker.addEventListener('input', function (event) {
        // Get the RGB values from the color picker
        rgbArray = hexToRgb(event.target.value);

        // Display the RGB values
    });

    // Function to convert hex color to RGB
    function hexToRgb(hex) {
        // Remove the hash character if present
        hex = hex.replace(/^#/, '');

        // Parse the hex values to RGB
        var bigint = parseInt(hex, 16);
        var r = (bigint >> 16) & 255;
        var g = (bigint >> 8) & 255;
        var b = bigint & 255;
        return [r, g, b];
    }

});
var dropdown = document.getElementById('ledLightFunction');
let selectedValue = 0;
dropdown.addEventListener('change', function () {
    // Get the selected value from the dropdown
    selectedValue = dropdown.value;

    // Display or use the selected value as needed
    console.log("Selected value: " + selectedValue);
});


document.getElementById("btn-submit").addEventListener("click", async function () {
    console.log("Enter submit");
    let topic, message;
    switch (selectedValue) {
        case 0:
            topic = 'led/control/color';
            const colorObject = {
                red: rgbArray[0],
                green: rgbArray[1],
                blue: rgbArray[2],
            };
            message = `JSON:${JSON.stringify(colorObject)}`
            break;
        case 1:
            //set topic and message for 
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
    }
    await eel.publish_to_mqtt(topic, message)();
})
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
