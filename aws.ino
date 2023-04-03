#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
 
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
 
void connectAWS()
{ 
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
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
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["status"] = 0;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(String &topic, String &payload)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const int message = doc["status"];
  Serial.println("status: ");
  Serial.println(message);
}

void setup()
{
  Serial.begin(115200);
  connectAWS();
  // dht.begin();
}
 
void loop()
{
  // h = dht.readHumidity();
  // t = dht.readTemperature();
  status = 0;

  if (isnan(status)) {
    Serial.println(F("Failed to read status!"));
    return;
  }

  Serial.print(F("Status: "));
  Serial.println(status);
 
  // if (isnan(h) || isnan(t) )  // Check if any reads failed and exit early (to try again).
  // {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }
 
  // Serial.print(F("Humidity: "));
  // Serial.print(h);
  // Serial.print(F("%  Temperature: "));
  // Serial.print(t);
  // Serial.println(F("°C "));

  // sends and receives packets 
  client.loop();
 
  publishMessage();
  
  delay(1000);
}
