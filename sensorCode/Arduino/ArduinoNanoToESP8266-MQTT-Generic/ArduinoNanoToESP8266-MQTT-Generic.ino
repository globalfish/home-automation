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
// 2020-08-09: modified to handle multiple sensors and send data out through serial as a JSON doc

#include <AltSoftSerial.h>
#include <ArduinoJson.h>
#include <stdlib.h>

//
// definitions of sensors and their pins; uncoment ones being used
//

//
// Serial port. If using AltSoftSerial these are not optional but hardcoded values
#define SERIAL1TX 9
#define SERIAL1RX 8                                                  

#define PIR1   // HC-SR501 or similar 3-pin sensor
#define PIR1DATAPIN 6

//#define PIR2   // HC-SR501 or similar 3-pin sensor
//#define PIR2DATAPIN 5

#define SOUND1 // Arduino sound sensor
#define SOUND1DATAPIN 5

#define DHT1 // DHT11 temperature and humidity sensor
#define DHTDATAPIN 4

#define LDR1  // light sensor
#define LDR1DATAPIN A0

//#define DISTANCE1
//#define DISTANCE1DATAPIN 9
//#define DISTANCE1TRIGPIN 10

//#define DISTANCE2
//#define DISTANCE2DATAPIN 11
//#define DISTANCE2TRIGPIN 12

//#define TEMP1W // one wire temp sensor
//#define TEMP1DATAPIN 8


// Using this over SoftwareSerial since supposedly better. Hardcoded to use pins 8,9
AltSoftSerial ser;

// Setup DHT11
#ifdef DHT1
#define DHTTYPE  DHT11   
#include <Adafruit_Sensor.h>
#include <DHT.h>
DHT dht(DHTDATAPIN, DHTTYPE);
#endif

// Setup a oneWire instance to communicate with Maxim/Dallas temperature ICs
#ifdef TEMP1W
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(TEMP1DATAPIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
#endif

const int slowPollingFrequency = 5000; // poll every <this> milliseconds for slower sensors like DHT11, PIR, etc.
const int fastPollingFrequency = 500; // poll every <this> milliseconds for faster sensors like motion, sound, light

// initialize sensor timers
long currentTime = millis();
long lastPIRTime = currentTime;
long lastSound1Time = currentTime;
long lastDHTTime = currentTime;
long lastLDRTime = currentTime;
long lastDistanceTime = currentTime;

// initialize sensor values
int lastPIR1 = 0;
int lastPIR2 = 0;
int lastSound1 = 0;
float lastDHTHumidity = 0.0;
float lastDHTTemperature = 0.0;
int lastLDR1 = 0;
float lastDistance1 = 0.0;
float lastDistance2 = 0.0;
int lastTemp1W = 0.0;

// sensor thresholds; update sensor values only if change > threshold
int tempThreshold = 1; // degree change
int humidityThreshold = 1; // humidity %
int lightThreshold = 50; // LDR signal level (0-1024)
int distanceThreshold = 0.5; // inches

// this runs once
void setup() {

  // enable debug serial
  Serial.begin(9600);
  // enable software serial
  ser.begin(19200);
  
#ifdef PIR1
  pinMode(PIR1DATAPIN, INPUT);
#endif

#ifdef PIR2
  pinMode(PIR2DATAPIN, INPUT);
#endif

#ifdef SOUND1
  pinMode(SOUND1DATAPIN, INPUT);
#endif

#ifdef DHT1
  pinMode(DHTDATAPIN, INPUT);
  dht.begin();
#endif

#ifdef DISTANCE1
  pinMode(DISTANCE1DATAPIN, INPUT);
  pinMode(DISTANCE1TRIGPIN, OUTPUT);
#endif

#ifdef DISTANCE2
  pinMode(DISTANCE2DATAPIN, INPUT);
  pinMode(DISTANCE2TRIGPIN, OUTPUT);
#endif

#ifdef TEMP1W
  // Initialize OneWire library. if we are using DB18B20 type of sensors
  sensors.begin();
#endif
}

void loop() {
  //
  // emit events whenever there is a change, or as per polling frequency
  //

  boolean valueChanged = false;
  
#ifdef PIR1
  // get proximity reading from PIR1
  int currPIR = digitalRead(PIR1DATAPIN);

  // change in PIR output
  if ( (millis() - lastPIRTime > slowPollingFrequency) && currPIR != lastPIR1 ) { // when state changes

    lastPIRTime = millis();
    lastPIR1= currPIR;
    valueChanged = true;
  }
#endif


#ifdef PIR2
  // get proximity reading from PIR2
  int currPIR2 = digitalRead(PIR2DATAPIN);

  // change in PIR output
  if ( (millis() - lastPIRTime > slowPollingFrequency) && currPIR2 != lastPIR2 ) { // when state changes

    lastPIRTime = millis();
    lastPIR2 = currPIR2;
    valueChanged = true;
  }
#endif


#ifdef SOUND1
  int currSound1 = digitalRead(SOUND1DATAPIN);

  // change in sound level/trigger
  if( (millis() - lastSound1Time > slowPollingFrequency) && currSound1 != lastSound1  ) {

    lastSound1Time = millis();
    lastSound1 = currSound1;
    valueChanged = true;
  }
#endif


#ifdef DHT1 // read temp and humidity from DHT11
  int currHumidity = dht.readHumidity();
  int currTemp2 = (dht.readTemperature(true)); // read in F
  // timer triggered OR significant change in temperature
  if ( (millis() - lastDHTTime > slowPollingFrequency) &&
       ((abs(currTemp2 - lastDHTTemperature) > tempThreshold) ||
       (abs(currHumidity - lastDHTHumidity) > humidityThreshold))) { 

    // update timers and value
    lastDHTTime = millis();
    lastDHTTemperature = currTemp2;
    lastDHTHumidity = currHumidity;
    valueChanged = true;
  }
#endif


#ifdef LDR1 // read light level
  // get light level from LDR
  int currLight1 = readLightLevel(LDR1DATAPIN);

  // timer triggered OR significant change in light intensity
  if (( (millis() - lastLDRTime > slowPollingFrequency) &&
      (abs( currLight1 - lastLDR1) > lightThreshold))) { 
 
    // update timers and value
    lastLDRTime = millis();
    lastLDR1 = currLight1;
    valueChanged = true;
  }
#endif


#ifdef DISTANCE1
  // get distance reading from PING sensor
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(DISTANCE1TRIGPIN, OUTPUT);
  digitalWrite(DISTANCE1TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(DISTANCE1TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(DISTANCE1TRIGPIN, LOW);
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(DISTANCE1DATAPIN, INPUT);
  long duration = pulseIn(DISTANCE1DATAPIN, HIGH);
  // convert the time into a distance
  float currDistance = microsecondsToInches(duration);

  // significant change in distance
  if((millis() - lastDistanceTime > fastPollingFrequency) &&
     (abs(lastDistance1 - currDistance) > distanceThreshold)){
    lastDistanceTime = millis();
    lastDistance1 = currDistance;
    valueChanged = true;
  }
#endif


#ifdef DISTANCE2
  // get distance reading from PING sensor
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(DISTANCE2TRIGPIN, OUTPUT);
  digitalWrite(DISTANCE2TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(DISTANCE2TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(DISTANCE2TRIGPIN, LOW);
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(DISTANCE2DATAPIN, INPUT);
  long duration2 = pulseIn(DISTANCE2DATAPIN, HIGH);
  // convert the time into a distance
  float currDistance2 = microsecondsToInches(duration2);

  // significant change in distance
  if((millis() - lastDistanceTime > fastPollingFrequency) &&
     (abs(lastDistance2 - currDistance2) > distanceThreshold)){
    lastDistanceTime = millis();
    lastDistance2 = currDistanc2;
    valueChanged = true;
  }
#endif

#ifdef TEMP1W  // read temperature
  // get temperature from DS18B20
  sensors.requestTemperatures(); // Send the command to get temperatures
  int currTemp1W = (sensors.getTempCByIndex(0) * 9 / 5 ) + 32; // C to F

  // significant change in temperature
  if (( (millis() - lastTempTime > slowPollingFrequency) &&
      (abs(currTemp1W - lastTemp1W) > tempThreshold))) { 

    // update timers and value
    lastTempTime = millis();
    lastTemp1W = currTemp1W;
    valueChanged = true;
  }
#endif

  if( valueChanged) {
    DynamicJsonDocument jdoc(256);
    
  #ifdef PIR1
    jdoc["motion1"] = lastPIR1;
  #endif
  
  #ifdef PIR2 
    jdoc["motion2"] = lastPIR2;
  #endif
  
  #ifdef SOUND1
    jdoc["sound1"] = lastSound1;
  #endif
  
  #ifdef DHT1
    jdoc["temperature"] = lastDHTTemperature;
    jdoc["humidity"] = lastDHTHumidity;
  #endif
  
  #ifdef LDR1
    jdoc["light1"] = lastLDR1;
  #endif
  
  #ifdef DISTANCE1
    jdoc["distance1"] = lastDistance1;
  #endif
  
  #ifdef DISTANCE2
    jdoc["distance2"] = lastDistance2;
  #endif
  
  #ifdef TEMP1W
    jdoc["temperature2"] = lastTemp1W;      
  #endif
  
    serializeJson(jdoc, Serial);
    serializeJson(jdoc, ser);
  }
}

float readLightLevel(int analogPin) {
  // read the value from LDR.
  // read 10 values for averaging.
  int val = 0, count = 10;
  for (int i = 0; i < count; i++) val += analogRead(analogPin);
  val = val/count;
  return val;
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74.0 / 2.0;
}

void sendDataOverSerial(char *stringBuf) {

  //Serial.println(stringBuf);
  ser.print(stringBuf);
  Serial.println(stringBuf); // debug to serial monitor if connected
  ser.flush();
  delay(500);
}
