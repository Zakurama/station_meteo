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

#define FREQ_MESURE 10 // s
#define WIFI_DELAY 500 // ms
#define WIFI_ATTENTE_CONNEXION 5
#define MAX_LEN_ONE_DATA 64
#define MAX_LEN_BUFFER_DATA 512

#define uS_TO_S_FACTOR 1000000
#define ms_TO_S_FACTOR 1000

// Connexion Wifi
const char* ssid_wifi = WIFI_SSID;
const char* password_wifi = WIFI_PASSWORD;

// Information serveur
const char* rootCACertificate = CERTIFICATE;
const char* url_serveur = SERVER_URL;
const int serveur_port = SERVER_PORT;
const char* server_password = SERVER_PASSWORD;  
const char* post_endpoint = POST_ENDPOINT;

// Mémoire RTC. Max 8kb
RTC_DATA_ATTR char buffer_data[MAX_LEN_BUFFER_DATA];
RTC_DATA_ATTR int timestamp;

// Création des différents objets
WiFiClientSecure client;
Adafruit_Si7021 sensor = Adafruit_Si7021();
ArtronShop_BH1750 bh1750(0x23, &Wire);

void enregistrerDonnees() {
  
  // On récupère les valeurs des capteurs
  float luminosite = bh1750.light(); // En lux 
  float humidite = sensor.readHumidity(); // En %
  float temperature = sensor.readTemperature(); // En C°
  timestamp = timestamp +  millis() + FREQ_MESURE*ms_TO_S_FACTOR;
  
  // Formater la nouvelle donnée à ajouter
  char new_data[MAX_LEN_ONE_DATA];
  snprintf(new_data, sizeof(new_data), "%.2f-%.2f-%.2f-%d|", temperature, luminosite, humidite,timestamp);

  // Ajouter la nouvelle donnée à la chaîne stockée dans la mémoire RTC
  strncat(buffer_data, new_data, sizeof(buffer_data) - strlen(buffer_data) - 1);  // On évite le dépassement de mémoire
  
  // Afficher la chaîne mise à jour
  Serial.println("Données enregistrées :");
  Serial.println(buffer_data);
  
}

void envoyer_donnee() {
       
    // Connexion Wifi

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid_wifi);

    WiFi.begin(ssid_wifi, password_wifi);

    for (int t=0; t<WIFI_ATTENTE_CONNEXION; t++){
        delay(WIFI_DELAY);
        Serial.print(".");
    }
    
    client.setCACert(rootCACertificate);
    
    if (WiFi.status() != WL_CONNECTED){
         Serial.println("Station non connectée au Wifi.");
    }
     
    else if (!client.connect(url_serveur, serveur_port)) {
         Serial.println("Station non connectée au serveur.");
    }
    else{

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        Serial.println("Connexion au serveur réussie !");
        
        // Conversion de  dans un JSON au bon format
        String jsonPayload;

        float temperature, luminosity, humidite;
        int timestamp_;
    
        // Commence la chaîne JSON
        jsonPayload = "{\"data\":[";
    
        // Utilise strtok pour découper la chaîne d'entrée selon les séparateurs '|'
        const char* delimiter = "|";
        char* token = strtok((char*)buffer_data, delimiter);
        int first = 1;  // Utilisé pour ajouter une virgule avant les objets sauf le premier
    
        // Parcourt chaque bloc de capteurs
        while (token != NULL) {
            // Extrait les quatre valeurs séparées par '-'
            if (sscanf(token, "%f-%f-%f-%d", &temperature, &luminosity, &humidite, &timestamp_) == 4) {
                if (!first) {
                    // Ajoute une virgule avant chaque objet sauf le premier
                    jsonPayload += ",";
                }
                first = 0;
    
                // Ajoute les valeurs dans le format JSON à la chaîne de sortie
                jsonPayload += "{\"temperature\":";
                jsonPayload += String(temperature, 2);  // Ajoute la température avec 2 décimales
                jsonPayload += ",\"luminosity\":";
                jsonPayload += String(luminosity, 2);   // Ajoute la luminosité avec 2 décimales
                jsonPayload += ",\"humidite\":";
                jsonPayload += String(humidite, 2);     // Ajoute l'humidité avec 2 décimales
                jsonPayload += ",\"timestamp\":";
                jsonPayload += timestamp_;
                jsonPayload += "}";
            }
            token = strtok(NULL, delimiter);  // Récupère le suivant
        }
    
        // Termine la chaîne JSON et ajoute le mot de passe
        jsonPayload += "],\"password\":\"";
        jsonPayload += server_password;
        jsonPayload += "\"}";
        
        // Envoyer une requête POST
        client.println("POST "+ String(post_endpoint)+" HTTP/1.1"); 
        client.println("Host: " + String(url_serveur));
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(jsonPayload.length());
        client.println();
        client.println(jsonPayload);
        Serial.println("Payload envoyée : " + jsonPayload);
        
        // Reset buffer_data
        memset(buffer_data, 0, sizeof(buffer_data));
    }
}

void setup()
{
    Serial.begin(115200);
    
    //Setup des capteurs
    Wire.begin();
    if (!bh1750.begin()) {
      Serial.println("BH1750 non trouvé");
    }
    else if (!sensor.begin()) {
      Serial.println("Si7021 non trouvé");
    }
    else{
       Serial.println("Capteurs bh1750 et Si7021 trouvés");

      if (strlen(buffer_data) <= MAX_LEN_BUFFER_DATA){
        enregistrerDonnees();
      }
      envoyer_donnee();
    }

    esp_sleep_enable_timer_wakeup(FREQ_MESURE * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    
}

void loop() {

}
