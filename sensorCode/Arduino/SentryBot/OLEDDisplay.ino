
String currMotionText = "NA";
int currFrontClearance = 0;
int currLeftClearance = 0;
int currRightClearance = 0;

void OLEDSetup(){

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay(); 
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Welcome to");

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 17);
  display.println("SentryBot");

  display.display();

  delay(500);
  display.clearDisplay();

  display.setCursor(0,30); display.setTextSize(2); display.print("Motion");
}

void updateDisplayMotion(String displayText) {

  display.setTextSize(2);
  display.setTextColor(BLACK); display.setCursor(88,30); display.print(currMotionText); 
  currMotionText = displayText;
  display.setTextColor(WHITE); display.setCursor(88,30); display.print(currMotionText);

  display.display();
}

void displayDistances(int left, int front, int right) {

  display.setTextSize(2);
  display.setCursor(0,0); display.setTextSize(2); display.setTextColor(BLACK);display.print(currLeftClearance);
  display.setCursor(45,0); display.setTextSize(2); display.setTextColor(BLACK);display.print(currFrontClearance);
  display.setCursor(88,0); display.setTextSize(2); display.setTextColor(BLACK);display.print(currRightClearance);
  currLeftClearance = left;
  currFrontClearance = front;
  currRightClearance = right;
  display.setCursor(0,0); display.setTextSize(2); display.setTextColor(WHITE);display.print(currLeftClearance);
  display.setCursor(45,0); display.setTextSize(2); display.setTextColor(WHITE);display.print(currFrontClearance);
  display.setCursor(88,0); display.setTextSize(2); display.setTextColor(WHITE);display.print(currRightClearance);
  display.display();
}
