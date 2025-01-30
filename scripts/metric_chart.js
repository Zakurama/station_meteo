let humidity_chart = document.getElementById("humidity_chart");
let temperature_chart = document.getElementById("temperature_chart");
let luminosity_chart = document.getElementById("luminosity_chart");

function getMeteoData() {
    fetch('get_all_info_meteo.php') // Appel au fichier PHP
        .then(response => {
            if (!response.ok) {
                throw new Error("Erreur lors de la récupération des données");
            }
            return response.json(); // Conversion de la réponse en JSON
        })
        .then(data => {
            if (data.status === "error") {
                console.error(data.message);
                return;
            }
            createChart(data, humidity_chart, data.humidite,'Humidité (%)'); // Création du graphique avec les données reçues
            createChart(data, temperature_chart, data.temperature,'Température (°C)');
            createChart(data, luminosity_chart, data.luminosity,'Luminosity (?)');
        })
        .catch(error => console.error("Erreur : ", error));
}

function createChart(data, namechart, datatype, dataname) {
    // Création du graphique avec Chart.js
    new Chart(namechart, {
        type: 'line', // Type de graphique
        data: {
            //labels: data.timestamp.map(ts => new Date(ts * 1000).toLocaleString('fr-FR', { hour12: false,timeStyle: 'short', dateStyle: 'short'})), // Convertir timestamp en date lisible
            labels: data.timestamp.map(ts => new Date(ts * 1000)), // Timestamps en millisecondes
            datasets: [
                {
                    label: dataname,
                    //data: data.humidite, // Données d'humidité
                    data: data.timestamp.map((ts, index) => ({
                            x: ts*1000, // Timestamps en millisecondes
                            y: datatype[index]
                        })),
                    backgroundColor: 'rgba(54, 162, 235, 0.2)', // Couleur du fond
                    borderColor: 'rgba(54, 162, 235, 1)', // Couleur de la bordure
                    borderWidth: 1
                }
            ]
        },
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: true // L'axe Y commence à 0
                },
                x: {
                    type: 'time', // Échelle temporelle
                    //stepSize: 10,
                    time: {
                        unit: 'minute', // Granularité : heures
                        displayFormats: {
                            minute: 'HH:mm', // Format 24h
                            hour: 'HH:mm', // Format 24h
                        }
                    },
                    // title: {
                    //     display: true,
                    //     text: 'Heure'
                    // }
                }
            }
        }
    });
}

// Appeler la fonction pour récupérer et afficher les données
getMeteoData();
