void ldr_setup(){
  pinMode(LDR,INPUT);
}

// 1000+ fully bright
// 600- fully dark
// use 900 cut off 

int light_measure(){      
  return analogRead(LDR);
}

