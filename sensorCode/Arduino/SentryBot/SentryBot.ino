// 2020-05-01: original code by Aditya
// 2020-08-09: modified to streamline

// Libraries for the OLED display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <NewPing.h>
#include "SentryBotConfig.h"

void setup() {
  //Serial.begin(9600);
  setupDriveMotors();
  setupDistanceSensors();
  OLEDSetup();
}

void loop() {

  int leftClearance, frontClearance, rightClearance;
  readDistances(leftClearance, frontClearance, rightClearance);
  displayDistances(leftClearance, frontClearance, rightClearance);

  
  if(  frontClearance > MIN_CLEARANCE ) { // move forward if we can
  // scan sides
    if( rightClearance < 2*MIN_CLEARANCE ) { // obstacle on right, swerve left
      turnLeft(250);
    }
    if( leftClearance < 2*MIN_CLEARANCE ) { // obstacle on left, swerve right
      turnRight(250);
    }
    moveForward(10); // move forward
    updateDisplayMotion("FRN");
  }
  else { // obstacle ahead, scan both sides
    stopMotors();
    //delay(20);
    if( rightClearance > MIN_CLEARANCE ) { // turn right if you can
      turnRight(500);
      updateDisplayMotion("RYT");
    }
    else if ( leftClearance > MIN_CLEARANCE ) { // turn left if you can
      turnLeft(500);
      updateDisplayMotion("LFT");
    }
    else { // no path ahead so reverse
      moveBackward(50);
      updateDisplayMotion("REV");
    }
  }
}
