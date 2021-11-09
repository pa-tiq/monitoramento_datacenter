#include "WiFi.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <AsyncUDP.h>

#define DHTTYPE DHT11 // Tipo de Sensor DHT 11
#define DHTPIN 2 // Digital pin connected to the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

//WiFi network name and password 
const char * ssid = "IMEORF";
const char * pwd = "ricardofranco";

AsyncUDP udp;

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
  WiFi.begin(ssid, pwd);
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("conectando..");
  }
  Serial.println("");
  Serial.print("conectado a ");
  Serial.println(ssid);
  Serial.print("endereço IP: ");
  Serial.println(WiFi.localIP());
  
  if(udp.connect(IPAddress(192,168,0,153), 1234)) {
        Serial.println("UDP connected");
        udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            //packet.printf("Got %u bytes of data", packet.length());
        });
        //Send unicast
        //udp.print("Hello Server!");
    }
    else{
      Serial.println("UDP failed");
    }

}

void loop(){
    delay(5000);
    //Send broadcast on port 1234
    udp.broadcastTo("Anyone here?", 1234);
    Serial.println("pacote udp enviado");
    
}
