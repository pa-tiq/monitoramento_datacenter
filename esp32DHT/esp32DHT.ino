#include <PString.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>
#define DHTTYPE DHT11 // Tipo de Sensor DHT 11
#define DHTPIN 2 // Digital pin connected to the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

//WiFi network name and password 
const char * ssid = "IMEORF_5G";
const char * pwd = "ricardofranco";

// IP address to send UDP data to.
// it can be ip address of the server or 
// a network broadcast address
// here is broadcast address
const char * udpAddress = "192.168.0.156";
const int udpPort = 9001;
byte buf[255];
//create UDP instance
WiFiUDP udp;


void setup(){
  Serial.begin(115200);
  
  //Connect to the WiFi network
   WiFi.begin(ssid, pwd);
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //This initializes udp and transfer buffer
  udp.begin(udpPort);
  dht.begin();
}

void loop(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float y = 0;
  //data will be sent to server
  //buf = char(h);
  uint8_t buffer[50] = "bbb";
  //send hello world to server
  Serial.println(h);
  Serial.println(t);
  //buffer[50] = h;
  udp.beginPacket(udpAddress, udpPort);
  char hStr[20] = { 0 };
  y = 1000000*t + h*100;
  PString(hStr, sizeof(hStr), y);
  Serial.println(y);
  Serial.println(hStr);
  udp.write((const uint8_t *)hStr, sizeof(hStr));
  udp.endPacket();
  memset(buf, 0, 255);
  //processing incoming packet, must be called before reading the buffer
  udp.parsePacket();
  //receive response from server, it will be HELLO WORLD
  if(udp.read(buf, 255) > 0){
    Serial.print("Server to client: ");
    Serial.println((char *)buffer);
  }
  //Wait for 1 second
  delay(1000);
}
