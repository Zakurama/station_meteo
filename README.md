# Station Météo

Ce projet est constitué d'un code pour ESP32 [station_meteo.ino](station_meteo.ino), et d'un ensemble de fichier pour serveur web.
Le but de ce projet est de récupérer des informations météorologiques à partir de l'ESP32 et d'afficher ces informations en temps réel sur le serveur web.

Ce projet contient des fichiers annexes qui n'ont pas été publiés :
- Un fichier config.h contenant le certificat TLS et le mot de passe de la requête JSON.
- Un dossier private contenant un fichier config.php (contenant le mot de passe) et la base de donnée Sqlite sous le nom info_meteo.db
