#include <Keypad.h>
#include <Keyboard.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Settings to display 4x4 labels on 128x64 OLED display
const int COL_WIDTH = 32;
const int ROW_HEIGHT = 15;
const int X_ORIGIN = 5;
const int Y_ORIGIN = 5;

// define keypad characteristics
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

// define pins on XIAO that are connected to the keypad
byte rowPins[ROWS] = { 0, 1, 2, 3 };
byte colPins[COLS] = { 10, 9, 8, 7 };

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
// delay between key presses for keyboard
const int DELAY_INTERVAL = 50;


// we use 4 keys to define 4 macro modes (remaining 12 keys x 4 modes = 48 macros possible)
int CURRENT_MACRO_MODE = 0;  // start with Mode 0 as the default mode.

//////////////////////////////////////////////////
// To create a new macro:
//
// 1. Implement the macro function
// 2. Declare the function before it's called
// 3. Update 'macroFunctionList' with a 4-letter label and the function name
// 4. Update 'LABEL_MAP' with the label corresponding to the mode in which you want it to work
//    If you want it in each mode then you have to repeat the label in each mode
//
//////////////////////////////////////////////////

// forward declare macro functions
void WIN_taskmanager(void);
void WIN_filexplorer(void);
void WIN_onenote(void);
void WIN_copy(void);
void WIN_paste(void);
void WIN_screenCapture(void);
void WIN_password(void);
void POWERPOINT_circle(void);
void POWERPOINT_rectangle(void);
void POWERPOINT_arrow(void);
void POWERPOINT_textBox(void);
void TEAMS_camera_toggle(void);
void TEAMS_mute_toggle(void);
void TEAMS_share_toggle(void);
void TEAMS_chat(void);
void SHIFT_copy(void);
void SHIFT_paste(void);
void LINUX_password(void);

// we store the macrofunctions in arrays., Typedef it for ease
typedef void (*macroFunction)(void);

// maps Function label to function name. These function labels are used for display and indexing.
struct t_macroFunction {
  String macroLabel;
  macroFunction functionName;
} macroFunctionList[] = { 
  
  { "TASK", WIN_taskmanager },
  { "FILE", WIN_filexplorer },
  { "1NOT", WIN_onenote },
  { "PSWD", WIN_password },
  { "COPY", WIN_copy },
  { "PAST", WIN_paste },
  { "SCRN", WIN_screenCapture },
  { "TEXT", POWERPOINT_textBox },
  { "RECT", POWERPOINT_rectangle },
  { "CIRC", POWERPOINT_circle },
  { "ARRW", POWERPOINT_arrow },
  { "CAME", TEAMS_camera_toggle },
  { "MUTE", TEAMS_mute_toggle },
  { "SHAR", TEAMS_share_toggle },
  { "SCPY", SHIFT_copy },
  { "SPST", SHIFT_paste },
  { "CHAT", TEAMS_chat }
};

// Arrange labels for each mode. The macro functions will be called automatically based on the label text below. 
// The label text must match EXACTLY to what's listed in the macroFunctionList array
struct t_modeInfo {
  String modeName;
  String label[ROWS * (COLS - 1)];
} LABEL_MAP[ROWS] = {
  { "WIND", { "TASK", "FILE", "1NOT", "PSWD", " -- ", " -- ", " -- ", " -- ", " -- ", "COPY", "PAST", "SCRN" } },
  { "POWR", { "TASK", "FILE", "1NOT", "TEXT", " -- ", " -- ", "RECT", "CIRC", "ARRW", "COPY", "PAST", "SCRN" } },
  { "MTNG", { "TASK", "FILE", "1NOT", "CAME", "MUTE", "SHAR", "CHAT", " -- ", " -- ", "COPY", "PAST", "SCRN" } },
  { "LINX", { " -- ", " -- ", " -- ", "PSWD", " -- ", " -- ", " -- ", " -- ", " -- ", "SCPY", "SPST", "SCRN" } }
};

// the functionmap that will be used to call from keypress. This will be built up once at startup.
void (*functionMap[4][12])(void);


void setup() {
  Serial.begin(57600);
  Keyboard.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(1000);

  buildFunctionMap(); // use the display labels and build the function map from them

  initDisplay();
  setMacroMode(0);  // start by default in macro mode 0
}

// for a given label find and return the corresponding macro function
macroFunction getFunctionForLabel(String label) {

  int numLabels = sizeof( macroFunctionList)/sizeof(t_macroFunction);

  for(int i = 0; i < numLabels; i++) {
    if(strcmp(macroFunctionList[i].macroLabel.c_str(),  label.c_str()) == 0){
      return macroFunctionList[i].functionName;
    };
  };

  return 0;
}

void buildFunctionMap() {

  for(int i = 0; i < 4; i++) {
    for (int j = 0; j < 12; j++) {
      functionMap[i][j] = getFunctionForLabel(LABEL_MAP[i].label[j]);
    }
  }
}


void initDisplay() {

  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void setMacroMode(int mode) {
  CURRENT_MACRO_MODE = mode;   // set the mode so other functions can read it
  display.clearDisplay();      // clear out previous labels
  drawModeLabels(mode);        // display new labels for the new macro mode
  highlightCurrentMode(mode);  // draw highlight box around new macro mode
  display.display();           // refresh with new values
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
      writeText(i, j, LABEL_MAP[mode].label[i * 3 + j]);
    };

    // the last column of labels is for the macro mode.
    writeText(i, COLS - 1, LABEL_MAP[i].modeName);
  };
}

// write text in specified row-col of OLED display
void writeText(int row, int col, String text) {

  display.setCursor(X_ORIGIN + COL_WIDTH * col, Y_ORIGIN + ROW_HEIGHT * row);
  display.print(text);
}

// draw a box around the correct mode. Use black box to "clear" other mode names
void highlightCurrentMode(int mode) {

  for (int rowNumber = 0; rowNumber < ROW_HEIGHT; rowNumber++) {
    display.drawRect(X_ORIGIN + COL_WIDTH * 3 - 4,           // x coord for box, fixed for each row
                     Y_ORIGIN + ROW_HEIGHT * rowNumber - 4,  // y coord for box for each row
                     30,                                     // width of box
                     14,                                     // height of box
                     (mode == rowNumber ? WHITE : BLACK));   // only draw in white for highlighted 'mode'
  }
}

void loop() {
  char customKey = customKeypad.getKey();

  switch (customKey) {

    // The first 4 set the macro mode
    case 'A':
      setMacroMode(0);
      break;
    case 'B':
      setMacroMode(1);
      break;
    case 'C':
      setMacroMode(2);
      break;
    case 'D':
      setMacroMode(3);
      break;

    // Run the macro function assigned to the key, based on the mode that has been selected
    case '1':
      if (functionMap[getMacroMode()][0] != NULL) { functionMap[getMacroMode()][0](); };
      break;
    case '2':
      if (functionMap[getMacroMode()][1] != NULL) { functionMap[getMacroMode()][1](); };
      break;
    case '3':
      if (functionMap[getMacroMode()][2] != NULL) { functionMap[getMacroMode()][2](); };
      break;
    case '4':
      if (functionMap[getMacroMode()][3] != NULL) { functionMap[getMacroMode()][3](); };
      break;
    case '5':
      if (functionMap[getMacroMode()][4] != NULL) { functionMap[getMacroMode()][4](); };
      break;
    case '6':
      if (functionMap[getMacroMode()][5] != NULL) { functionMap[getMacroMode()][5](); };
      break;
    case '7':
      if (functionMap[getMacroMode()][6] != NULL) { functionMap[getMacroMode()][6](); };
      break;
    case '8':
      if (functionMap[getMacroMode()][7] != NULL) { functionMap[getMacroMode()][7](); };
      break;
    case '9':
      if (functionMap[getMacroMode()][8] != NULL) { functionMap[getMacroMode()][8](); };
      break;
    case '*':
      if (functionMap[getMacroMode()][9] != NULL) { functionMap[getMacroMode()][9](); };
      break;
    case '0':
      if (functionMap[getMacroMode()][10] != NULL) { functionMap[getMacroMode()][10](); };
      break;
    case '#':
      if (functionMap[getMacroMode()][11] != NULL) { functionMap[getMacroMode()][11](); };
      break;
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

void POWERPOINT_arrow() {
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

void POWERPOINT_textBox() {
  hitKey(KEY_LEFT_ALT);
  hitKey('n');
  hitKey('x');
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

void TEAMS_mute_toggle() {
  holdKey(KEY_LEFT_CTRL);
  holdKey(KEY_LEFT_SHIFT);
  hitKey('m');
  Keyboard.releaseAll();
}

void TEAMS_camera_toggle() {
  holdKey(KEY_LEFT_CTRL);
  holdKey(KEY_LEFT_SHIFT);
  hitKey('o');
  Keyboard.releaseAll();
}

void TEAMS_share_toggle() {
  holdKey(KEY_LEFT_CTRL);
  holdKey(KEY_LEFT_SHIFT);
  hitKey('e');
  Keyboard.releaseAll();
}

void TEAMS_chat() {
  holdKey(KEY_LEFT_CTRL);
  holdKey(KEY_LEFT_SHIFT);
  hitKey('n');
  Keyboard.releaseAll();
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

void LINUX_password() {
  Keyboard.print("venkat");
  delay(100);
  hitKey(KEY_RETURN);
}
