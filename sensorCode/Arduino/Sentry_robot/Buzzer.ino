void buzzer_setup(){
  pinMode(BUZZER,OUTPUT);
}

void buzz(){
  tone(BUZZER,400);
  delay(200);
  noTone(BUZZER);
  delay(100);
}

