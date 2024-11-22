#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>          
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

// Utile pour les mots de passe
#include "config.h"

// Déclaration de la broche d'entrée du capteur de temperature
#define KY001_Signal_PIN 5
// Configuration des libraries
OneWire oneWire(KY001_Signal_PIN);          
DallasTemperature sensors(&oneWire);   

// Définir les broches du BMP280
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10
// Initialiser BMP280
Adafruit_BMP280 bmp;

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
    
    //Setup capteur pression
    if (!bmp.begin()) {  
      Serial.println(F("N'a pas trouvé de capteur BMP280 valide, vérifier le câblage!"));
      while (1);
    }
}

void loop() {
  
    // On récupère les valeurs des capteurs
    sensors.requestTemperatures(); 
    float temperature = sensors.getTempCByIndex(0);
    float humidity = dht.readHumidity();
    float pressure = bmp.readPressure()/100;
    
    // On envoie les infos au serveur
    String password = SERVER_PASSWORD;  
    String jsonPayload = "{\"temperature\": " + String(temperature) + ", \"pression\": "+ String(pressure) +", \"humidite\": "+ String(humidity) +", \"password\":\""+ password +"\"}";

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
