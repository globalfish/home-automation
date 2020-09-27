void pir_setup(){
  pinMode(PIR1,INPUT);
}

/*
void pir_detect(){
  int val=0,pirState=LOW;
  val = digitalRead(PIR1);
  if (val == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
}
*/

int pir_detect(){
 int pirStat=0;
 pirStat = digitalRead(PIR1); 
 if (pirStat == HIGH) {            // if motion detected
   return 1;
 } 
 else {
   return 0;
 }
}

