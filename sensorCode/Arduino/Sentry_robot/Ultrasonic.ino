
void ultrasonic_setup(){
  pinMode(U_TRIG,OUTPUT);
  pinMode(U_ECHO,INPUT); 
  pinMode(ULevel_ECHO,INPUT); 
  for(int i=0;i<50;i++){
    ground_level += measure_level();
    delay(20);
  }
  ground_level = ground_level/50.0;
}

int measure_distance_level(long forward_distance, long height){
  long duration;
  float level;
  try_again:
  duration = measure_distance();
  level = measure_level();
  if(level==0 || duration ==0)
    goto try_again;
  Serial.print("Distance = ");
  Serial.print(duration);
  Serial.print("    Level = ");
  Serial.println(level);
  if(duration>forward_distance && (abs(height-level)< 15) ){
    return 1;
  }
  return 0;
  
}


long measure_distance(){
  long duration;
  digitalWrite(U_TRIG,LOW);
  delayMicroseconds(2);
  digitalWrite(U_TRIG,HIGH);
  delayMicroseconds(10);
  duration = pulseIn(U_ECHO,HIGH);
  return microsecondsToCentimeters(duration);
  
}

float measure_level(){
  long level;
  digitalWrite(U_TRIG,LOW);
  delayMicroseconds(2);
  digitalWrite(U_TRIG,HIGH);
  delayMicroseconds(10);
  level = pulseIn(ULevel_ECHO,HIGH);
  return microsecondsToMillimeters(level);
  
}

long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}

float microsecondsToMillimeters(long microseconds) {
   return microseconds / (2.9*2.0);
}
