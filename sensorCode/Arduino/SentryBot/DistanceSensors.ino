
void setupDistanceSensors(){
  pinMode(FRONT_TRIG,OUTPUT);
  pinMode(FRONT_ECHO,INPUT); 
  pinMode(LEFT_TRIG,OUTPUT);
  pinMode(LEFT_ECHO,INPUT); 
  pinMode(RIGHT_TRIG,OUTPUT);
  pinMode(RIGHT_ECHO,INPUT); 
}

void readDistances(int & leftDistance, int & frontDistance, int & rightDistance) {

  leftDistance = leftSonar.convert_in(leftSonar.ping_median());
  frontDistance = frontSonar.ping_in();
  rightDistance = rightSonar.convert_in(rightSonar.ping_median());
}
