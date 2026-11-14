#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
// WiFi credentials
const char* ssid = "username";
const char* password = "1234";
// Google Script ID - deploy as web app and get the URL
const char* scriptURL = "https://script.google.com/macros/s/AKfycbwy2u_rxUpDJ7nRb-ftISVAYIjtINiHuiSxDKuSe2dp7MHHmDuYzT7TkEdYzVCmqoA/exec";
// DHT sensor configuration
#define DHTPIN 16 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);
// Data sending interval (in milliseconds)
const unsigned long sendInterval = 600000; // 10 minutes
unsigned long previousMillis = 0;
void setup() {
// Initialize serial communication
Serial.begin(115200);
delay(1000);
Serial.println("ESP32 DHT11 Sensor Data Logger");
// Initialize DHT sensor
dht.begin();
// Connect to WiFi
WiFi.begin(ssid, password);
Serial.print("Connecting to WiFi");
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println();
Serial.print("Connected to WiFi with IP: ");
Serial.println(WiFi.localIP());
}
void loop() {
unsigned long currentMillis = millis();
// Check if it's time to send data
if (currentMillis - previousMillis >= sendInterval) {
previousMillis = currentMillis;
// Read temperature and humidity
float humidity = dht.readHumidity();
float temperature = dht.readTemperature();
// Check if reading was successful
if (isnan(humidity) || isnan(temperature)) {
Serial.println("Failed to read from DHT sensor!");
return;
}
Serial.print("Temperature: ");
Serial.print(temperature);
Serial.print("°C, Humidity: ");
Serial.print(humidity);
Serial.println("%");
// Send data to Google Sheets
sendDataToGoogleSheets(temperature, humidity);
}
}
void sendDataToGoogleSheets(float temperature, float humidity) {
// Check WiFi connection
if (WiFi.status() != WL_CONNECTED) {
Serial.println("WiFi not connected");
return;
}
HTTPClient http;
http.begin(scriptURL);
http.addHeader("Content-Type", "application/json");
// Create JSON data
StaticJsonDocument<200> doc;
doc["temperature"] = temperature;
doc["humidity"] = humidity;
String jsonString;
serializeJson(doc, jsonString);
// Send HTTP POST request
int httpResponseCode = http.POST(jsonString);
if (httpResponseCode > 0) {
String response = http.getString();
Serial.println("HTTP Response code: " + String(httpResponseCode));
Serial.println("Response: " + response);
} else {
Serial.print("Error on sending POST: ");
Serial.println(httpResponseCode);
}
http.end();
}