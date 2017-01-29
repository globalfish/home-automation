/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "ssid";
const char* password = "password";
const char* mqtt_server = "mqtt_server";
char* mgmtTopic = "SensorManagement";
char* sensorDataTopic = "SensorData";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  processAction((char*) payload, length);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client88")) {
      //tempMessage = "Connected to network with IP " + WiFi.localIP().toString();
      char tempMessage[80] = "";
      strcat(tempMessage, "Connected to network with IP ");
      char myIPString2[24] = "";
      IPAddress myIP = WiFi.localIP();
      sprintf(myIPString2, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
      strcat(tempMessage, myIPString2);
      client.publish(mgmtTopic, tempMessage);
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
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  char message[255] = "";
  int indx = 0;
  while (Serial.available() && indx < 255) {
    delay(1);
    if (Serial.available() > 0) {
      char c = Serial.read();
      message[indx] = c;
      indx++;
    }
    message[indx] = '\0';
  }
  if ( strstr(message, "INFO") != NULL) {
    char heartBeatMsg[335] = ""; // since message can be max of 255 and ack is max of 80
    strcat(heartBeatMsg, message);
    strcat(heartBeatMsg, " IP = ");
    char myIPString[24] = "";
    IPAddress myIP = WiFi.localIP();
    sprintf(myIPString, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
    strcat(heartBeatMsg, myIPString);
    strcat(heartBeatMsg, "\0");
    client.publish(mgmtTopic, heartBeatMsg);
  }
  else {
    client.publish(sensorDataTopic, message);
  }
}
//
// process actions that are sent to the ESP8266 either from MQTT
// or (eventually) from the Arduino
//

void processAction(char *actionString, int length) {

  char *actionKeyword = "ACTION"; // forward to Arduino
  if ( strstr(actionString, actionKeyword) != NULL)
    for (int i = 0; i < length; i++) {
      Serial.print(actionString[i]); // sends to Arduino over serial port
    }
  Serial.flush();
}
