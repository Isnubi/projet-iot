#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>

// ===================== PINS =====================

// LEDs sur pins PWM
#define LED_VERTE 6
#define LED_ROUGE 5
#define LED_POMPE 3

// Capteurs
#define DHTPIN 13
#define DHTTYPE DHT22
#define SOIL_PIN A0
#define POT_LED A1
#define BUTTON_PIN 4

// Pompe (relais)
#define RELAIS_POMPE 12

// ===================== OBJETS =====================
DHT dht(DHTPIN, DHTTYPE);
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
WiFiSSLClient wifiClient;
MqttClient mqttClient(wifiClient);

// ===================== VARIABLES =====================
bool pompeState = false;
bool tslPresent = false;

unsigned long lastRead = 0;
unsigned long lastMqttSend = 0;
unsigned long lastPumpActivation = 0;

float humAir = NAN;
float tempAir = NAN;
float lux = -1;
int humSol = -1;
int humValue = -1;

const char broker[] = "108d626db3644648a1521cc3fc8f0112.s1.eu.hivemq.cloud";
int port = 8883;

char mqttUser[] = "isnubi";
char mqttPass[] = "Bornforrevolution1*";

char ssid[] = "devolo-007";
char pass[] = "TJPBTRLVHXRHJILM";

bool serialEnabled = false;

// ===================================================
void connectWiFi() {
  Serial.print("Connexion WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  Serial.print("Connexion MQTT...");
  mqttClient.setUsernamePassword(mqttUser, mqttPass);
  while (!mqttClient.connect(broker, port)) {
    Serial.print("Erreur MQTT: ");
    Serial.println(mqttClient.connectError());
    delay(2000);
  }
  Serial.println("\nMQTT connecté");
}

// ===================================================
// ===================== SETUP ========================
// ===================================================
void setup() {
  Serial.begin(9600);
  delay(2000);

  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_POMPE, OUTPUT);
  pinMode(RELAIS_POMPE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RELAIS_POMPE, LOW);  // pompe OFF

  dht.begin();

  if (tsl.begin()) {
    tsl.setGain(TSL2591_GAIN_MED);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
    tslPresent = true;
    Serial.println("TSL2591 OK");
  } else {
    Serial.println("TSL2591 ABSENT");
  }

  connectWiFi();
  connectMQTT();

  Serial.println("=== SYSTEME ARROSAGE UNO PRET ===");
  Serial.println("Commandes : POMPE ON | POMPE OFF | STATUS");
}

// ===================================================
// ===================== LOOP =========================
// ===================================================
void loop() {
  // Intensité LEDs
  int potValue = analogRead(POT_LED);
  int brightness = map(potValue, 0, 1023, 0, 255);

  // Activation manuelle de la pompe avec le bouton
  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;
  if (buttonPressed && !pompeState) {
    pompeState = true;
    digitalWrite(RELAIS_POMPE, HIGH);
    analogWrite(LED_POMPE, brightness);
    delay(1000); // Pompe activée pendant 1 seconde
    lastPumpActivation = millis();
    pompeState = false;
    digitalWrite(RELAIS_POMPE, LOW);
    digitalWrite(LED_POMPE, LOW);
  }

  // Lecture des capteurs (toutes les 2 secondes)
  if (millis() - lastRead > 2000) {
    lastRead = millis();

    // DHT22
    humAir = dht.readHumidity();
    tempAir = dht.readTemperature();

    // Hygromètrie du sol 
    humValue = analogRead(SOIL_PIN);
    humValue = constrain(humValue, 400, 1023);
    humSol = map(humValue, 400, 1023, 100, 0);

    // Luminosité
    if (tslPresent) {
      sensors_event_t event;
      tsl.getEvent(&event);
      lux = event.light ? event.light : 0;
    }

    // Gestion des LEDs en fonction de l'humidité du sol
    if (!isnan(humSol)) {
      if (humSol > 30.0) {
        analogWrite(LED_VERTE, brightness);
        digitalWrite(LED_ROUGE, LOW);
      } else {
        digitalWrite(LED_VERTE, LOW);
        analogWrite(LED_ROUGE, brightness);
      }

      // Activation automatique de la pompe si l'humidité du sol est < 30% avec un délai de 5 minutes entre chaque activation
      if (humSol < 30 && !pompeState && millis() - lastPumpActivation > 300000) {
        pompeState = true;
        digitalWrite(RELAIS_POMPE, HIGH);
        analogWrite(LED_POMPE, brightness);
        delay(1000); // Pompe activée pendant 1 seconde
        pompeState = false;
        digitalWrite(RELAIS_POMPE, LOW);
        digitalWrite(LED_POMPE, LOW);
        lastPumpActivation = millis();
      }
    } else {
      digitalWrite(LED_VERTE, LOW);
      digitalWrite(LED_ROUGE, LOW);
    }
  }

  // Envoi MQTT (toutes les 5 secondes)
  if (millis() - lastMqttSend > 5000) {
    mqttClient.poll();

    lastMqttSend = millis();
    sendMQTT(humAir, tempAir, humSol, lux, pompeState);

    // Affichage série
    if (serialEnabled == true) {
      Serial.print("Air H: ");
      Serial.print(isnan(humAir) ? -1 : humAir);
      Serial.print("% | T: ");
      Serial.print(isnan(tempAir) ? -1 : tempAir);
      Serial.print("C | Sol: ");
      Serial.print(humSol);
      Serial.print("% (");
      Serial.print(humValue);
      Serial.print(") | Lux: ");
      if (lux >= 0) Serial.println(lux);
      else Serial.println("N/A");
    }
  }

  handleSerial();
}

void handleSerial() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "POMPE ON") {
      // On récupère l'intensité pour la LED
      int potValue = analogRead(POT_LED);
      int brightness = map(potValue, 0, 1023, 0, 255);

      pompeState = true;
      digitalWrite(RELAIS_POMPE, HIGH);
      analogWrite(LED_POMPE, brightness);
      Serial.println("Pompe ACTIVEE");
    } else if (cmd == "POMPE OFF") {
      pompeState = false;
      digitalWrite(RELAIS_POMPE, LOW);
      digitalWrite(LED_POMPE, LOW);
      Serial.println("Pompe DESACTIVEE");
    } else {
      Serial.println("Commande inconnue");
    }
  }
}

// ===================================================
void sendMQTT(float humAir, float tempAir, float humSol, float lux, bool pompeState) {
  if (serialEnabled == true) {
    Serial.println("Envoi MQTT...");
  }
  mqttClient.beginMessage("louisg/arrosage/air/humidity");
  mqttClient.print(humAir);
  mqttClient.endMessage();

  mqttClient.beginMessage("louisg/arrosage/air/temperature");
  mqttClient.print(tempAir);
  mqttClient.endMessage();

  mqttClient.beginMessage("louisg/arrosage/soil");
  mqttClient.print(humSol);
  mqttClient.endMessage();

  mqttClient.beginMessage("louisg/arrosage/light");
  mqttClient.print(lux);
  mqttClient.endMessage();

  // JSON global
  mqttClient.beginMessage("louisg/arrosage/status");
  mqttClient.print("{");
  mqttClient.print("\"temp\":");
  mqttClient.print(tempAir);
  mqttClient.print(",");
  mqttClient.print("\"humAir\":");
  mqttClient.print(humAir);
  mqttClient.print(",");
  mqttClient.print("\"humSol\":");
  mqttClient.print(humSol);
  mqttClient.print(",");
  mqttClient.print("\"lux\":");
  mqttClient.print(lux);
  mqttClient.print("\"}");
  mqttClient.endMessage();
}
