

#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Defines usernames and passwords.
// see personalInfo.h.sample
#include "personalInfo.h"

#include <SHT1x.h>

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  0
#define clockPin 2
SHT1x sht1x(dataPin, clockPin);

WiFiClient wifiClient;
PubSubClient client((char *)server, 11797, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived

  Serial.print("Received some data topic:");
  Serial.print(topic);
  Serial.print(" payload");
  Serial.print((char *)payload);
  Serial.print(" \n");

}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void publish_number(const char *topic, const char *key, float num)
{
  // {"key1":value1}
  String payload = "{\"";
  payload += key;
  payload += "\":";
  payload += num;
  payload += "}";

  Serial.print("Sending payload: ");
  Serial.println(payload);

  if (!client.publish((char *)topic, (char*) payload.c_str())) {
    Serial.println("Publish failed");
  }
}

void setup() {
  Serial.begin(78400);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Generate client name based on MAC address and last 8 bits of microsecond counter
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);

  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);

  if (client.connect((char*) clientName.c_str(), (char *)mqtt_user, (char *)mqtt_password)) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Topic is: ");
    Serial.println(topic);

    if (client.publish((char *)topic, "hello from ESP8266")) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }

    if (client.subscribe("inTopic")) {
      Serial.println("Subscribe ok");
    }
    else
    {
      Serial.println("Subscribe failed");
    }

  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }
}

void loop() {
  static int counter = 0;

  float temp_c;
  float temp_f;
  float humidity;

  // Read values from the sensor
  temp_f = sht1x.readTemperatureF();
  humidity = sht1x.readHumidity();

  if (client.connected()) {
    
    publish_number(topic, "counter", counter);
    
    publish_number(topic, "temp", temp_f);
    
    publish_number(topic, "humidity", humidity);
  }
  else
  {
    Serial.println("Client is no longer connected, restarting...");
    abort();
  }

  ++counter;
  Serial.println("----");
  delay(5000);

  client.loop();
}


