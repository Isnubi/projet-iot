# Listes des composants - Projet IoT

## AUTHORS

- Louis GAMBART
- Guillaume MECENE

## Rappel du contexte

Le projet vise à concevoir un système complet de surveillance environnementale permettant :
- la **mesure locale** de différents paramètres (humidité du sol, température, luminosité…),
- l’**affichage sur un écran LCD**,  
- la **transmission longue portée via LoRa**,  
- l’envoi des données vers un **serveur MQTT** via ESP32,  
- l’exploitation des données dans un **dashboard externe**.

## Partie noeud capteur

### Matériel déjà à disposition

- Arduino Mega 2560
- Module LoRA SX1278
- Antenne 433MHz pour SX1278
- Capteur de température et d'humidité DHT22
- Ecran LCD 16x2
- Bouton poussoir
- 2 x LED verte
- 2 x LED rouge
- 2 x LED bleue
- Résistance 220 ohms
- Résistance 4,7k ohms
- 19 x Breadboard medium
- Câble USB pour Arduino Mega
- Câbles Dupont

### Matériel nécessaire
- Capteur d'humidité au sol FC-28 + module analogique
    - [Lien Amazon.fr](https://amazon.fr/dp/B07CNRJN8W)
    - [Lien Amazon.com.be](https://amazon.com.be/dp/B07V6SZYZW)
- 2 x Potentiomètre 10k ohms
    - [Lien Amazon.fr](https://amazon.fr/dp/B0734K6WYP)
    - [Lien Amazon.com.be](https://amazon.com.be/dp/B0734K6WYP)
- 1 x Pompe à eau
    - [Lien Amazon.fr](https://amazon.fr/dp/B082PM8L6X)
    - [Lien Amazon.com.be](https://amazon.com.be/dp/B07F9MT92K)

## Partie ESP32

### Matériel déjà à disposition

- ESP32 DevKit V1 (ESP32-WROOM-32)
- Module LoRa SX1278 (Ra-01 AI-Thinker) × 2
- Antennes 433 MHz pour SX1278 × 2
- Câble USB pour ESP32
- Câbles Dupont
- 19 x Breadboard
- Câbles Dupont
