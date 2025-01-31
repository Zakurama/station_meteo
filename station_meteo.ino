#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>          
#include <Arduino.h>
#include <Wire.h>
#include <ArtronShop_BH1750.h>
#include <ArduinoJson.h>
#include "Adafruit_Si7021.h"

// Utile pour les mots de passe
#include "config.h"

#define FREQ_MESURE 5000 // ms
#define WIFI_DELAY 500 // ms
#define NB_MESURES_STOCKEE 100

// Connexion Wifi
const char* ssid_wifi = WIFI_SSID;
const char* password_wifi = WIFI_PASSWORD;

// Information serveur
const char* rootCACertificate = CERTIFICATE;
const char* url_serveur = SERVER_URL;
const int serveur_port = SERVER_PORT;
const char* server_password = SERVER_PASSWORD;  
const char* post_endpoint = POST_ENDPOINT;

// Création des différents objets
WiFiClientSecure client;
Adafruit_Si7021 sensor = Adafruit_Si7021();
ArtronShop_BH1750 bh1750(0x23, &Wire); 

// Création du format d'envoi JSON 
const size_t CAPACITY = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(NB_MESURES_STOCKEE) + NB_MESURES_STOCKEE * JSON_OBJECT_SIZE(4);  
// 2 car deux champs dans le JSON (password et data) et 4 parce data a 4 champs (température, luminosite, humidite, timestamp)
DynamicJsonDocument doc(CAPACITY);
// Tableau de stokage des valeurs (si non connectée au serveur)
JsonArray data = doc.createNestedArray("data");

void setup()
{
    Serial.begin(115200);
        
    // Connexion Wifi

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid_wifi);

    WiFi.begin(ssid_wifi, password_wifi);

    while (WiFi.status() != WL_CONNECTED) {
        delay(WIFI_DELAY);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Connexion au serveur
    
    client.setCACert(rootCACertificate);

    if (!client.connect(url_serveur, serveur_port)) {
        Serial.println("Échec de connexion au serveur !");
        return;
    }
    else{
      Serial.println("Connexion au serveur réussie !");
      }
    
    //Setup des capteurs
    Wire.begin();
    if (!bh1750.begin()) {
      Serial.println("BH1750 non trouvé");
      while(true);
    }
  
    if (!sensor.begin()) {
      Serial.println("Si7021 non trouvé");
      while (true);
    }

    Serial.println("Capteurs bh1750 et Si7021 trouvés");

    // Ajout du mot de passe au JSON
    doc["password"] = server_password;
}

void loop() {

     // On récupère les valeurs des capteurs
     float luminosite = bh1750.light(); // En lux 
     float humidite = sensor.readHumidity();// En % ?
     float temperature = sensor.readTemperature();// En C°
        
     // Création d'une nouvelle mesure
     JsonObject measurement = data.createNestedObject();
     measurement["temperature"] = temperature;
     measurement["luminosity"] = luminosite;
     measurement["humidite"] = humidite;
     measurement["timestamp"] = millis();
    
     if (WiFi.status() != WL_CONNECTED){
         // On essaie de se reconnecter
         Serial.println("Station non connectée au Wifi ! Reconnexion en cours...");
         WiFi.disconnect();
         WiFi.reconnect();
     }
     else if (!client.connected()) {
         Serial.println("Station non connectée au serveur, tentative de reconnexion...");
         client.stop();  // Fermer la connexion précédente
         if (client.connect(url_serveur, serveur_port)) {
             Serial.println("Reconnecté au serveur");
          } else {
             Serial.println("Échec de la reconnexion");
          }
      }

     else{

        // Conversion JSON en String
        String jsonPayload;
        serializeJson(doc, jsonPayload);
        
        // Envoyer une requête POST
        client.println("POST "+ String(post_endpoint)+" HTTP/1.1"); 
        client.println("Host: " + String(url_serveur));
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(jsonPayload.length());
        client.println();
        client.println(jsonPayload);
        Serial.println("Payload envoyée : " + jsonPayload);

        // Réinitialisation du buffer
        data.clear();
    }
    
    delay(FREQ_MESURE);  // Attendre avant de renvoyer
}
