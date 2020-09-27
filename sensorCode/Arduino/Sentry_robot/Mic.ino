void mic_setup(){
  pinMode(MIC,INPUT);
}

int sound_measure(){
  int val=0,ntrials=200;
  for(int i =0; i<ntrials ; i++){
    val+=digitalRead(MIC);
    delayMicroseconds(2);
  }
  
  if(float(val)/float(ntrials) > 0.95)    // choose your confidence level and adjust microphone sensitivity also accordingly
    return 1;
  else
    return 0;
}

