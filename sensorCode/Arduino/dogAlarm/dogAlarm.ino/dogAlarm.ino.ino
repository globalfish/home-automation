/*
  Ping))) Sensor

  This sketch reads a PING))) ultrasonic rangefinder and returns the distance
  to the closest object in range. To do this, it sends a pulse to the sensor to
  initiate a reading, then listens for a pulse to return. The length of the
  returning pulse is proportional to the distance of the object from the sensor.

  The circuit:
	- +V connection of the PING))) attached to +5V
	- GND connection of the PING))) attached to ground
	- SIG connection of the PING))) attached to digital pin 7

  created 3 Nov 2008
  by David A. Mellis
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Ping
*/

// this constant won't change. It's the pin number of the sensor's output:
const int trig1Pin = 3;
const int echo1Pin = 4;
const int trig2Pin = 5;
const int echo2Pin = 6;

int BUZZERPIN = 2;

// set up buzzer frequencies and durations
int freq[] = {
  2000, 4000, 5000, 6000, 8000, 8000, 2000, 500
};

int duration[] = {
  200, 200, 200, 200, 200, 200, 200, 200
};

void setup() {

}
// put your setup code here, to run once:

void loop() {

  //
  //  Serial.print("LEFT (in.) ");
  //  Serial.print(inches1);
  //
  //  Serial.print("   RIGHT (in.) ");
  //  Serial.print(inches2);
  //
  //  Serial.println();

  delay(100);
}

long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are 73.746
  // microseconds per inch (i.e. sound travels at 1130 feet per second).
  // This gives the distance travelled by the ping, outbound and return,
  // so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}

void makeNoise() {
  for ( int i = 0; i < 8; i++) {
    tone(BUZZERPIN, freq[i], duration[i]);
    delay(duration[i] * 0.05);
  }
}

void checkForPresennce() {
  // establish variables for duration of the ping, and the distance result
  // in inches and centimeters:
  long duration1, inches1, duration2, inches2, inches;

  digitalWrite(trig1Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig1Pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trig1Pin, LOW);
  duration1 = pulseIn(echo1Pin, HIGH);

  digitalWrite(trig2Pin, LOW);
  digitalWrite(trig2Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig2Pin, HIGH);
  delayMicroseconds(5);
  duration2 = pulseIn(echo2Pin, HIGH);

  // convert the time into a distance
  inches1 = microsecondsToInches(duration1);
  inches2 = microsecondsToInches(duration2);

  inches = min(inches1, inches2);

  if (inches < 6 && inches > 0) {
    makeNoise();
  }
}

