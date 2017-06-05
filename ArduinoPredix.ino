#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
//DHT
#include <DHT.h>
//ArduinoJson
#include <ArduinoJson.h>
// RextClient Library
#include "RestClient.h"

#define DEBUG_PRINT 1

#define DHTPIN D2
#define LEDPIN D7
#define STATUSLEDPIN D0
#define LIGHTSENSOR D1
#define RELAY1 D5
#define RELAY2 D6
#define DHTTYPE DHT22 //Set for 11, 21, or 22
#define USE_SERIAL Serial1

DHT dht(DHTPIN, DHTTYPE, 26);
WiFiClientSecure client;

// ---------- /Config ----------//
const char* DEVICE_ID = "devicebandung";
//const char* WIFI_SSID = "Windows Phone4569";
//const char* WIFI_PASSWORD = "87654322";
const char* WIFI_SSID = "NOKIA 909.1_1718";
const char* WIFI_PASSWORD = "04536952";
const char* SDKHosting = "tranquil-savannah-54974.herokuapp.com";

//variable for REST Client
RestClient rsclient = RestClient(SDKHosting);
String response;

//variable for lightsensor
int lastLightState;
int currentLightState = LOW;
int numberLoop = 0;

//variable for thermal
unsigned long PUBLISH_THERMAL_INTERVAL = 4000;
unsigned long DETECT_LIGHT_INTERVAL = 4000;
unsigned long SUBSCRIBE_TOPIC_INTERVAL = 2000;
//# of connections
long connection = 0;

unsigned long prevThermalMillis = 0;
unsigned long prevLightMillis = 0;
unsigned long prevSubscriptionMillis = 0;

//temp variable for the prev temperature after the threshold is exceeded
float prevTemp = 0.f;

//function for turning on or off relay
void switchRelay(int relayPin, bool isOn) {
  digitalWrite(relayPin, isOn ? HIGH : LOW);
}

//function for turning on or off led
void switchLed(bool isOn) {
  digitalWrite(LEDPIN, isOn ? HIGH : LOW);
}

//periodically reads the current thermal data and sends to the web server
void updateReadThermalData() {
  unsigned long currentMillis = millis();
  if (currentMillis - prevThermalMillis >= PUBLISH_THERMAL_INTERVAL) {
    prevThermalMillis = currentMillis;

    float curTemp = dht.readTemperature();
    String h = String(dht.readHumidity());    // Read temperature as Fahrenheit (isFahrenheit = true)
    String c = String(curTemp);

    if (isnan(dht.readHumidity()) || isnan(dht.readTemperature())) {
      if (DEBUG_PRINT) {
        Serial.println("Failed to read from DHT sensor!");
      }
      return;
    } else {
      if (DEBUG_PRINT) {
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(c);
        Serial.print(" *C\t\n");
      }
    };

    //Generate JSON
    //String values = "{\"name\": \"ArduinoPredix " + c + "\", \"datapoints\": [[1, "+ h +"], [2, "+ c +"], [0, 0]]}";
    String values = "{\"name\": \"ArduinoPredixDevice\", \"temperature\": "+ c +", \"humidity\": "+ h +"}";

    //Post into webservice
    response = "";
    rsclient.setContentType("application/json");
    int statusCode = rsclient.post("/services/windservices/temperature", values.c_str(), &response);
    Serial.print("Status code from server: ");
    if (statusCode >= 200 && statusCode < 300) {
      Serial.println("SUCCESS");
      
    } else {
      Serial.print("Failed with code ");
      Serial.println(statusCode);
      Serial.print("Response body from server: ");
      Serial.println(response);
    }
    
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");

  rsclient.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Setup!");
}

void loop() {
  updateReadThermalData();
}
