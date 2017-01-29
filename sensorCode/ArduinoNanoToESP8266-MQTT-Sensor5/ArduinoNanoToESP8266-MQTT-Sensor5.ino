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

const int SENSORID = 5;

// Define our pins
int ldrPin = A0;   // analog LDR
int ldr2Pin = A1; // second light sensor
int serialRX = 2;  // Serial port RX (to ESP8266); connect to TX on other side
int serialTX = 3;  // Serial port TX (to ESP8266); connect to RX on other side
int motionPin = 4; // motion sensor data
int relay1 = 5;   // relay1
int relay2 = 6;   // relay2
int relay3 = 7; // relay3
int buzzer = 8; // piezo buzzer
int doorSensor1Pin = 9; // check when door1 is open
int tempPin = 10;  // data wire of DS18B20 onewire temperature sensor
int doorSensor2Pin = 11; // check when door2 is open
int ledPin = 12;   // heartbeat LED
int hiBriteLedPin = 13; // high intensity LED. Using default value due to how wiring is done, also relay4

long timer1, timeInterval1; // timers to track stuff
long timer2, timeInterval2; // timers to track stuff
long lightSamplerTimer, temperatureSamplerTimer, motionSamplerTimer, distanceSamplerTimer;
long lightSampler2Timer, motionSampler2Timer, temperatureSampler2Timer;
long doorSampler1Timer, doorSampler2Timer;
long heartbeatTimer;
long distance = 0;
char jsonObj1[256];

float temperature1, temperature2, lightLevel;
float prevTemperature1 = 0.0, prevTemperature2 = 0.0, prevLightLevel = 0.0;

int prevDoor1ClosedState = 1; // state to track if door is open or not
int prevDoor2ClosedState = 1; // state to track if door is open or not
int door1ClosedState, door2ClosedState;

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
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);

  // enable debug serial
  Serial.begin(9600);
  // enable software serial
  ser.begin(9600);
  // Start up the library
  sensors.begin();

  pinMode(doorSensor1Pin, INPUT_PULLUP);
  pinMode(doorSensor2Pin, INPUT_PULLUP);

  timer1 = 10000; // 10 seconds for light?
  timer2 = 30000; // milliseconds between measurements
  lightSamplerTimer = millis();
  lightSampler2Timer = millis();
  temperatureSamplerTimer = millis();
  motionSamplerTimer = millis();
  motionSampler2Timer = millis();
  distanceSamplerTimer = millis();
  doorSampler1Timer = millis();
  doorSampler2Timer = millis();

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
  temperature1 = sensors.getTempCByIndex(0);
  temperature2 = sensors.getTempCByIndex(1);
  temperature1 = (temperature1 * 9 / 5) + 32;
  temperature2 = (temperature2 * 9 / 5) + 32;
  if ( millis() - temperatureSamplerTimer > timer2 || // timer triggered OR
       abs(temperature1 - prevTemperature1) > 1 ) { // more than a degree change in temperature
    temp[0] = '\0';
    dtostrf(temperature1, 7, 2, temp);
    jsonObj1[0] = '\0';
    constructJSONObj("temperature1", temp, jsonObj1);
    sendDataOverSerial(jsonObj1);
    temperatureSamplerTimer = millis();
    prevTemperature1 = temperature1;
  }

  if ( millis() - temperatureSampler2Timer > timer2 || // timer triggered OR
       abs(temperature2 - prevTemperature2) > 1 ) { // more than a degree change in temperature
    for ( int i = 0; i < 20; i++) temp[i] = '\0';
    dtostrf(temperature2, 7, 2, temp);
    jsonObj1[0] = '\0';
    constructJSONObj("temperature2", temp, jsonObj1);
    sendDataOverSerial(jsonObj1);
    temperatureSampler2Timer = millis();
    prevTemperature2 = temperature2;
  }


  // get light level from LDR2
  lightLevel = readLightLevel(ldr2Pin);
  char temp2[20];
  dtostrf(lightLevel, 9, 2, temp2);
  if ( millis() - lightSamplerTimer > timer2  || // timer triggered, OR
       abs( lightLevel - prevLightLevel) > 100) { // sudden change in light intensity
    jsonObj1[0] = '\0';
    constructJSONObj("light2", temp2, jsonObj1);
    sendDataOverSerial(jsonObj1);
    lightSamplerTimer = millis();
    prevLightLevel = lightLevel;
  }

  // check if door is open
  // get door sensor open reading at any time
  door1ClosedState = digitalRead(doorSensor1Pin);
  delay(1);
  if ( millis() - doorSampler1Timer > timer2 ||  // check door state every timer2 seconds, OR
       door1ClosedState != prevDoor1ClosedState) { //  when state changes
    char temp3[3];
    sprintf(temp3, "%d", door1ClosedState);
    jsonObj1[0] = '\0';
    constructJSONObj("door1", temp3, jsonObj1);
    sendDataOverSerial(jsonObj1);
    doorSampler1Timer = millis();
    if ( prevDoor1ClosedState != door1ClosedState)
      beepBuzzer();
    prevDoor1ClosedState = door1ClosedState;
  }


  // check if door is open
  // get door sensor open reading at any time
  door2ClosedState = digitalRead(doorSensor2Pin);
  delay(1);
  if ( millis() - doorSampler2Timer > timer2 ||  // check door state every timer2 seconds, OR
       door2ClosedState != prevDoor2ClosedState) { //  when state changes
    char temp3[3];
    sprintf(temp3, "%d", door2ClosedState);
    jsonObj1[0] = '\0';
    constructJSONObj("door2", temp3, jsonObj1);
    sendDataOverSerial(jsonObj1);
    doorSampler2Timer = millis();
    if ( prevDoor2ClosedState != door2ClosedState)
      beepBuzzer();
    prevDoor2ClosedState = door2ClosedState;
  }


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
            char ackMsg[80];
            strcat(ackMsg, "INFO: Executed Action ID: ");
            strcat(ackMsg, messageID);
            strcat(ackMsg, "\0");
            sendDataOverSerial(ackMsg);
          }
          else if ( token != NULL && strcmp(token, "LOW") == 0) {
            digitalWrite(pinID, LOW);
            char ackMsg[80] = "";
            strcat(ackMsg, "INFO: Executed Action ID: ");
            strcat(ackMsg, messageID);
            strcat(ackMsg, "\0");
            sendDataOverSerial(ackMsg);
          }
        }
      }
    }
  }
  //Serial.println();
}

void beepBuzzer() {

  tone( buzzer, 10000, 50);
}
