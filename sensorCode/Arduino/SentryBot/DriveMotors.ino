// Motor driver L298N from onlineTPS - 
// Layout - Pins 1-5 bottom row, 10-6 top row (left to right)
// For Anti-Clockwise rotation (Both motors positive voltage)
// Pin 1 - Enable 1 
// Pin 2 - Input 1 - HIGH
// Pin 3 - Input 2 - LOW
// Pin 4 - NC
// Pin 5 - GND
// Pin 6 - Enable 2
// Pin 7 - Input 3 - HIGH
// Pin 8 - Input 4 - LOW
// Pin 9 - NC
// Pin 10 - VCC


void setupDriveMotors(){
  pinMode(M1_P,OUTPUT);
  pinMode(M1_N,OUTPUT);
  pinMode(M2_P,OUTPUT);
  pinMode(M2_N,OUTPUT);

  stopMotors();
}

void moveBackward(int numSteps){
  for( int i = 0; i < numSteps; i++ ) {
    digitalWrite(M1_P,HIGH);
    digitalWrite(M1_N,LOW);
    digitalWrite(M2_P,LOW);
    digitalWrite(M2_N,HIGH);
  }
}


void moveForward(int numSteps){
  for( int i = 0; i < numSteps; i++ ) {
    digitalWrite(M1_P,LOW);
    digitalWrite(M1_N,HIGH);
    digitalWrite(M2_P,HIGH);
    digitalWrite(M2_N,LOW);
  }
}


void stopMotors(){
  digitalWrite(M1_P,LOW);
  digitalWrite(M1_N,LOW);
  digitalWrite(M2_P,LOW);
  digitalWrite(M2_N,LOW);
}


void turnRight(int numSteps){
  for( int i = 0; i < numSteps; i++ ) {
    digitalWrite(M1_P,LOW);
    digitalWrite(M1_N,HIGH);
    digitalWrite(M2_P,LOW);
    digitalWrite(M2_N,HIGH);
    delay(1);
  }
}

void turnLeft(int numSteps){
  for( int i = 0; i < numSteps; i++ ) {
    digitalWrite(M1_P,HIGH);
    digitalWrite(M1_N,LOW);
    digitalWrite(M2_P,HIGH);
    digitalWrite(M2_N,LOW);
    delay(1);
  }
}
