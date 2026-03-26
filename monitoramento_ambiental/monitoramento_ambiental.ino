#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"
#include "SPIFFS.h"
#include <time.h>

// ===== CONFIG WiFi (AP) =====
const char* ssid = "SCRM-02";
const char* password = "senai501";

// ===== CONFIG REFRESH =====
int refreshTempo = 10; // segundos (mude aqui)

// ===== DHT11 =====
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== WebServer =====
WebServer server(80);

// ===== Controle de tempo =====
unsigned long tempoAnterior = 0;
unsigned long intervalo = 10000; // leitura sensor

// ===== Uptime =====
String getUptime() {
  long segundos = millis() / 1000;
  int h = segundos / 3600;
  int m = (segundos % 3600) / 60;
  int s = segundos % 60;

  char buffer[20];
  sprintf(buffer,"%02d:%02d:%02d",h,m,s);
  return String(buffer);
}

// ===== Data/Hora =====
String getDataHora() {

  struct tm t;
  char mesStr[4];
  int dia, ano, hora, min, seg;

  sscanf(__DATE__, "%s %d %d", mesStr, &dia, &ano);
  sscanf(__TIME__, "%d:%d:%d", &hora, &min, &seg);

  String meses = "JanFebMarAprMayJunJulAugSepOctNovDec";
  int mes = (meses.indexOf(mesStr) / 3);

  t.tm_year = ano - 1900;
  t.tm_mon = mes;
  t.tm_mday = dia;
  t.tm_hour = hora;
  t.tm_min = min;
  t.tm_sec = seg;

  time_t tempoInicial = mktime(&t);
  time_t tempoAtual = tempoInicial + (millis() / 1000);

  struct tm *tempoFinal = localtime(&tempoAtual);

  char buffer[25];
  sprintf(buffer,"%02d/%02d/%04d %02d:%02d:%02d",
          tempoFinal->tm_mday,
          tempoFinal->tm_mon + 1,
          tempoFinal->tm_year + 1900,
          tempoFinal->tm_hour,
          tempoFinal->tm_min,
          tempoFinal->tm_sec);

  return String(buffer);
}

// ===== SPIFFS =====
void iniciarSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Erro SPIFFS");
    return;
  }

  if (!SPIFFS.exists("/dados.csv")) {
    File file = SPIFFS.open("/dados.csv", FILE_WRITE);
    file.println("DataHora,Temperatura,Umidade");
    file.close();
  }
}

// ===== salvar =====
void salvarLeitura(float temp, float hum) {
  File file = SPIFFS.open("/dados.csv", FILE_APPEND);

  if (!file) return;

  String linha = getDataHora() + "," + String(temp) + "," + String(hum);
  file.println(linha);
  file.close();
}

// ===== ultima linha =====
String ultimaLinha() {
  File file = SPIFFS.open("/dados.csv");
  if (!file) return "";

  String linha, ultima;

  while (file.available()) {
    linha = file.readStringUntil('\n');
    if (linha.length() > 5) ultima = linha;
  }

  file.close();
  return ultima;
}

// ===== HTML =====
String gerarHTML() {

String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
html += "<meta http-equiv='refresh' content='" + String(refreshTempo) + "'>";

html += "<style>";

html += "*{margin:0;padding:0;box-sizing:border-box;font-family:Segoe UI,Arial;}";

html += "body{";
html += "background:linear-gradient(135deg,#070b22,#111633,#2a0a4a);";
html += "color:white;";
html += "min-height:100vh;";
html += "}";

html += ".container{max-width:900px;margin:auto;padding:20px;}";

html += ".header{";
html += "background:linear-gradient(90deg,#4facfe,#8a5cff);";
html += "padding:20px;";
html += "border-radius:20px;";
html += "margin-bottom:20px;";
html += "box-shadow:0 10px 30px rgba(0,0,0,0.4);";
html += "}";

html += ".header h1{margin:0;font-size:26px;}";

html += ".status{";
html += "margin-top:8px;";
html += "font-size:13px;";
html += "opacity:0.9;";
html += "}";

html += ".grid{";
html += "display:grid;";
html += "grid-template-columns:1fr 1fr;";
html += "gap:15px;";
html += "}";

html += ".card{";
html += "background:rgba(255,255,255,0.05);";
html += "padding:20px;";
html += "border-radius:20px;";
html += "box-shadow:0 10px 25px rgba(0,0,0,0.4);";
html += "backdrop-filter:blur(10px);";
html += "}";

html += ".card-title{";
html += "font-size:13px;";
html += "opacity:0.6;";
html += "margin-bottom:8px;";
html += "}";

html += ".big{";
html += "font-size:42px;";
html += "font-weight:bold;";
html += "}";

html += ".temp{color:#8a5cff;}";
html += ".hum{color:#4facfe;}";

html += ".full{grid-column:1/3;}";

html += ".info{font-size:14px;opacity:0.8;}";

html += ".btn{";
html += "display:block;";
html += "text-align:center;";
html += "margin-top:20px;";
html += "padding:16px;";
html += "border-radius:15px;";
html += "text-decoration:none;";
html += "font-weight:bold;";
html += "background:linear-gradient(90deg,#4facfe,#8a5cff);";
html += "color:white;";
html += "box-shadow:0 5px 20px rgba(138,92,255,0.4);";
html += "}";

html += ".footer{";
html += "text-align:center;";
html += "margin-top:15px;";
html += "font-size:12px;";
html += "opacity:0.5;";
html += "}";

html += "@media(max-width:600px){";
html += ".grid{grid-template-columns:1fr;}";
html += ".full{grid-column:1;}";
html += "}";

html += "</style></head><body>";

html += "<div class='container'>";

html += "<div class='header'>";
html += "<h1>Monitoramento Ambiental</h1>";
html += "<div class='status'>Sistema Online • Atualização automática</div>";
html += "</div>";

html += "<div class='grid'>";

html += "<div class='card'>";
html += "<div class='card-title'>Status do Sistema</div>";
html += "<div class='info'>Online</div>";
html += "<div class='info'>Uptime: " + getUptime() + "</div>";
html += "</div>";

html += "<div class='card'>";
html += "<div class='card-title'>Responsáveis</div>";
html += "<div class='info'>Ana Clara Grizotto</div>";
html += "<div class='info'>Emily Gabrielle</div>";
html += "</div>";

String linha = ultimaLinha();

if (linha != "") {

int p1 = linha.indexOf(',');
int p2 = linha.lastIndexOf(',');

String data = linha.substring(0, p1);
String temp = linha.substring(p1 + 1, p2);
String hum = linha.substring(p2 + 1);

html += "<div class='card'>";
html += "<div class='card-title'>Temperatura</div>";
html += "<div class='big temp'>" + temp + "°C</div>";
html += "</div>";

html += "<div class='card'>";
html += "<div class='card-title'>Umidade</div>";
html += "<div class='big hum'>" + hum + "%</div>";
html += "</div>";

html += "<div class='card full'>";
html += "<div class='card-title'>Última leitura</div>";
html += "<div class='info'>" + data + "</div>";
html += "</div>";
}

html += "</div>";

html += "<a class='btn' href='/download'>⬇ Baixar dados XLS</a>";

html += "<div class='footer'>";
html += "Dashboard ESP32 • Monitoramento em tempo real";
html += "</div>";

html += "</div></body></html>";

return html;
}

// ===== download =====
void handleDownload() {
  File file = SPIFFS.open("/dados.csv");

  server.sendHeader("Content-Type","application/vnd.ms-excel");
  server.sendHeader("Content-Disposition","attachment; filename=dados.xls");
  server.send(200,"text/plain",file.readString());

  file.close();
}

// ===== servidor =====
void configurarServidor() {

server.on("/", []() {
server.send(200,"text/html",gerarHTML());
});

server.on("/download", handleDownload);

server.begin();
}

// ===== AP =====
void iniciarAP() {
WiFi.softAP(ssid,password);
Serial.println(WiFi.softAPIP());
}

// ===== sensor =====
void lerSensor() {
float temp = dht.readTemperature();
float hum = dht.readHumidity();

if (!isnan(temp) && !isnan(hum)) {
salvarLeitura(temp, hum);
}
}

// ===== setup =====
void setup() {
Serial.begin(115200);

dht.begin();
iniciarSPIFFS();
iniciarAP();
configurarServidor();
}

// ===== loop =====
void loop() {

server.handleClient();

if (millis() - tempoAnterior >= intervalo) {
tempoAnterior = millis();
lerSensor();
}

}