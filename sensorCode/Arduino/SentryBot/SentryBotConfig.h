// 
// motor pins

// Left Drive Motor
#define M1_P 9
#define M1_N 10
// Right Drive Motor
#define M2_P 11
#define M2_N 12

//
// Ultrasonic sensors for directly in front (collision) and front left and right (avoidance)
//
#define RIGHT_TRIG 7
#define RIGHT_ECHO 8
#define FRONT_TRIG 5
#define FRONT_ECHO 6
#define LEFT_TRIG 3
#define LEFT_ECHO 4

#define MAX_DISTANCE 300 // range of vision in cms

NewPing leftSonar(LEFT_TRIG, LEFT_ECHO, MAX_DISTANCE);
NewPing frontSonar(FRONT_TRIG, FRONT_ECHO, MAX_DISTANCE);
NewPing rightSonar(RIGHT_TRIG, RIGHT_ECHO, MAX_DISTANCE);

//
// Operational parameters and thresholds
const int MIN_CLEARANCE = 5; // minimum clearance in inches to trigger possible movement

//
// OLED Display
#define OLED_ADDR 0x3c
Adafruit_SSD1306 display(128,64);
