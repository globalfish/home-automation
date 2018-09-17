// timer code from http://www.gammon.com.au/power
// buzzer code from Arduino examples
// ultrasonic ping code from Arduino examples

// my original contribution is assigning pin numbers and learning that
// when using 2 ping sensors you wanted to finish reading one and then 
// triggering the other, and not try to trigger and read them in parallel.

#include <avr/sleep.h>
#include <avr/wdt.h>


// this constant won't change. It's the pin number of the sensor's output:
const int trig1Pin = 3;http://www.gammon.com.au/power
const int echo1Pin = 4;
const int trig2Pin = 5;
const int echo2Pin = 6;
const int BUZZERPIN = 2;
const byte LED = 9;

// set up buzzer frequencies and durations
int freq[] = {
  2000, 4000, 5000, 6000, 8000, 8000, 2000, 500
};

int duration[] = {
  200, 200, 200, 200, 200, 200, 200, 200
};

void flash ()
{
  pinMode (LED, OUTPUT);
  for (byte i = 0; i < 1; i++)
  {
    digitalWrite (LED, HIGH);
    delay (50);
    digitalWrite (LED, LOW);
    delay (50);
  }

  pinMode (LED, INPUT);

}  // end of flash

// watchdog interrupt
ISR (WDT_vect)
{
  wdt_disable();  // disable watchdog

}  // end of WDT_vect

void setup () {

  // initialize serial communication:
  Serial.begin(9600);
  pinMode(trig1Pin, OUTPUT);
  pinMode(echo1Pin, INPUT);
  pinMode(trig2Pin, OUTPUT);
  pinMode(echo2Pin, INPUT);
}

void loop ()
{

  flash ();
  checkForPresence();
  delay(100);
  
  // disable ADC
  ADCSRA = 0;

  // clear various "reset" flags
  MCUSR = 0;
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval
  //WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1) | bit (WDP0);    // set WDIE, and 4 seconds delay
  wdt_reset();  // pat the dog

  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  noInterrupts ();           // timed sequence follows
  sleep_enable();

  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS);
  interrupts ();             // guarantees next instruction executed

  sleep_cpu ();

  // cancel sleep as a precaution
  sleep_disable();

} // end of loop

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

void checkForPresence() {
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
