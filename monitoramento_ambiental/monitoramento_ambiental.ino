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
html += "background:linear-gradient(135deg,#020617,#0b0f2a,#1a0b2e);";
html += "color:white;";
html += "min-height:100vh;";
html += "}";

html += ".container{max-width:1000px;margin:auto;padding:20px;}";

html += ".header{";
html += "background:linear-gradient(90deg,#0ea5e9,#7c3aed);";
html += "padding:25px;";
html += "border-radius:22px;";
html += "margin-bottom:20px;";
html += "box-shadow:0 20px 40px rgba(0,0,0,0.5);";
html += "}";

html += ".header h1{font-size:28px;margin-bottom:5px;}";

html += ".status{";
html += "font-size:13px;";
html += "opacity:0.9;";
html += "display:flex;";
html += "align-items:center;";
html += "gap:8px;";
html += "}";

html += ".dot{";
html += "width:8px;";
html += "height:8px;";
html += "background:#22c55e;";
html += "border-radius:50%;";
html += "}";

html += ".grid{";
html += "display:grid;";
html += "grid-template-columns:1fr 1fr;";
html += "gap:18px;";
html += "}";

html += ".card{";
html += "background:rgba(255,255,255,0.05);";
html += "padding:22px;";
html += "border-radius:22px;";
html += "backdrop-filter:blur(14px);";
html += "box-shadow:0 15px 35px rgba(0,0,0,0.45);";
html += "border:1px solid rgba(255,255,255,0.06);";
html += "}";

html += ".card-title{";
html += "font-size:12px;";
html += "letter-spacing:1px;";
html += "text-transform:uppercase;";
html += "opacity:0.6;";
html += "margin-bottom:10px;";
html += "}";

html += ".big{";
html += "font-size:48px;";
html += "font-weight:700;";
html += "}";

html += ".temp{";
html += "color:#a78bfa;";
html += "}";

html += ".hum{";
html += "color:#38bdf8;";
html += "}";

html += ".info{";
html += "font-size:14px;";
html += "opacity:0.85;";
html += "margin-bottom:3px;";
html += "}";

html += ".full{grid-column:1/3;}";

html += ".btn{";
html += "display:block;";
html += "text-align:center;";
html += "margin-top:22px;";
html += "padding:18px;";
html += "border-radius:16px;";
html += "text-decoration:none;";
html += "font-weight:600;";
html += "background:linear-gradient(90deg,#0ea5e9,#7c3aed);";
html += "color:white;";
html += "box-shadow:0 10px 25px rgba(124,58,237,0.4);";
html += "transition:0.2s;";
html += "}";

html += ".btn:hover{transform:translateY(-2px);}";

html += ".footer{";
html += "text-align:center;";
html += "margin-top:18px;";
html += "font-size:12px;";
html += "opacity:0.5;";
html += "}";

html += ".divider{";
html += "height:1px;";
html += "background:rgba(255,255,255,0.08);";
html += "margin:10px 0;";
html += "}";

html += "@media(max-width:700px){";
html += ".grid{grid-template-columns:1fr;}";
html += ".full{grid-column:1;}";
html += "}";

html += "</style></head><body>";

html += "<div class='container'>";

// HEADER
html += "<div class='header'>";
html += "<h1>Monitoramento Ambiental</h1>";
html += "<div class='status'><div class='dot'></div>Sistema Online • Atualização automática</div>";
html += "</div>";

html += "<div class='grid'>";

// STATUS
html += "<div class='card'>";
html += "<div class='card-title'>Status do Sistema</div>";
html += "<div class='info'>Online</div>";
html += "<div class='info'>Uptime: " + getUptime() + "</div>";
html += "</div>";

// PROJETO
html += "<div class='card'>";
html += "<div class='card-title'>Projeto</div>";
html += "<div class='info'><b>Responsáveis</b></div>";
html += "<div class='info'>Ana Clara Grizotto</div>";
html += "<div class='info'>Emily Gabrielle</div>";

html += "<div class='divider'></div>";

html += "<div class='info'><b>Turma:</b> DSTB-18</div>";
html += "<div class='info'><b>Orientador:</b> Prof. Israel</div>";
html += "</div>";

String linha = ultimaLinha();

if (linha != "") {

int p1 = linha.indexOf(',');
int p2 = linha.lastIndexOf(',');

String data = linha.substring(0, p1);
String temp = linha.substring(p1 + 1, p2);
String hum = linha.substring(p2 + 1);

// TEMPERATURA
html += "<div class='card'>";
html += "<div class='card-title'>Temperatura</div>";
html += "<div class='big temp'>" + temp + "°C</div>";
html += "</div>";

// UMIDADE
html += "<div class='card'>";
html += "<div class='card-title'>Umidade</div>";
html += "<div class='big hum'>" + hum + "%</div>";
html += "</div>";

// DATA
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