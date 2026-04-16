#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
 
// ===== WIFI =====
const char* ssid = "smart_city";
const char* password = "senai501";
 
IPAddress local_IP(192, 168, 0, 52); // Alterar conforme tabela
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
 
// ===== IDs DOS SENSORES =====
String Sensor_ID_Temp = "3";  // Alterar conforme tabela
String Sensor_ID_Hum  = "4";  // Alterar conforme tabela
 
// ===== DHT =====
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
 
// ===== API =====
String baseURL = "http://192.168.0.101";
String token = "";
String timestamp = "";
 
// ===== CONTROLE =====
unsigned long tempoAnterior = 0;
unsigned long intervalo = 30000;
 
// ===== WIFI =====
void conectarWiFi() {
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
 
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("\nConectado!");
  Serial.println(WiFi.localIP());
}
 
// ===== GET TIMESTAMP =====
void obterTimestamp() {
  HTTPClient http;
  http.begin(baseURL + "/api/time/");
 
  int httpCode = http.GET();
 
  if (httpCode == 200) {
    String payload = http.getString();
 
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
 
    timestamp = doc["timestamp"].as<String>();
    Serial.println("Timestamp: " + timestamp);
  }
 
  http.end();
}
 
// ===== GET TOKEN =====
void obterToken() {
  HTTPClient http;
  http.begin(baseURL + "/api/token/");
  http.addHeader("Content-Type", "application/json");
 
  String body = "{\"username\":\"smart_city\",\"password\":\"senai501\"}";
 
  int httpCode = http.POST(body);
 
  if (httpCode == 200) {
    String payload = http.getString();
 
    StaticJsonDocument<300> doc;
    deserializeJson(doc, payload);
 
    token = doc["access"].as<String>();
    Serial.println("Token obtido");
    Serial.println(token);
  }
 
  http.end();
}
 
// ===== POST GENERICO =====
void enviarDado(String url, float valor, String sensorID) {
  HTTPClient http;
 
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + token);
 
  StaticJsonDocument<200> doc;
  doc["valor"] = String(valor);
  doc["timestamp"] = timestamp;
  doc["sensor"] = sensorID;
 
  String body;
  serializeJson(doc, body);
 
  int httpCode = http.POST(body);
 
  Serial.println("POST " + url + " -> " + httpCode);
 
  http.end();
}
 
// ===== LEITURA E ENVIO =====
void lerEEnviar() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
 
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Erro leitura DHT");
    return;
  }
 
  obterTimestamp();
 
  enviarDado(baseURL + ":8000/api/temperatura/", temp, Sensor_ID_Temp);
  enviarDado(baseURL + ":8000/api/umidade/", hum, Sensor_ID_Hum);
}
 
// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  dht.begin();
 
  conectarWiFi();
  obterToken();
}
 
// ===== LOOP =====
void loop() {
  if (millis() - tempoAnterior >= intervalo) {
    tempoAnterior = millis();
    lerEEnviar();
  }
}