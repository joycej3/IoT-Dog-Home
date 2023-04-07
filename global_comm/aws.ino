#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// #include "DHT.h"
// #define DHTPIN 14     // Digital pin connected to the DHT sensor
// #define DHTTYPE DHT11   // DHT 11
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
 
float h;
float t;
int status;
 
// DHT dht(DHTPIN, DHTTYPE);
 
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(512);

WiFiServer server(8080);

void connectWifi() {
  // WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected to Wi-Fi with IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}
 
void connectAWS()
{ 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  Serial.println("AWS IoT Connected!");

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  // Create a message handler
  client.onMessage(messageHandler);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(THINGNAME)) {
      Serial.println("Connected to AWS IoT Core");
    }
  }
}
 
void publishMessage(String data)
{

  Serial.print("Publish MQTT Connection: ");
  Serial.println(client.connected());

  if (!client.connected()) {
    reconnect();
  }

  StaticJsonDocument<200> doc;
  doc["status"] = 0;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void publishJsonMessage(String json_data)
{

  Serial.print("Publish MQTT Connection: ");
  Serial.println(client.connected());

  if (!client.connected()) {
    reconnect();
  }

  // Parse the received JSON data
  StaticJsonDocument<200> receivedDoc;
  DeserializationError error = deserializeJson(receivedDoc, json_data);
  
  if (error) {
    Serial.print("Error parsing received JSON: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Extract the temperature and humidity from the received JSON data
  int door = receivedDoc["door"];

  // Create a new JSON document to send to AWS IoT Core
  StaticJsonDocument<200> publishDoc;
  publishDoc["door"] = door;
  
  char jsonBuffer[512];
  serializeJson(publishDoc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(String &topic, String &payload)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const int message = doc["door"];
  Serial.print("door: ");
  Serial.println(message);
}

void setup()
{
  Serial.begin(115200);
  connectWifi();
  connectAWS();
}
 
void loop()
{
  // sends and receives packets 
  client.loop();

  WiFiClient wclient = server.available();
  if (wclient) {
    Serial.println("Client connected");
    while (wclient.connected()) {
      if (wclient.available()) {
        String data = wclient.readStringUntil('\n');
        Serial.print("Received data: ");
        Serial.println(data);
      
        publishMessage(data);

        wclient.stop(); // Stop the client after receiving data
        Serial.println("Client disconnected");
        break; // Exit the inner loop
      }
    }
  }
  // Maintain the MQTT connection when there's no active client connection
  else {
    client.loop();
  }
}
