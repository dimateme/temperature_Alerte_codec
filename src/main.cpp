#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include "TemperatureStub.h"
#include <PubSubClient.h>

// sensor dhht22
#include <DHT.h>
#define DHTPIN 27
#define DHTTYPE DHT22
TemperatureStub *myTemp;

// Variable pour la connection Wifi
/*const char *SSID = "EcoleDuWeb2.4g";
const char *PASSWORD = "EcoleDuWEB";
const char USERNAME[] = "paul";
const char MQTT_PASS[] = "Savoir7$";
const char MQTT_SERVER[] = "172.16.5.103";*/

const char *SSID = "Helix-34";
const char *PASSWORD = "Qwerty122";
const char USERNAME[] = "polo";
const char MQTT_PASS[] = "polo";
const char MQTT_SERVER[] = "10.0.0.135";

const uint16_t MQTT_PORT = 1883;
const char MQTT_CLIENT[] = "ESP32_MQTT";

const char TOPIC_SUB[] = "topic_sub";
const char TOPIC_PUB[] = "esp32/temperature";

char bufferTemperature[100]; // le buffer permet de stocker la valeur de la température et de la convertir en string
float temperature = 0.0f;    // la température du sensor dht22
float temperaturePrecedente = 0.0f;

void messageHandler(char *topic, uint8_t *payload, unsigned int len)
{
  Serial.print(topic);
  Serial.print(" with value: ");
  for (uint16_t i = 0; i < len; ++i)
    Serial.print((char)payload[i]);
  Serial.println();
}

void mqttPubSub(void)
{
  WiFiClient client;
  PubSubClient mqtt(client);

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);

  mqtt.setCallback(messageHandler);

  while (1)
  {
    if (!mqtt.connected())
    {
      Serial.print("Connecting to MQTT broker \"");
      Serial.print(MQTT_SERVER);
      Serial.println("\"...");
      if (mqtt.connect(MQTT_CLIENT))
      {
        Serial.println("Connected to MQTT broker");
        if (!mqtt.subscribe(TOPIC_SUB))
        {
          Serial.println("Error subscribing to the topic!");
        }
      }
      else
      {
        Serial.println("Failed to connect to MQTT!");
        delay(1000);
      }
    }
    else
    {
      mqtt.loop();
      temperaturePrecedente = temperature;
      temperature = myTemp->getTemperature();
      // comparaison de la temperature precedente et la temperature actuelle
      if (temperature != temperaturePrecedente)
      {
        sprintf(bufferTemperature, "%4.1f", temperature);
        Serial.println(bufferTemperature);
        if (!mqtt.publish(TOPIC_PUB, String(bufferTemperature).c_str()))
        {
          Serial.println("Error publishing uptime topic!");
        }
      }

      delay(1000);
    }
  }
}
void setup()
{

  Serial.begin(9600);
  delay(100);

  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  Serial.print("Connecting to SSID \"");
  Serial.println(SSID);

  while (!WiFi.isConnected()) // en attente de la connection
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println(); // connection est établie
  Serial.print("Connected to WiFi with IP ");
  Serial.println(WiFi.localIP());
  // Gestion de la temperature
  myTemp = new TemperatureStub();
  myTemp->init(DHTPIN, DHT22);
  temperature = myTemp->getTemperature();
  sprintf(bufferTemperature, "%4.1f", temperature);
}

void loop()
{
  mqttPubSub();
}