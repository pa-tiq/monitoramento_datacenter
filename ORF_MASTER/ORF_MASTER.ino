#include <esp_now.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>
#include <M5Stack.h>//M5Stack-Core-ESP32

extern const unsigned char CTAV2[];
extern const unsigned char TEMPINVERT[]; //45x45
extern const unsigned char UMIDADEINVERT[]; //30X45
extern const unsigned char MovimentoINVERT[]; //45X45

#define TelaResumo 0
#define TelaMaster 1
#define TelaSlave1 2
#define TelaSlave2 3
#define TelaSlave3 4

//* variaveis globais
//TODO: Mudar para variaveis locais que podem ser acessadas externamente
//Variaveis dos dados ambientais
float HS1 = 0.0;
float HS2 = 0.0;
float HS3 = 0.0;
float TS1 = 0.0;
float TS2 = 0.0;
float TS3 = 0.0;

boolean changedValue = false; //armazena se é necessário atualizar a tela
//* end variaveis globais

void ImprimeResumo(float TM, float HM, boolean Presenca, float TS1, float HS1,  float TS2, float HS2, float TS3, float HS3, boolean changedTela);
void ImprimeSlave(int SlaveId, float Temp, float Hum, boolean changedTela);
void ImprimeMaster(float TM, float HM, boolean Presenca, boolean changedTela);

// Replace with your network credentials (STATION)
const char* ssid = "IMEORF";
const char* password = "ricardofranco";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  float temp;
  float hum;
  String timestamp;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;

JSONVar board;

AsyncWebServer server(80);
AsyncEventSource events("/events");

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  board["id"] = incomingReadings.id;
  board["temperature"] = incomingReadings.temp;
  board["humidity"] = incomingReadings.hum;
  board["readingId"] = String(incomingReadings.readingId);
  board["timestamp"] = timeStamp;
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());

  //SALVA OS DADOS RECEBIDOS NAS VARIAVEIS GLOBAIS AMBIENTAIS
  switch (incomingReadings.id)
  {
    case 1:
      TS1 = incomingReadings.temp;
      HS1 = incomingReadings.hum;
      break;
    case 2:
      TS2 = incomingReadings.temp;
      HS2 = incomingReadings.hum;
      break;
    case 3:
      TS3 = incomingReadings.temp;
      HS3 = incomingReadings.hum;
      break;
    default:
      break;
  }
  changedValue = true;

  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("t value: %4.2f \n", incomingReadings.temp);
  Serial.printf("h value: %4.2f \n", incomingReadings.hum);
  Serial.printf("readingID value: %d \n", incomingReadings.readingId);
  Serial.println();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP-NOW DASHBOARD</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.temperature { color: #fd7e14; }
    .card.humidity { color: #1b78e2; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ESP32 DHT Server</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Medidor 1 - Temperatura</h4>
        <p><span class="reading"><span id="t1"></span> &deg;C</span></p>
        <p class="packet">Leitura em: <span id="rt1"></span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> Medidor 1 - Umidade</h4>
        <p><span class="reading"><span id="h1"></span> &percnt;</span></p>
        <p class="packet">Leitura em: <span id="rh1"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Medidor 2 - Temperatura</h4>
        <p><span class="reading"><span id="t2"></span> &deg;C</span></p>
        <p class="packet">Leitura em: <span id="rt2"></span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> Medidor 2 - Umidade</h4>
        <p><span class="reading"><span id="h2"></span> &percnt;</span></p>
        <p class="packet">Leitura em: <span id="rh2"></span></p>
      </div>
       <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> Medidor 3 - Temperatura</h4>
        <p><span class="reading"><span id="t3"></span> &deg;C</span></p>
        <p class="packet">Leitura em: <span id="rt3"></span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> Medidor 3 - Umidade</h4>
        <p><span class="reading"><span id="h3"></span> &percnt;</span></p>
        <p class="packet">Leitura em: <span id="rh3"></span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');

 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);

 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);

 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);
  document.getElementById("t"+obj.id).innerHTML = obj.temperature.toFixed(2);
  document.getElementById("h"+obj.id).innerHTML = obj.humidity.toFixed(2);
  document.getElementById("rt"+obj.id).innerHTML = obj.timestamp;
  document.getElementById("rh"+obj.id).innerHTML = obj.timestamp;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setRotation(2);
  //M5.Lcd.printf("Display Test!");
  M5.Lcd.pushImage(11, 10, 217, 300, (uint16_t *)CTAV2);

  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);

  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();

  timeClient.begin();
  timeClient.setTimeOffset(3600);
}

void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping", NULL, millis());
    lastEventTime = millis();
  }
  // variavel ambiental
  float TM = 0.0;
  float HM = 0.0;
  boolean Presenca = false;
  //variavel da tela
  boolean changedTela = true;
  int TelaAtual = TelaResumo;
  int brightness = 100;

  while(!timeClient.update()){
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  //
  while (1) {
    M5.update(); //Read the press state of the key.   A, B, C
    if (M5.BtnC.wasReleased()) {
      changedTela = true;
      if (TelaAtual > 0) {
        TelaAtual = TelaAtual - 1;
      }
      else {
        TelaAtual = 4;
      }
      // Serial.print("BtnA");

    } else if (M5.BtnB.wasReleased()) {
      changedTela = true;
      TelaAtual = TelaResumo;
      Serial.print("BtnB menos de 3s");
    } else if (M5.BtnA.wasReleased()) {
      changedTela = true;
      TelaAtual = TelaAtual + 1;
      TelaAtual = TelaAtual % 5;
      //Serial.print("BtnC");
    } else if (M5.BtnB.pressedFor(3000)) {
      changedTela = true;
      brightness = brightness + 100;
      brightness = brightness % 200; //chaveia a tela ligada/desligada (brightness 0 ou 100)
      M5.Lcd.setBrightness(brightness);
      TelaAtual = TelaResumo;
      while (!M5.BtnB.wasReleased())
      {
        M5.update();
        delay(50);
      }
      //Serial.print("BtnB mais de 3s");
    }

    if ((changedTela || changedValue) && brightness != 0)
    {
      switch (TelaAtual)
      {
        case TelaResumo:
          ImprimeResumo(TM, HM, Presenca, TS1, HS1, TS2, HS2, TS3, HS3, changedTela);
          break;
        case TelaMaster:
          ImprimeMaster(TM, HM, Presenca, changedTela);
          break;
        case TelaSlave1:
          ImprimeSlave(1, TS1, HS1, changedTela);
          break;
        case TelaSlave2:
          ImprimeSlave(2, TS2, HS2, changedTela);
          break;
        case TelaSlave3:
          ImprimeSlave(3, TS3, HS3, changedTela);
          break;
        default:
          TelaAtual = TelaResumo;
          ImprimeResumo(TM, HM, Presenca, TS1, HS1, TS2, HS2, TS3, HS3, changedTela);
          break;
      }
      changedTela = false;
      changedValue = false;
    }
  }
}

void ImprimeResumo(float TM, float HM, boolean Presenca, float TS1, float HS1,  float TS2, float HS2, float TS3, float HS3,boolean changedTela)
{
  if (changedTela)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
  } 
  M5.Lcd.setCursor(30, 5);
  M5.Lcd.println("Medidor Central");
  M5.Lcd.printf("Temperatura: %.1f", TM);
  M5.Lcd.print((char)247);
  M5.Lcd.printf("C \nUmidade: %.1f%% \n", HM);
  M5.Lcd.printf("Sensor de Presenca: ");
  if (Presenca)
  { M5.Lcd.println("Detectada");
  }
  else {
    M5.Lcd.println("Nao Detectada");
  }
  M5.Lcd.println();
  M5.Lcd.println("     Medidor 1");
  M5.Lcd.printf("Temperatura: %.1f", TS1);
  M5.Lcd.print((char)247);
  M5.Lcd.println("C");
  M5.Lcd.printf("Umidade: %.1f%%", HS1);
  M5.Lcd.println();
  M5.Lcd.println();
  M5.Lcd.println("     Medidor 2");
  M5.Lcd.printf("Temperatura: %.1f", TS2);
  M5.Lcd.print((char)247);
  M5.Lcd.println("C");
  M5.Lcd.printf("Umidade: %.1f%%", HS2);
  M5.Lcd.println();
  M5.Lcd.println();
  M5.Lcd.println("     Medidor 3");
  M5.Lcd.printf("Temperatura: %.1f", TS3);
  M5.Lcd.print((char)247);
  M5.Lcd.println("C");
  M5.Lcd.printf("Umidade: %.1f%%", HS3);
}

void ImprimeSlave(int SlaveId, float Temp, float Hum,boolean changedTela)
{
  int ypos = 50;
  int ypos2 = ypos + 55;
  
  if (changedTela)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.printf("Medidor %d", SlaveId);
    M5.Lcd.println();
    
    M5.Lcd.pushImage(5, ypos, 45, 45, (uint16_t *)TEMPINVERT);
  }
  M5.Lcd.setCursor(53, ypos + 10);
  M5.Lcd.printf("%.1f", Temp);
  M5.Lcd.print((char)247);
  M5.Lcd.println("C");
  
  M5.Lcd.pushImage(10, ypos2, 30, 45, (uint16_t *)UMIDADEINVERT);
  M5.Lcd.setCursor(53, ypos2 + 10);
  M5.Lcd.printf("%.1f%%", Hum);
}

void ImprimeMaster(float TM, float HM, boolean Presenca,boolean changedTela)
{
  int ypos = 50;
  int ypos2 = ypos + 55;
  int ypos3 = ypos2 + 55;
  
  if (changedTela)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(50, 10);
    M5.Lcd.printf("Central");
    
    M5.Lcd.pushImage(5, ypos, 45, 45, (uint16_t *)TEMPINVERT);
  }
  M5.Lcd.setCursor(63, ypos + 10);
  M5.Lcd.printf("%.1f", TM);
  M5.Lcd.print((char)247);
  M5.Lcd.println("C");
  
  M5.Lcd.pushImage(10, ypos2, 30, 45, (uint16_t *)UMIDADEINVERT);
  M5.Lcd.setCursor(63, ypos2 + 10);
  M5.Lcd.printf("%.1f%%", HM);
  
  M5.Lcd.pushImage(10, ypos3, 45, 45, (uint16_t *)MovimentoINVERT);
  M5.Lcd.setCursor(63, ypos3 + 10);
  if (Presenca)
  { M5.Lcd.println("Detectada");
  }
  else {
    M5.Lcd.println("-----");
  }

}
