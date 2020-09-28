#include <Stepper.h>

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 2, 3, 4, 5);

void setup() {

  // set the speed at 60 rpm:
  myStepper.setSpeed(15);
  
  Serial.begin(9600);
}

long lastSound = micros();
boolean heardSound1 = false;
boolean heardSound2 = false;
long sound1time = 0;
long sound2time = 0;
int firstSound = 0;

void loop() {

  if( (micros() - lastSound) > 1000 ) {
    heardSound1 = false;
    heardSound2 = false;
    firstSound = 0;
  }
  if( !heardSound1 && !heardSound2) {

    if( digitalRead(9) == 0 ) {

      heardSound1 = true;
      sound1time = micros();
      firstSound = 1;
      lastSound = sound1time;
    }
    if( digitalRead(7) == 0 ) {

      heardSound1 = true;
      sound1time= micros();
      firstSound = 2;
      lastSound = sound1time;
    }
  }

  if( heardSound1 && !heardSound2) {

    if( firstSound == 2 && digitalRead(9) == 0 ) {

      heardSound2 = true;
      sound2time = micros();
    }
    if( firstSound == 1 && digitalRead(7) == 0 ) {

      heardSound2 = true;
      sound2time = micros();
    }
  }

  if( heardSound1 && heardSound2) {

    int delta = abs(sound1time - sound2time);
    int direction = 0;
    int steps = 0;
    int maxDelta = 1000;
    Serial.print(delta);
    if (firstSound == 1) {
      Serial.print(", RIGHT, ");
      direction = -1;
    }
    else {
      Serial.print(", LEFT, ");
      direction = 1;
    }
    if (delta < maxDelta/10) {
      Serial.println(" 0-10 degrees");
      steps = 100;
    }
    if (delta >= maxDelta/10 &&  delta < maxDelta/5) {
      Serial.println(" 10-30 degrees");
      steps = 200;
    }
    if (delta >= maxDelta/5 && delta < maxDelta/3) {
      Serial.println(" 30-60 degrees");
      steps = 300;
    }
    if (delta >= maxDelta/3) { 
      Serial.println(" 60-90 degrees");
      steps = 400;
    }
    firstSound = 0;
    heardSound1 = false;
    heardSound2 = false;
    myStepper.step(steps * direction);
    delay(1000);
    myStepper.step(steps * direction * (-1));
  }
}
