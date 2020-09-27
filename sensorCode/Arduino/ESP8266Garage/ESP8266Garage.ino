#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

WiFiClient espClient;
PubSubClient client(espClient);

#define RELAY1PIN 12
#define MQTT_SERVER "192.168.1.22"
const char* COMMAND_TOPIC = "/home/sensors/multi11/door1";
const char* STATE_TOPIC = "/home/sensors/multi11/door1/state";
boolean doorOpen = false;

void setup() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("station", "password");
    
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  
  pinMode(RELAY1PIN, OUTPUT);
  digitalWrite(RELAY1PIN, HIGH);
  Serial.begin(19200);
}

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length]='\0';
  Serial.println((char *)payload);
  Serial.println(topic);
  if( strcmp(topic, COMMAND_TOPIC) == 0 ) {
    if( (strcmp((char *)payload, "OPEN") == 0) ||
        (strcmp((char *)payload, "CLOSE") == 0)) { 
      digitalWrite(12, LOW);
      delay(500);
      digitalWrite(12, HIGH);
      doorOpen = !doorOpen;
      client.publish(STATE_TOPIC,(strcmp((char*)payload, "OPEN"))==0?"OPEN":"CLOSED");
    }
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    if (client.connect("multi11")) {
      client.subscribe(COMMAND_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
