#include <Keypad.h>
#include <Keyboard.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 0, 1, 2, 3 };
byte colPins[COLS] = { 10, 9, 8, 7 };

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Settings to display 4x4 labels on 128x64 OLED display
const int COL_WIDTH = 32;
const int ROW_HEIGHT = 15;
const int X_ORIGIN = 5;
const int Y_ORIGIN = 5;

// delay between keys for keyboard
const int DELAY_INTERVAL = 50;

// we use 4 keys to define 4 macro modes (remaining 12 keys x 4 modes = 48 macros possible)
int CURRENT_MACRO_MODE = 0;

struct t_modeInfo {
  String modeName;
  String label[ROWS * (COLS - 1)];
};

// Display labels for the macro display. This will correspond to macro functions initializer.
struct t_modeInfo MODE_LABELS[ROWS] = {
  { "WIND", { "TASK", "FILE", "ONE ", "PSWD", " -- ", " -- ", " -- ", " -- ", " -- ", "COPY", "PAST", "SCRN" } },
  { "POWR", { "TASK", "FILE", "ONE ", " -- ", " -- ", " -- ", "RECT", "CIRC", "ARRW", "COPY", "PAST", "SCRN" } },
  { "MTNG", { "TASK", "FILE", "ONE ", "MUTE", "UNMT", " -- ", " -- ", " -- ", " -- ", "COPY", "PAST", "SCRN" } },
  { "LINX", { " -- ", " -- ", " -- ", "PSWD", " -- ", " -- ", " -- ", " -- ", " -- ", "SCPY", "SPST", "SCRN" } }
};

// forward declare functions so we can initialize macro functions 
void WIN_taskmanager(void);
void WIN_filexplorer(void);
void WIN_onenote(void);
void WIN_copy(void);
void WIN_paste(void);
void WIN_screenCapture(void);
void WIN_password(void);
void POWERPOINT_circle(void);
void POWERPOINT_rectangle(void);
void POWERPNT_arrow(void);
void SHIFT_copy(void);
void SHIFT_paste(void);

// functions for each key in each of the macro modes. This will correspond to macro labels.
void (*macroFunction[4][12])(void) = {
    { WIN_taskmanager, WIN_filexplorer, WIN_onenote, 
      WIN_password, 0, 0, 
      0, 0, 0, 
      WIN_copy, WIN_paste, WIN_screenCapture},
    { WIN_taskmanager, WIN_filexplorer, WIN_onenote, 
      0, 0, 0, 
      POWERPOINT_rectangle, POWERPOINT_circle, POWERPNT_arrow, 
      WIN_copy, WIN_paste, WIN_screenCapture},
    { WIN_taskmanager, WIN_filexplorer, WIN_onenote, 
      0, 0, 0, 
      0, 0, 0, 
      WIN_copy, WIN_paste, WIN_screenCapture },
    { 0, 0, 0, 
      0, 0, 0, 
      0, 0, 0, 
      SHIFT_copy, SHIFT_paste, WIN_screenCapture } 
  };

void setup() {
  Serial.begin(57600);
  Keyboard.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for ( ;;)
    ;
  }
  delay(1000);

  initDisplay();
  setMacroMode(0);  // start by default in macro mode 0
}

void initDisplay() {

  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void setMacroMode(int mode) {
  CURRENT_MACRO_MODE = mode; // set the mode so other functions can read it
  display.clearDisplay(); // clear out previous labels
  drawModeLabels(mode); // display new labels for the new macro mode
  highlightCurrentMode(mode); // draw highlight box around new macro mode
  display.display(); // refresh with new values
}

int getMacroMode() {
  return CURRENT_MACRO_MODE;
}

// layout the labels in a nice 4x4 format
void drawModeLabels(int mode) {

  // here we just draw the labels for 4 rows of labels and 3 columns
  // the last column of labels is for the macro mode. 
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS - 1; j++) {
      writeText(i, j, MODE_LABELS[mode].label[i * 3 + j]);
    };

    // the last column of labels is for the macro mode. 
    writeText(i, COLS - 1, MODE_LABELS[i].modeName);
  };
}

// write text in specified row-col of OLED display
void writeText(int row, int col, String text) {

  display.setCursor(X_ORIGIN + COL_WIDTH * col, Y_ORIGIN + ROW_HEIGHT * row);
  display.print(text);
}

// draw a box around the correct mode. Use black box to "clear" other mode names
void highlightCurrentMode(int mode) {

    display.drawRect(X_ORIGIN + COL_WIDTH * 3 - 4, Y_ORIGIN + ROW_HEIGHT * 0 - 4, 30, 14, (mode==0?WHITE:BLACK));
    display.drawRect(X_ORIGIN + COL_WIDTH * 3 - 4, Y_ORIGIN + ROW_HEIGHT * 1 - 4, 30, 14, (mode==1?WHITE:BLACK));
    display.drawRect(X_ORIGIN + COL_WIDTH * 3 - 4, Y_ORIGIN + ROW_HEIGHT * 2 - 4, 30, 14, (mode==2?WHITE:BLACK));
    display.drawRect(X_ORIGIN + COL_WIDTH * 3 - 4, Y_ORIGIN + ROW_HEIGHT * 3 - 4, 30, 14, (mode==3?WHITE:BLACK));
}

void loop() {
  char customKey = customKeypad.getKey();

  switch (customKey) {

    // The first 4 set the macro mode
    case 'A': setMacroMode(0); Serial.print(customKey); break;
    case 'B': setMacroMode(1); Serial.print(customKey); break;
    case 'C': setMacroMode(2); Serial.print(customKey); break;
    case 'D': setMacroMode(3); Serial.print(customKey); break;

    // Run the macro function assigned to the key, based on the mode that has been selected
    case '1': if (macroFunction[getMacroMode()][0] != NULL) {macroFunction[getMacroMode()][0]();}; break;
    case '2': if (macroFunction[getMacroMode()][1] != NULL) {macroFunction[getMacroMode()][1]();}; break;
    case '3': if (macroFunction[getMacroMode()][2] != NULL) {macroFunction[getMacroMode()][2]();}; break;
    case '4': if (macroFunction[getMacroMode()][3] != NULL) {macroFunction[getMacroMode()][3]();}; break;
    case '5': if (macroFunction[getMacroMode()][4] != NULL) {macroFunction[getMacroMode()][4]();}; break;
    case '6': if (macroFunction[getMacroMode()][5] != NULL) {macroFunction[getMacroMode()][5]();}; break;
    case '7': if (macroFunction[getMacroMode()][6] != NULL) {macroFunction[getMacroMode()][6]();}; break; 
    case '8': if (macroFunction[getMacroMode()][7] != NULL) {macroFunction[getMacroMode()][7]();}; break;
    case '9': if (macroFunction[getMacroMode()][8] != NULL) {macroFunction[getMacroMode()][8]();}; break;
    case '*': if (macroFunction[getMacroMode()][9] != NULL) {macroFunction[getMacroMode()][9]();}; break;
    case '0': if (macroFunction[getMacroMode()][10] != NULL) {macroFunction[getMacroMode()][10]();}; break;
    case '#': if (macroFunction[getMacroMode()][11] != NULL) {macroFunction[getMacroMode()][11]();}; break;
  }
}

// hold down a key, e.g. ALT, SHIFT, etc.
void holdKey(u_int8_t key) {
  Keyboard.press(key);
  delay(DELAY_INTERVAL);
}

// press and release a key
void hitKey(u_int8_t key) {
  Keyboard.press(key);
  delay(100);
  Keyboard.release(key);
}

void WIN_screenCapture() {
  holdKey(KEY_LEFT_GUI);
  holdKey(KEY_LEFT_SHIFT);
  holdKey('s');
  Keyboard.releaseAll();
}

void WIN_copy() {
  holdKey(KEY_LEFT_CTRL);
  holdKey('c');
  Keyboard.releaseAll();
}

void WIN_paste() {
  holdKey(KEY_LEFT_CTRL);
  holdKey('v');
  Keyboard.releaseAll();
}

void POWERPNT_arrow() {
  hitKey(KEY_LEFT_ALT);
  hitKey('n');
  hitKey('s');
  hitKey('h');
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_RIGHT_ARROW);
  hitKey(KEY_RETURN);
}

void POWERPOINT_circle() {
  hitKey(KEY_LEFT_ALT);
  hitKey('n');
  hitKey('s');
  hitKey('h');
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_RIGHT_ARROW);
  hitKey(KEY_RETURN);
}


void POWERPOINT_rectangle() {
  hitKey(KEY_LEFT_ALT);
  hitKey('n');
  hitKey('s');
  hitKey('h');
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_DOWN_ARROW);
  hitKey(KEY_RETURN);
}

void WIN_password() {
  Keyboard.print("********");
  delay(100);
  hitKey(KEY_RETURN);
}

void WIN_onenote() {
  holdKey(KEY_LEFT_GUI);
  holdKey(KEY_LEFT_SHIFT);
  hitKey('n');
  Keyboard.releaseAll();
}

void WIN_filexplorer() {
  holdKey(KEY_LEFT_GUI);
  hitKey('e');
  Keyboard.releaseAll();
}

void WIN_taskmanager() {
  holdKey(KEY_LEFT_GUI);
  hitKey('x');
  Keyboard.releaseAll();
  hitKey('t');
}

void SHIFT_copy() {
  holdKey(KEY_LEFT_CTRL);
  holdKey(KEY_LEFT_SHIFT);
  holdKey('c');
  Keyboard.releaseAll();
}

void SHIFT_paste() {
  holdKey(KEY_LEFT_CTRL);
  holdKey(KEY_LEFT_SHIFT);
  holdKey('v');
  Keyboard.releaseAll();
}
