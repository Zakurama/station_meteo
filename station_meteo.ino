#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>          
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Utile pour les mots de passe
#include "config.h"

// Déclaration de la broche d'entrée du capteur de temperature
#define KY001_Signal_PIN 5
// Configuration des libraries
OneWire oneWire(KY001_Signal_PIN);          
DallasTemperature sensors(&oneWire);   

// Déclaration de la broche d'entrée du capteur d'humidité
#define DHTPIN 2     
// Le capteur est initialisé
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Connexion Wifi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Certificat serveur
const char* rootCACertificate = CERTIFICATE;

WiFiClientSecure client;

void setup()
{
    Serial.begin(115200);
    
    // Initialisation des capteurs
    
    sensors.begin();
    dht.begin();
    delay(10);
    
    // Connexion Wifi

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Connexion au serveur
    client.setCACert(rootCACertificate);

    if (!client.connect("meteo-iot.rezoleo.fr", 443)) {
        Serial.println("Échec de connexion au serveur !");
        return;
    }
    else{
      Serial.println("Connexion au serveur réussie !");
      }
}

void loop() {
  
    // On récupère les valeurs des capteurs
    sensors.requestTemperatures(); 
    float temperature = sensors.getTempCByIndex(0);
    float humidity = dht.readHumidity();
    
    // On envoie les infos au serveur
    String password = SERVER_PASSWORD;  
    String jsonPayload = "{\"temperature\": " + String(temperature) + ", \"pression\": 1013, \"humidite\": "+ String(humidity) +", \"password\":"+ password +"}";

    // Envoyer une requête POST
    client.println("POST https://meteo-iot.rezoleo.fr/recup_info_meteo.php HTTP/1.1");
    client.println("Host: meteo-iot.rezoleo.fr");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonPayload.length());
    client.println();
    client.println(jsonPayload);
    Serial.println("Payload envoyée : " + jsonPayload);

    delay(10000);  // Attendre avant de renvoyer
}
