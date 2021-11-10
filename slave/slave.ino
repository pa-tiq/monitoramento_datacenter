#include "WiFi.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <esp_now.h>

#define DHTTYPE DHT11 // Tipo de Sensor DHT 11
#define DHTPIN 2 // Digital pin connected to the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

//WiFi network name and password 
const char * ssid = "IMEORF";
const char * pwd = "ricardofranco";

// REPLACE WITH THE RECEIVER'S MAC Address
//AC:67:B2:38:F7:F8
uint8_t broadcastAddress[] = {0xAC, 0x67, 0xB2, 0x38, 0xF7, 0xF8};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    int id; // must be unique for each sender board
    String temp;
    String hum;
} struct_message;

struct_message myData;


// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

void setup(){
  Serial.begin(115200);
  dht.begin();
    
  //Connect to the WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pwd);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("conectando..");
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

   // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("");
  Serial.print("conectado a ");
  Serial.println(ssid);
  Serial.print("endereço IP: ");
  Serial.println(WiFi.localIP());
  
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

}

void loop(){
  // Set values to send
  myData.id = 1;
  myData.temp = readDHTTemperature();
  myData.hum = readDHTHumidity();

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(10000);    
}
