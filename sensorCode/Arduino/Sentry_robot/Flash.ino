void flash_setup(){
  pinMode(FLASH,OUTPUT);
}

void flash(){
  for(int i =0; i<5; i++){
    digitalWrite(FLASH,HIGH);
    delay(100);
    digitalWrite(FLASH,LOW);
    delay(100);
  }
}

