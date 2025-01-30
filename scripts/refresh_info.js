let luminosityElement = document.getElementById("luminosity")
let temperatureElement = document.getElementById("temperature")
let humidityElement = document.getElementById("humidity")
let luminosityDivElement = document.getElementById("luminosity-div")
let temperatureDivElement = document.getElementById("temperature-div")

// Function to fetch the meteorological data
function getMeteoData() {
    fetch('get_info_meteo.php')
        .then(response => response.json())
        .then(data => {
            // Log the data to the console or do something with it
            if (data.status && data.status === 'error') {
                console.log('Error:', data.message);
            } else {
                temperatureElement.innerHTML = data.temperature + "°C";
                luminosityElement.innerHTML = data.luminosity + "hPa";
                humidityElement.innerHTML = data.humidite + "%";
                verify_extreme_conditions(data.temperature);
            }
        })
        .catch(error => {
            console.error('Error fetching data:', error);
        });
}

// Set interval to fetch data every 2 seconds (2000 milliseconds)
setInterval(getMeteoData, 2000);
getMeteoData(); // Call the function once to load the data immediately

function verify_extreme_conditions(temperature) {
    if (temperature > 30) {
        temperatureDivElement.style.backgroundColor = "rgba(213, 12, 12, 1)";
    }
    else if (temperature < 5) {
        temperatureDivElement.style.backgroundColor = "rgba(33, 172, 230, 0.72)";
    }
    else {
        temperatureDivElement.style.backgroundColor = "";
    }
}
