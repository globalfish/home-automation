// main script for the Sentry Robot. 
// SubFiles - 
// 1) Driving Motors - Directional Movement 
// 2) Ultrasonic Sensor - obstacle sensing
// 3) (will be moved to camera board) Servos - Pan tilt 
// 4) Light Sensor - LDR to detect light level 
// 5) PIR motion sensor - Motion sensing
// 6) Microphone - Sound sensing 
// 7) Flash Light - Alert light flash
// 8) Alarm - Alert sound buzzer

#include "Pin_Connection.h"

float ground_level = 0;
int loop_counter=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  digitalWrite(LED_BUILTIN,HIGH);
  motor_setup(); 
  ultrasonic_setup();
  // servo_setup();
  ldr_setup();
  pir_setup();
  mic_setup();
  flash_setup();
  buzzer_setup();
  Serial.println(ground_level);
  delay(2000);
  digitalWrite(LED_BUILTIN,LOW);  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(loop_counter%10 == 0){
    find_max_distance();
  }

 if(measure_distance_level(20,ground_level)){
    motor_forward();
  }
  else if(abs(measure_level()-ground_level) > 15){
    Serial.println("level error");
    digitalWrite(LED_BUILTIN,HIGH);
    motor_stop();
    delay(500);
    motor_backward();
    delay(2000);
    digitalWrite(LED_BUILTIN,LOW);
    find_max_distance();
  }
  else{
    int dist = measure_distance();
      if((dist<20)&&(dist!=0)){
        Serial.println("Distance error");
        digitalWrite(LED_BUILTIN,HIGH);
        motor_stop();
        delay(300);
        digitalWrite(LED_BUILTIN,LOW);
        delay(200);
        motor_backward();
        digitalWrite(LED_BUILTIN,HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN,LOW);
        delay(500);
        find_max_distance();
      }
  }

  loop_counter++;
  if(loop_counter>=50){
    do_something();
    loop_counter=0;
    // if(intruder_detect()) alert();
  }

  delay(1000);
}

int find_max_distance(){
  int dist[8],index=-1,m_dist=0;
  measure_again:
  for(int i=0;i<8;i++){
    motor_stop();
    delay(500);
    dist[i]=measure_distance();
    if(dist[i]>200){
      motor_forward();
      return i;
    }
    if(dist[i]>m_dist){
      m_dist=dist[i];
      index=i;
    }
    
    motor_clockwise();
    delay(550);
  }
  if((index==-1)||m_dist<=0){
    buzz();
    motor_stop();
    delay(50);
    motor_backward();
    delay(500);
    goto measure_again;
  }
  motor_stop();
  delay(100);
  motor_clockwise();
  delay(index*550);
  return index;
  
}

void do_something(){
    buzz(); 
    digitalWrite(LED_BUILTIN,HIGH);
    motor_stop();
    delay(500);
    int r=random(4);
    switch (r){
      case 0 : motor_backward();
        break;
      case 1 : motor_clockwise();
        break;
      case 2 : motor_anticlockwise();
        break;
      case 3: motor_forward();
    }
    delay(random(1,6)*1000);
    digitalWrite(LED_BUILTIN,LOW);
}

int intruder_detect(){
  int confidence=0;
  if(pir_detect() && sound_measure()){
    delay(100);
    for(int i=0;i<5;i++){
      if(pir_detect() && sound_measure()) return 1;
    }
  }
  return 0;
}

void alert(){
  buzz();
  flash();
  motor_clockwise();
  delay(2000);
  buzz();
  flash();
}

