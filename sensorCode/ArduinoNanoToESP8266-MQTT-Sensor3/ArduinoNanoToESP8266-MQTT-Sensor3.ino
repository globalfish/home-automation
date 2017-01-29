// esp8266_test.ino
//
// Original:
// Plot LM35 data on thingspeak.com using an Arduino and an ESP8266 WiFi
// module.
//
// Author: Mahesh Venkitachalam
// Website: electronut.in

// adapted for my use
// -Venkat Swaminathan. Dec 2016

#include <SoftwareSerial.h>
#include <stdlib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int SENSORID = 3;

// Define our pins
int ldrPin = A0;   // analog LDR
int ldr2Pin = A1; // second light sensor
int serialRX = 2;  // Serial port RX (to ESP8266); connect to TX on other side
int serialTX = 3;  // Serial port TX (to ESP8266); connect to RX on other side
int motionPin = 4; // motion sensor data
int trigPin = 5;   // trigger for HC-SR04 Ultrasonic sensor
int echoPin = 6;   // received echo for HC-SR04 Ultrasonic Sensor
int doorSensorPin = 7; // check when door is open
int buzzer = 8; // piezo buzzer
int motion2Pin = 9; // second motion sensor
int tempPin = 10;  // data wire of DS18B20 onewire temperature sensor
int ledPin = 12;   // heartbeat LED
int hiBriteLedPin = 13; // high intensity LED. Using default value due to how wiring is done

long timer1, timeInterval1; // timers to track stuff
long timer2, timeInterval2; // timers to track stuff
long lightSamplerTimer, temperatureSamplerTimer, motionSamplerTimer, distanceSamplerTimer;
long lightSampler2Timer, motionSampler2Timer;
long heartbeatTimer;
long distance = 0;
char jsonObj1[256];

float temperature, prevTemperature = 0.0, lightLevel, prevLightLevel1, prevLightLevel2;

int doorClosedState = 1; // state to track if door is open or not

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(tempPin);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
SoftwareSerial ser(serialRX, serialTX); // RX, TX

void blinkLED(int ledPin, boolean beep = false);

// this runs once
void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);

  // enable debug serial
  Serial.begin(9600);
  // enable software serial
  ser.begin(9600);
  // Start up the library
  sensors.begin();

  pinMode(doorSensorPin, INPUT_PULLUP);
  
  timer1 = 10000; // 10 seconds for light?
  timer2 = 30000; // milliseconds between measurements
  lightSamplerTimer = millis();
  lightSampler2Timer = millis();
  temperatureSamplerTimer = millis();
  motionSamplerTimer = millis();
  motionSampler2Timer = millis();
  distanceSamplerTimer = millis();

  blinkLED(ledPin, true);
}

// the loop
void loop() {

  //
  // general principle of event emitter is to emit events
  // on a timer, or if there is a large change in values
  //
  char temp[20] = ""; // use for conversions
  // get temperature from DS18B20
  sensors.requestTemperatures(); // Send the command to get temperatures
  temperature = sensors.getTempCByIndex(0);
  temperature = (temperature * 9 / 5) + 32;
  if ( millis() - temperatureSamplerTimer > timer2 || // timer triggered OR
       abs(temperature - prevTemperature) > 1 ) { // more than a degree change in temperature
    temp[0] = '\0';
    dtostrf(temperature, 7, 2, temp);
    jsonObj1[0] = '\0';
    constructJSONObj("temperature", temp, jsonObj1);
    sendDataOverSerial(jsonObj1);
    temperatureSamplerTimer = millis();
    prevTemperature = temperature;
  }

  // get light level from LDR
  lightLevel = readLightLevel(ldrPin);
  char temp1[20];
  dtostrf(lightLevel, 9, 2, temp1);
  if ( millis() - lightSamplerTimer > timer2  || // timer triggered, OR
       abs( lightLevel - prevLightLevel1) > 100) { // sudden change in light intensity
    jsonObj1[0] = '\0';
    constructJSONObj("light", temp1, jsonObj1);
    sendDataOverSerial(jsonObj1);
    lightSamplerTimer = millis();
    prevLightLevel1 = lightLevel;
  }


  // get light level from LDR2
  lightLevel = readLightLevel(ldr2Pin);
  char temp2[20];
  dtostrf(lightLevel, 9, 2, temp2);
  if ( millis() - lightSampler2Timer > timer2 ) {
    jsonObj1[0] = '\0';
    constructJSONObj("light2", temp2, jsonObj1);
    sendDataOverSerial(jsonObj1);
    lightSampler2Timer = millis();
  }

  // check if door is open
  // get door sensor open reading at any time
  int sensorVal = digitalRead(doorSensorPin);
  delay(1);
  if (  sensorVal != doorClosedState) { // only trigger this when state changes
    char temp3[3];
    sprintf(temp3, "%d", sensorVal);
    jsonObj1[0] = '\0';
    constructJSONObj("door", temp3, jsonObj1);
    sendDataOverSerial(jsonObj1);
    doorClosedState = sensorVal;
  }
  
  // get distance reading from PING sensor
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  long duration = pulseIn(echoPin, HIGH);
  // convert the time into a distance
  distance = microsecondsToInches(duration);


  // get proximity reading from PIR
  int motionLevel = digitalRead(motionPin);
  if ( (motionLevel == 1) || (millis() - motionSamplerTimer > timer2 )) {
    char temp2[3];
    sprintf(temp2, "%d", motionLevel);
    jsonObj1[0] = '\0';
    constructJSONObj("motion", temp2, jsonObj1);
    sendDataOverSerial(jsonObj1);
    motionSamplerTimer = millis();
  }


  // check for messages from ESP8266 that may have come in from MQTT
  char message[255]; // 255 chars should be sufficient for everyone ;-)
  boolean haveMessage = false;
  int indx = 0;
  if (ser.available()) {
    haveMessage = true;
    delay(1);
    while ( ser.available() && indx < 255 ) {
      char c = ser.read();
      message[indx] = c;
      indx++;
    }
    message[indx] = '\0'; // null terminate the string
  }

  if ( haveMessage) // if we do have a message then process it
    processAction(message);

  // heartbeat LED
  if ( millis() - heartbeatTimer > 5000 ) {
    blinkLED(ledPin);
    heartbeatTimer = millis();

    char infoMsg[50];
    infoMsg[0] = '\0';
    strcat(infoMsg, "INFO: SensorID ");
    temp[0] = '\0';
    sprintf(temp, "%d", SENSORID);
    strcat(infoMsg, temp);
    strcat(infoMsg, " is UP.");
    strcat(infoMsg, "\0");
    sendDataOverSerial(infoMsg);

  }
}

float readLightLevel(int analogPin) {
  // read the value from LDR.
  // read 10 values for averaging.
  int val = 0;
  val += analogRead(analogPin);

  return val;
}

void blinkLED(int pin, boolean beep) {

  // blink LED on board
  digitalWrite(pin, HIGH);
  if ( beep ) {
    tone( buzzer, 100 * pin, 100);
  }
  delay(200);
  digitalWrite(pin, LOW);
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

void sendDataOverSerial(char *stringBuf) {

  //Serial.println(stringBuf);
  ser.print(stringBuf);
  ser.flush();
  delay(100);
}

void constructJSONObj( const char *measurement, char *value, char *object) {
  char temp[20] = ""; // use for conversions
  object[0] = '\0'; // init string
  strcat(object, "{\"SensorId\":\"");
  sprintf(temp, "%d", SENSORID);
  strcat(object, temp);
  strcat(object, "\",\"Measurement\":\"");
  strcat(object, measurement);
  strcat(object, "\",\"Value\":\"");
  strcat(object, value);
  strcat(object,  "\" }");
  strcat(object, "\0");
}



//
// process actions that are sent to the ESP8266 either from MQTT
// or from the Arduino
//
// action strings are of the form: "ACTION,<sensorID>,<messageID>,<verb>,<object>,<value>" where
//   <sensorID> = which sensor this message is meant for
//   <messageID> = unique message ID possibly used for logging and ack
//   <verb> = "SETVAR" or "SETPIN"
//   <object> = one of "PIN","SENSORID","MQTTSRVR","SSID","PASSWORD"
//   <value> = a string that corresponds to <object>, e.g. pin number or String.
//
void processAction(char *actionString) {

  char temp[20]; // use for conversions
  char tempString[255];
  strcpy(tempString, actionString);
  char *token;
  char *savePtr;
  token = strtok_r(tempString, ",", &savePtr);
  //Serial.print("0<"); Serial.print(token); Serial.print(">");
  if ( token != NULL && strcmp(token, "ACTION") == 0) { // found ACTION keyword
    token = strtok_r(NULL, ",", &savePtr);
    //Serial.print("1<"); Serial.print(token); Serial.print(">");
    int sensorID = atoi(token); // which sensor ID is this message meant for
    if ( sensorID == SENSORID ) { // this is for me
      token = strtok_r(NULL, ",", &savePtr);
      //Serial.print("2<"); Serial.print(token); Serial.print(">");
      char messageID[10];
      strcpy(messageID, token);
      token = strtok_r(NULL, ",", &savePtr); // get verb
      //Serial.print("3<"); Serial.print(token); Serial.print(">");
      if ( token != NULL && strcmp(token, "SETPIN") == 0) { // looks like we want to set a pin value
        token = strtok_r(NULL, ",", &savePtr);
        //Serial.print("4<"); Serial.print(token); Serial.print(">");
        if ( token != NULL) {
          int pinID = atoi(token); // get which pin we want to modify
          token = strtok_r(NULL, ",", &savePtr);
          //Serial.print("5<");Serial.print(token);Serial.print(">");
          if ( token != NULL && strcmp(token, "HIGH") == 0) {
            digitalWrite(pinID, HIGH);
            // String outMessage = "INFO: Executed Action ID: " + messageID + ". ";
            char ackMsg[80];
            strcat(ackMsg, "INFO: Executed Action ID: ");
            strcat(ackMsg, messageID);
            strcat(ackMsg, "\0");
            //Serial.println(outMessage);
            sendDataOverSerial(ackMsg);
          }
          else if ( token != NULL && strcmp(token, "LOW") == 0) {
            digitalWrite(pinID, LOW);
            // String outMessage = "INFO: Executed Action ID: " + messageID + ". ";
            char ackMsg[80] = "";
            strcat(ackMsg, "INFO: Executed Action ID: ");
            strcat(ackMsg, messageID);
            strcat(ackMsg, "\0");
            //Serial.println(outMessage);
            sendDataOverSerial(ackMsg);
          }
        }
      }
    }
  }
}

void beepBuzzer() {

  tone( buzzer, 10000, 50);
}
