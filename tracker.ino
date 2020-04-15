/*****************************************
* ESP32 GPS VKEL 9600 Bds
******************************************/

#include <TinyGPS++.h>                       
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Char O'Pinho the Mage";
const char* password =  "raapaaiz";
const char* mqttServer = "192.168.0.16";
const int mqttPort = 1883;
const char* channelName = "tracker1/gps";

WiFiClient MQTTclient;
PubSubClient client(MQTTclient);
unsigned long lastMsg = 0;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

TinyGPSPlus gps;                            
HardwareSerial SmartGPS(1);                 
void setup()
{
  Serial.begin(115200);
  SmartGPS.begin(9600, SERIAL_8N1, 12, 15);   //17-TX 18-RX
  Serial.println("Attempting to connect...");
  WiFi.begin(ssid, password); // Connect to WiFi.
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Couldn't connect to WiFi.");
      while(1) delay(100);
  }
  client.setServer(mqttServer, 1883); // Connect to PubNub.
}


void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (client.connected()) {

    delay(1000);
      
    Serial.print("Latitude  : ");
    Serial.println(gps.location.lat(), 5);
    Serial.print("Longitude : ");
    Serial.println(gps.location.lng(), 4);
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    Serial.print("Altitude  : ");
    Serial.print(gps.altitude.feet() / 3.2808);
    Serial.println("M");
    Serial.print("Time      : ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.println(gps.time.second());
    Serial.println("**********************");
  
    //client.publish(channelName,"{\"location\": {\"lat\": -41.326530612244895, \"lon\": -57.9591836734694}, \"t\": \"15/04/20 01:40\"}"); // Publish message.
    String lat = String(gps.location.lat(), 6);
    String lng = String(gps.location.lng(), 6);
    String t =  String(gps.time.hour()) + "-" + String(gps.time.minute()) + "-" +String(gps.time.second());
    String date = String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year());
    String pos = lat + ";" + lng + ";" + t + ";" + date;
    client.publish(channelName, pos.c_str()); // Publish message.

    smartDelay(2000);                                      
  
    if (millis() > 5000 && gps.charsProcessed() < 10)
      Serial.println(F("No GPS data received: check wiring"));
    }
     Serial.println("DDDDD  : ");

}

static void smartDelay(unsigned long ms)                
{
  unsigned long start = millis();
  do
  {
    while (SmartGPS.available())
      Serial.write(byte(SmartGPS.read()));
      gps.encode(SmartGPS.read());

  } while (millis() - start < ms);
}
